#pragma once
#include "../export.h"
#include <atomic>
#include "sinks.h"
#include "thread"
#include <App.h>

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
  std::thread appThread, broadcastThread;
  std::atomic<bool> running{false};
  uWS::App *globalApp;
  uWS::Loop *globalLoop;
  const char* ip;
  int port;
};
} // namespace swav
