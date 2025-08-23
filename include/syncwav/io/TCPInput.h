#pragma once
#include "../export.h"
#include "sinks.h"
#include <atomic>
#include <ixwebsocket/IXWebSocket.h>

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
