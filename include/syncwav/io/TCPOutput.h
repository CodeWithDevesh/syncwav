#pragma once
#include "../Export.h"
#include "Sinks.h"
#include "thread"
#include <atomic>
#include <ixwebsocket/IXWebSocketServer.h>

namespace swav {
class SWAV_API TCPOutput : public Output {
public:
  TCPOutput(Context &context, const char *ip, int port, int bufferSizeInFrames);
  ~TCPOutput();
  void start() override;
  void stop() override;

private:
  void run();

private:
  std::thread broadcastThread;
  std::atomic<bool> running{false};
  ix::WebSocketServer *server = nullptr;
  std::mutex socketsMutex;
  std::unordered_map<std::string, ix::WebSocket *> sockets;
  const char *ip;
  int port;
};
} // namespace swav
