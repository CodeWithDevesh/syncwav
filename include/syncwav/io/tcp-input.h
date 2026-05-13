#pragma once
#include "../export.h"
#include "input.h"
#include <atomic>
#include <miniaudio.h>

#ifdef SWAV_USE_WEBSOCKETS
#include <ixwebsocket/IXWebSocket.h>
#endif

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
  const char *ip;
  int port;

#ifdef SWAV_USE_WEBSOCKETS
  std::thread congestionThread;
  ix::WebSocket *websocket = nullptr;
#endif
};
} // namespace swav
