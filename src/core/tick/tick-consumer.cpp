#include "syncwav/format.h"
#include <syncwav/context.h>
#include <syncwav/io/output.h>
#include <syncwav/log.h>
#include <syncwav/tick/tick-consumer.h>
#include <syncwav/tick/tick-producer.h>
#include <syncwav/backend/miniaudio/format.h>

namespace swav {

TickConsumer::TickConsumer(Context &context) : context(context) {
  buffer = new ma_pcm_rb();
  if (ma_pcm_rb_init(toMiniaudioFormat(context.format), context.channels,
                     context.bufferSizeInFrames, NULL, NULL,
                     buffer) != MA_SUCCESS) {
    std::exit(-1);
  }

  tempBuffer = malloc(ma_get_bytes_per_frame(toMiniaudioFormat(context.format),
                                             context.channels) *
                      context.bufferSizeInFrames);
}

TickConsumer::~TickConsumer() {
  stop();
  if (buffer) {
    ma_pcm_rb_uninit(buffer);
    delete buffer;
  }
  free(tempBuffer);
}

uint32_t TickConsumer::availableWrite() const {
  return ma_pcm_rb_available_write(buffer);
}

uint32_t TickConsumer::availableRead() const {
  return ma_pcm_rb_available_read(buffer);
}

void TickConsumer::write(const void *data, uint32_t noOfFrames) {
  log::t("[input] Got {} frames of data... writing to internal buffer", noOfFrames);
  void *bufferOut;
  ma_uint32 framesToWrite = noOfFrames;
  ma_uint32 framesWritten = 0;
  noOfFrames = std::min(noOfFrames, availableWrite());
  while (framesWritten < noOfFrames) {
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
    framesToWrite = noOfFrames - framesWritten;
  }
}

void TickConsumer::read(void *buff, uint32_t &frames) {
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

void TickConsumer::tick(uint32_t frames, TickProducer *producer) {
  log::t("[input] Got tick for {} frames", frames);
  if (stopped)
    return;
  if (producer != authorisedProducer)
    return;
  // read the data into the temp buffer
  // frames will be changed to the number of frames actually read
  read(tempBuffer, frames);
  log::t("[input] Read {} frames, writing to op buffers", frames);

  for (auto source : context.outputs) {
    source->write(tempBuffer, frames);
  }
}

void TickConsumer::reAuthorize() {
  bool s = stopped;
  // TODO: context.stop();
  int ind = 0;
  for (int i = 0; i < context.outputs.size(); i++) {
    if (context.outputs[i]->getPriority() > context.outputs[ind]->getPriority())
      ind = i;
  }
  for (int i = 0; i < context.outputs.size(); i++) {
    context.outputs[i]->setAuthority(false);
    if (i == ind) {
      context.outputs[i]->setAuthority(true);
      authorisedProducer = context.outputs[i];
    }
  }
  /* TODO:
  if (!s)
    context.start();
  */
}

} // namespace swav
