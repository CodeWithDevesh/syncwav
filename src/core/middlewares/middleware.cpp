#include "syncwav/Format.h"
#include <syncwav/io/Sinks.h>
#include <syncwav/Log.h>
#include <syncwav/middlewares/Middleware.h>

namespace swav {

Middleware::Middleware(const char *name, Context &context,
                       uint32_t bufferSizeInFrames)
    : name(name), context(context) {
  log::i("[middleware] Creating Middleware: {}", name);
  buffer = new ma_pcm_rb();
  if (ma_pcm_rb_init(toMiniaudioFormat(context.format), context.channels,
                     bufferSizeInFrames, NULL, NULL, buffer)) {
    log::e("Failed to allocate buffer for {}", name);
    std::exit(-1);
  }
  log::i("Buffer for {} allocated successfully", name);
}

void Middleware::initialize(int32_t id, Output *output) {
  if (initialized) {
    log::e("[middleware] [{}: {}] already initialized", this->output->name,
           name);
    return;
  }
  this->id = id;
  this->output = output;
  initialized = true;
  log::i("[middleware] {} Initialized {} with id: {}", this->output->name, name,
         id);
}

Middleware::~Middleware() {
  log::i("[middleware] Destroying Middleware id: {}", id);
  if (buffer) {
    ma_pcm_rb_uninit(buffer);
    delete buffer;
  }
}

void Middleware::write(const void *data, uint32_t noOfFrames) {
  void *bufferOut;
  ma_uint32 framesToWrite = noOfFrames;
  ma_uint32 framesWritten = 0;
  noOfFrames = std::min(noOfFrames, availableWrite());
  log::t("[middleware] [{}: {}] Writing {} frames", noOfFrames, output->name,
         name);
  while (framesWritten < noOfFrames) {
    if (ma_pcm_rb_acquire_write(buffer, &framesToWrite, &bufferOut) !=
        MA_SUCCESS) {
      log::e("[middleware] [{}: {}] Failed to aquire write on buffer",
             output->name, name);
      return;
    }
    memcpy(bufferOut,
           static_cast<const uint8_t *>(data) +
               (framesWritten * context.framesSizeInBytes),
           static_cast<size_t>(framesToWrite) * context.framesSizeInBytes);
    ma_pcm_rb_commit_write(buffer, framesToWrite);
    log::t("[middleware] [{}] Written {} frames to {}", framesToWrite,
           output->name, name);
    framesWritten += framesToWrite;
    framesToWrite = noOfFrames - framesWritten;
  }
}

void Middleware::read(void *outBuff, uint32_t frames) {
  ma_uint32 framesToRead = frames;
  uint32_t framesRead = 0;
  frames = std::min(frames, availableRead());
  log::t("[middleware] [{}: {}] Reading {} frames", frames, output->name, name);
  while (framesRead < frames) {
    void *bufferin;
    if (ma_pcm_rb_acquire_read(buffer, &framesToRead, &bufferin) !=
        MA_SUCCESS) {
      log::t("[middleware] [{}: {}] Failed to acquire read on buffer",
             output->name, name);
      return;
    }
    memcpy(static_cast<uint8_t *>(outBuff) +
               (framesRead * context.framesSizeInBytes),
           bufferin, framesToRead * context.framesSizeInBytes);
    ma_pcm_rb_commit_read(buffer, framesToRead);
    log::t("[middleware] [{}: {}] Read {} frames", framesToRead, output->name,
           name);
    framesRead += framesToRead;
    framesToRead = frames - framesRead;
  }
}

uint32_t Middleware::availableWrite() {
  return ma_pcm_rb_available_write(buffer);
}
uint32_t Middleware::availableRead() {
  return ma_pcm_rb_available_read(buffer);
}

void Middleware::start() {
  log::i("[middleware] [{}: {}] Starting", output->name, name);
}

void Middleware::stop() {
  log::i("[middleware] [{}: {}] Stopping", output->name, name);
}

} // namespace swav
