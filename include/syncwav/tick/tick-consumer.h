#pragma once
#include "../export.h"
#include "cstdint"
#include <miniaudio.h>

namespace swav {
class TickProducer;
class Context;

class SWAV_API TickConsumer {
public:
  TickConsumer(Context &context);
  virtual ~TickConsumer();
  void tick(uint32_t frames, TickProducer *src);
  void reAuthorize();

  // noOfFrames is set to the frames actually read
  void read(void *buffer, uint32_t &noOfFrames);

  uint32_t availableRead() const;
  inline virtual void stop() { stopped = true; }
  inline virtual void start() {
    stopped = false;
    reAuthorize();
  }

protected:
  ma_pcm_rb *buffer;
  void write(const void *data, uint32_t noOfFrames);
  uint32_t availableWrite() const;
  Context &context;
  bool stopped = true;
  TickProducer *authorisedProducer = nullptr;

private:
  void *tempBuffer;
};

} // namespace swav
