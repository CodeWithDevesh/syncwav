#include "syncwav/utils.h"
#include <syncwav/context.h>
#include <syncwav/io/sinks.h>
#include <syncwav/log.h>

namespace swav {
void Output::write(const void *data, uint32_t noOfFrames) {
  const Context &globalContext = getGlobalContext();
  void *bufferOut;
  ma_uint32 framesToWrite = noOfFrames;
  ma_uint32 framesWritten = 0;
  noOfFrames = std::min(noOfFrames, availableWrite());
  log::t("Writing {} frames", noOfFrames);
  while (framesWritten < noOfFrames) {
    if (ma_pcm_rb_acquire_write(buffer, &framesToWrite, &bufferOut) !=
        MA_SUCCESS) {
      log::e("Failed to aquire write on {}'s buffer", name);
      return;
    }
    memcpy(bufferOut,
           static_cast<const uint8_t *>(data) +
               (framesWritten * globalContext.framesSizeInBytes),
           static_cast<size_t>(framesToWrite) *
               globalContext.framesSizeInBytes);
    ma_pcm_rb_commit_write(buffer, framesToWrite);
    log::t("Written {} frames", framesToWrite);
    framesWritten += framesToWrite;
    framesToWrite = noOfFrames - framesWritten;
  }
}

void Output::read(void *outBuff, uint32_t frames) {
  const Context &globalContext = getGlobalContext();
  ma_uint32 framesToRead = frames;
  uint32_t framesRead = 0;
  frames = std::min(frames, availableRead());
  log::t("Reading {} frames", frames);
  while (framesRead < frames) {
    void *bufferin;
    if (ma_pcm_rb_acquire_read(buffer, &framesToRead, &bufferin) !=
        MA_SUCCESS) {
      log::t("Failed to acquire read on buffer");
      return;
    }
    memcpy(static_cast<uint8_t *>(outBuff) +
               (framesRead * globalContext.framesSizeInBytes),
           bufferin, framesToRead * globalContext.framesSizeInBytes);
    ma_pcm_rb_commit_read(buffer, framesToRead);
    log::t("Read {} frames", framesToRead);
    framesRead += framesToRead;
    framesToRead = frames - framesRead;
  }
}

uint32_t Output::availableWrite() { return ma_pcm_rb_available_write(buffer); }
uint32_t Output::availableRead() { return ma_pcm_rb_available_read(buffer); }

Output::Output(const char *name, uint32_t bufferSizeInFrames) : name(name) {
  log::i("Initializing Output: {}", name);
  const Context &globalContext = getGlobalContext();
  log::i("Allocating buffer for {}", name);
  buffer = new ma_pcm_rb();
  if (ma_pcm_rb_init(globalContext.format, globalContext.channels,
                     bufferSizeInFrames, NULL, NULL, buffer) != MA_SUCCESS) {
    log::c("Failed to allocate buffer for {}", name);
    std::exit(-1);
  }
  log::i("Buffer for {} allocated successfully", name);
}

Output::~Output() {
  if (buffer) {
    ma_pcm_rb_uninit(buffer);
    delete buffer;
  }
}
} // namespace swav
