#pragma once
#include "../export.h"
#include <cstdint>
#include <miniaudio.h>
#include <vector>
#include <memory>

namespace swav{

class Middleware;
struct Context;

class SWAV_API Output {
public:
  // This method will be called by input to write to the buffer
  void write(const void *data, uint32_t noOfFrames, int32_t src = -1);
  uint32_t availableWrite(int32_t src = -1);

  Output(const char *name, Context &context, uint32_t bufferSizeInFrames,
         uint32_t delay = 0);

  virtual ~Output();

  virtual void start() = 0;
  virtual void stop() = 0;

  // Returns -1 if not successfull else return the id of middleware
  int32_t addMiddleware(Middleware &middleware);

public:
  const char *name;

protected:
  void implicitWrite(const void *data, uint32_t noOfFrames);
  uint32_t implicitAvailableWrite();
  uint32_t availableRead();
  uint32_t delay;
  bool playing = false;
  void read(void *buffer, uint32_t noOfFrames);
  ma_pcm_rb *buffer;
  Context &context;
  std::vector<std::unique_ptr<Middleware>> middlewares;
};

}
