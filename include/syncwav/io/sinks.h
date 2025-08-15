#pragma once
#include "../export.h"
#include "../miniaudio.h"
#include <cstdint>
#include "../context.h"

namespace swav {
class SWAV_API Output {
public:
  // This method will be called by input to write to the buffer
  void write(const void *data, uint32_t noOfFrames);
  uint32_t availableWrite();

  Output(const char *name, Context& context, uint32_t bufferSizeInFrames);

  virtual ~Output();

  virtual void start() = 0;
  virtual void stop() = 0;

public:
  const char *name;

protected:
  uint32_t availableRead();
  void read(void* buffer, uint32_t noOfFrames);
  ma_pcm_rb *buffer;
  Context& context;
};

class SWAV_API Input {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~Input() = default;
  Input(const char *name, Context& context);

public:
  const char *name;

protected:
  Context& context;
};

} // namespace swav
