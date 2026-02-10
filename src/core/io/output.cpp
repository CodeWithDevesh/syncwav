#include "syncwav/tick/tick-producer.h"
#include <syncwav/context.h>
#include <syncwav/io/output.h>
#include <syncwav/log.h>

namespace swav {

Output::Output(const char *name, Context &context)
    : TickProducer(context), name(name) {
  log::i("[output] Initializing Output: {}", name);
  log::d("[output] target: {}, channels: {}, format: {}, buffer_size: {}", name,
         context.channels, toCString(context.format),
         context.bufferSizeInFrames * context.framesSizeInBytes);
}

Output::~Output() { log::i("[output] Uninitializing Output: {}", name); }

void Output::start() {
  TickProducer::start();
  log::i("Starting Output: {}", name);
}

void Output::stop() {
  TickProducer::stop();
  log::i("Stopping Output: {}", name);
}

} // namespace swav
