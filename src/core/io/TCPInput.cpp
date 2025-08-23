#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXUserAgent.h>
#include <string>
#include <syncwav/io/TCPInput.h>
#include <syncwav/log.h>

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
      for (auto &o : context.outputs) {
        o->write(msg->str.data(), msg->str.size() / context.framesSizeInBytes);
      }
    } else if (msg->type == ix::WebSocketMessageType::Open) {
      log::i("[TCP Input] Connection established");
    } else if (msg->type == ix::WebSocketMessageType::Error) {
      // Maybe SSL is not configured properly
      log::e("[TCP Input] Connection error: {}", msg->errorInfo.reason);
    }
  });
}

TCPInput::~TCPInput() {
  stop();
  delete websocket;
}

void TCPInput::start() {
  if (!running) {
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
  }
}

void TCPInput::run() {}

} // namespace swav
