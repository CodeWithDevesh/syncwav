#pragma once
#include "../export.h"
#include "../miniaudio.h"
#include <cstdint>

namespace swav {
class SWAV_API Output {
public:
  // This method will be called by input to write to the buffer
  void write(const void *data, uint32_t noOfFrames);
  uint32_t availableWrite();

  Output(const char *name, uint32_t bufferSizeInFrames);

  virtual ~Output();

  virtual void start() = 0;
  virtual void stop() = 0;

public:
  const char *name;

protected:
  uint32_t availableRead();
  void read(void* buffer, uint32_t noOfFrames);
  ma_pcm_rb *buffer;
};

class SWAV_API Input {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual ~Input() = default;
  Input(const char *name);

public:
  const char *name;
};

} // namespace swav
