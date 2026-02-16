#pragma once
#include "../export.h"
#include "output.h"
#include "thread"
#include <atomic>
#include <chrono>
#include <ixwebsocket/IXWebSocketServer.h>

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
  std::thread broadcastThread;
  std::atomic<bool> running{false};
  int minDelay = 2000, maxDelay = 50000;
  std::chrono::microseconds delay ;
  ix::WebSocketServer *server = nullptr;
  std::mutex socketsMutex;
  std::unordered_map<std::string, ix::WebSocket *> sockets;
  std::unordered_map<std::string, float> priorities;
  const char *ip;
  int32_t port;
  int32_t packetSize = 480 * 3;
  int highConsCnt = 0;
  int lowConsCnt = 0;
  int ignoreChange = 0;
  int step = 250;
};
} // namespace swav
