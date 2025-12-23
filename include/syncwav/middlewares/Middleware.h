#pragma once
#include "../Context.h"
#include <cstdint>

namespace swav {

class SWAV_API Middleware {
public:
  Middleware(const char *name, Context &context, uint32_t bufferSizeInFrames);
  void write(const void *data, uint32_t noOfFrames);
  uint32_t availableWrite();

  virtual ~Middleware();
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual std::unique_ptr<Middleware> clone() const = 0;
  void initialize(int32_t id, Output *output);

public:
  const char *name;
  inline uint32_t getId() { return id; }
  inline bool isInitialized() { return initialized; }

protected:
  bool playing = false;
  ma_pcm_rb *buffer;
  Context &context;
  bool initialized = false;
  Output *output;
  int32_t id = -1;
  uint32_t availableRead();
  void read(void *buffer, uint32_t noOfFrames);
};

} // namespace swav
