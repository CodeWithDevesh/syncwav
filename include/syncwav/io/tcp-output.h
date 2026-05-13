#pragma once
#include "../export.h"
#include "output.h"
#include <atomic>
#include <chrono>

#ifdef SWAV_USE_WEBSOCKETS
#include "thread"
#include <ixwebsocket/IXWebSocketServer.h>
#endif

namespace swav {
class SWAV_API TCPOutput : public Output {
public:
  TCPOutput(Context &context, const char *ip, int port);
  ~TCPOutput();
  void start() override;
  void stop() override;

private:
  void run();
  void calculateDelay(int avlPer);

private:
  std::atomic<bool> running{false};
  int minDelay = 2000, maxDelay = 50000;
  std::chrono::microseconds delay;
  const char *ip;
  int32_t port;
  int32_t packetSize = 480 * 3;
  int highConsCnt = 0;
  int lowConsCnt = 0;
  int ignoreChange = 0;
  int step = 250;
#ifdef SWAV_USE_WEBSOCKETS
  std::mutex socketsMutex;
  std::unordered_map<std::string, float> priorities;
  std::thread broadcastThread;
  ix::WebSocketServer *server = nullptr;
  std::unordered_map<std::string, ix::WebSocket *> sockets;
#endif
};
} // namespace swav
