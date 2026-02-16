#pragma once
#include "../export.h"
#include "cstdint"
#include <miniaudio.h>

namespace swav {
class TickConsumer;
class Context;

class SWAV_API TickProducer {
public:
  TickProducer(Context &context);
  virtual ~TickProducer();

  inline float getPriority() { return priority; }
  inline void setAuthority(bool auth) { hasAuthority = auth; };
  void write(const void *data, uint32_t noOfFrames);
  uint32_t availableWrite();
  void sendTick();

  virtual inline void start() { stopped = false; }
  virtual inline void stop() { stopped = true; }

protected:
  float priority = 0;
  int tickThreshold;
  bool hasAuthority = false;
  ma_pcm_rb *buffer;
  uint32_t availableRead();
  void read(void *buffer, uint32_t noOfFrames);
  Context &context;
  bool stopped = true;

private:
};

} // namespace swav
