#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "syncwav/context.h"
#include <syncwav/io/tcp-output.h>
#include <syncwav/log.h>

namespace swav {

TCPOutput::TCPOutput(Context &context, const char *ip, int port, int buffSiz)
    : Output("TCP Output", context, buffSiz), ip(ip), port(port) {
  ix::initNetSystem();
  server = new ix::WebSocketServer(port, ip);
  server->setOnClientMessageCallback(
      [&](std::shared_ptr<ix::ConnectionState> connectionState,
          ix::WebSocket &webSocket, const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
          swav::log::i("New connection, id: {}", connectionState->getId());
          std::lock_guard<std::mutex> lock(socketsMutex);
          sockets[connectionState->getId()] = &webSocket;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
          swav::log::i("Closed connection, id: {}", connectionState->getId());
          std::lock_guard<std::mutex> lock(socketsMutex);
          sockets.erase(connectionState->getId());
        }
      });
}

TCPOutput::~TCPOutput() {
  ix::uninitNetSystem();
  stop();
  delete (server);
}

void TCPOutput::start() {
  if (!running) {
    running = true;
    Output::start();
    run();
  }
}

void TCPOutput::stop() {
  if (running) {
    Output::stop();
    running = false;
    broadcastThread.join();
  }
}

void TCPOutput::run() {
  log::i("[TCP OUTPUT] starting the server on {}:{}", ip, port);
  server->listenAndStart();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  broadcastThread = std::thread(([&] {
    using namespace std::chrono;
    auto now = steady_clock::now();
    steady_clock::time_point nextTick = now + 10ms;
    void *buffer = malloc(context.framesSizeInBytes * 48000);
    while (running) {
      std::this_thread::sleep_until(nextTick);

      ma_uint32 framesAvailable = availableRead();

      if (framesAvailable < 480) {
        nextTick += 5ms;
        continue;
      }
      framesAvailable /= 480;

      ma_uint32 framesToSend = ((framesAvailable * 4) / 4);
      framesToSend *= 480;
      // ma_uint32 framesToSend = std::min(framesAvailable, 480u);
      log::t("[TCP Output] Gonna send {} frames of audio data.", framesToSend);
      read(buffer, framesToSend);

      std::string_view chunk(reinterpret_cast<const char *>(buffer),
                             framesToSend * context.framesSizeInBytes);
      {
        std::lock_guard<std::mutex> lock(socketsMutex);
        for (auto &[id, ws] : sockets) {
          ws->send(std::string(chunk), true);
        }
      }

      // nextTick += ((framesToSend * 1000) / context.sampleRate) * 1ms;
      nextTick += 5ms;
    }
    free(buffer);
    log::i("[TCP OUTPUT] stopping the server");
    server->stop();
  }));
}

} // namespace swav
