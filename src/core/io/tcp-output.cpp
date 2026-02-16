#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocketMessageType.h"
#include "syncwav/context.h"
#include <algorithm>
#include <chrono>
#include <string>
#include <syncwav/io/tcp-output.h>
#include <syncwav/log.h>

using namespace std::chrono;
namespace swav {

TCPOutput::TCPOutput(Context &context, const char *ip, int port)
    : Output("TCP Output", context), ip(ip), port(port) {
  ix::initNetSystem();
  priority = .5;
  delay = microseconds(30000);
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
          priorities.erase(connectionState->getId());
        } else if (msg->type == ix::WebSocketMessageType::Message) {
          int avlPer = std::stoi(msg->str);
          calculateDelay(avlPer);
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
    if (broadcastThread.joinable())
      broadcastThread.join();
  }
}

// Continously send frames to the clients
void TCPOutput::run() {
  log::i("[TCP OUTPUT] starting the server on {}:{}", ip, port);
  server->listenAndStart();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  broadcastThread = std::thread(([&] {
    auto now = steady_clock::now();
    steady_clock::time_point nextTick = now + 10ms;
    void *buff = malloc(context.framesSizeInBytes * context.bufferSizeInFrames);
    while (running) {
      std::this_thread::sleep_until(nextTick);

      // if (availableRead() < packetSize) {
      //   nextTick += delay;
      //   continue;
      // }
      // ma_uint32 framesToSend = std::min(framesAvailable, 480u);
      uint32_t toRead = packetSize;
      read(buff, toRead);
      log::t("[TCP Output] Gonna send {} frames of audio data.", toRead);

      std::string_view chunk(reinterpret_cast<const char *>(buff),
                             toRead * context.framesSizeInBytes);
      {
        std::lock_guard<std::mutex> lock(socketsMutex);
        for (auto &[id, ws] : sockets) {
          ws->send(std::string(chunk), true);
        }
      }

      nextTick += delay;
    }
    free(buff);
    log::i("[TCP OUTPUT] stopping the server");
    server->stop();
  }));
}

// Adjust send rate based on feedback from client
// avlPer is the percentage of available space in client buffer
void TCPOutput::calculateDelay(int avlPer) {
  if(ignoreChange){
    ignoreChange--;
    return;
  }
  int d = delay.count();
  log::i("Client buffer has {}% free space", avlPer);
  if (avlPer > 80) {
    highConsCnt++;
    if (highConsCnt == 2) {
      highConsCnt = 0;
      d = d - step;
    }
  } else {
    highConsCnt = 0;
  }
  if (avlPer < 60) {
    lowConsCnt++;
    if (lowConsCnt == 4) {
      lowConsCnt = 0;
      d = d + step;
    }
  } else {
    lowConsCnt = 0;
  }
  d = std::clamp(d, minDelay, maxDelay);
  if (d != delay.count()) {
    log::i("Changed delay from {} to {}", delay.count(), d);
    ignoreChange = 5;
  }
  delay = microseconds(d);
}

} // namespace swav
