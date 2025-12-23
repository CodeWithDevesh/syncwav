#pragma once
#include "../Export.h"
#include "Sinks.h"
#include <atomic>
#include <ixwebsocket/IXWebSocket.h>
#include <miniaudio.h>

namespace swav {
class SWAV_API TCPInput : public Input {
public:
  TCPInput(Context &context, const char *ip, int port);
  ~TCPInput();
  void start() override;
  void stop() override;

private:
  void run();

private:
  std::atomic<bool> running{false};
  ix::WebSocket *websocket = nullptr;
  const char *ip;
  int port;
};
} // namespace swav
