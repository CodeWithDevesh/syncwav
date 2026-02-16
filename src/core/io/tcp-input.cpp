#include <chrono>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXUserAgent.h>
#include <string>
#include <syncwav/context.h>
#include <syncwav/io/output.h>
#include <syncwav/io/tcp-input.h>
#include <syncwav/log.h>
#include <thread>

namespace swav {

TCPInput::TCPInput(Context &context, const char *ip, int port)
    : Input("TCP Input", context), ip(ip), port(port) {
  ix::initNetSystem();
  websocket = new ix::WebSocket();
  std::string conn = "ws://";
  conn += ip;
  conn += ":" + std::to_string(port);
  websocket->setUrl(conn);

  websocket->setOnMessageCallback([&](const ix::WebSocketMessagePtr &msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
      write(msg->str.data(), msg->str.size() / context.framesSizeInBytes);
    } else if (msg->type == ix::WebSocketMessageType::Open) {
      log::i("[TCP Input] Connection established");
    } else if (msg->type == ix::WebSocketMessageType::Error) {
      log::e("[TCP Input] Connection error: {}", msg->errorInfo.reason);
    }
  });
}

TCPInput::~TCPInput() {
  stop();
  delete websocket;
  ix::uninitNetSystem();
}

void TCPInput::start() {
  if (!running) {
    log::i("[TCP INPUT] starting the TCP input");
    Input::start();
    websocket->start();
    run();
    running = true;
  }
}

void TCPInput::stop() {
  if (running) {
    Input::stop();
    websocket->stop();
    running = false;
    if (congestionThread.joinable())
      congestionThread.join();
  }
}

void TCPInput::run() {
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  congestionThread = std::thread(([&] {
    using namespace std::chrono;
    auto now = steady_clock::now();
    steady_clock::time_point nextTick = now + 100ms;
    while (running) {
      std::this_thread::sleep_until(nextTick);
      ma_uint32 spaceAvl = availableWrite();
      log::i("Available Space: {}", spaceAvl);
      websocket->send(
          std::to_string((spaceAvl * 100) / context.bufferSizeInFrames));
      nextTick += 100ms;
    }
  }));
}

} // namespace swav
