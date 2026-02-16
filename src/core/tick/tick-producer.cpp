#include "syncwav/format.h"
#include <syncwav/context.h>
#include <syncwav/io/input.h>
#include <syncwav/log.h>
#include <syncwav/tick/tick-producer.h>
#include <syncwav/backend/miniaudio/format.h>

namespace swav {

TickProducer::TickProducer(Context &context)
    : context(context), tickThreshold(context.bufferSizeInFrames / 2) {
  buffer = new ma_pcm_rb();
  ma_pcm_rb_init(toMiniaudioFormat(context.format), context.channels,
                 context.bufferSizeInFrames, NULL, NULL, buffer);
}

TickProducer::~TickProducer() {
  if (buffer) {
    ma_pcm_rb_uninit(buffer);
    delete (buffer);
  }
}

void TickProducer::sendTick() {
  if (!hasAuthority)
    return;

  context.input->tick(availableWrite(), this);
}

void TickProducer::write(const void *data, uint32_t frames) {
  log::t("[output] Got {} frames of data... writing to internal buffer", frames);
  void *bufferOut;
  ma_uint32 framesToWrite = frames;
  ma_uint32 framesWritten = 0;
  frames = std::min(frames, availableWrite());
  while (framesWritten < frames) {
    if (ma_pcm_rb_acquire_write(buffer, &framesToWrite, &bufferOut) !=
        MA_SUCCESS) {
      return;
    }
    memcpy(bufferOut,
           static_cast<const uint8_t *>(data) +
               (framesWritten * context.framesSizeInBytes),
           static_cast<size_t>(framesToWrite) * context.framesSizeInBytes);
    ma_pcm_rb_commit_write(buffer, framesToWrite);
    framesWritten += framesToWrite;
    framesToWrite = frames - framesWritten;
  }
}

uint32_t TickProducer::availableWrite() {
  return ma_pcm_rb_available_write(buffer);
}

uint32_t TickProducer::availableRead() {
  return ma_pcm_rb_available_read(buffer);
}

void TickProducer::read(void *buff, uint32_t &frames) {
  // Send Ticks
  log::t("[output] Reading from buffer");
  log::t("[output] Available frames {}", availableRead());
  if (availableRead() < tickThreshold) {
    log::t("[output] Available read < threshold... sending tick");
    sendTick();
  }

  ma_uint32 framesToRead = frames;
  uint32_t framesRead = 0;
  frames = std::min(frames, availableRead());
  while (framesRead < frames) {
    void *bufferin;
    if (ma_pcm_rb_acquire_read(buffer, &framesToRead, &bufferin) !=
        MA_SUCCESS) {
      return;
    }
    memcpy(static_cast<uint8_t *>(buff) +
               (framesRead * context.framesSizeInBytes),
           bufferin, framesToRead * context.framesSizeInBytes);
    ma_pcm_rb_commit_read(buffer, framesToRead);
    framesRead += framesToRead;
    framesToRead = frames - framesRead;
  }
}

} // namespace swav
