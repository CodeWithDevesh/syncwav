#include <syncwav/io/input.h>
#include <syncwav/log.h>

namespace swav {
Input::Input(const char *name, Context &context)
    : TickConsumer(context), name(name) {
  log::i("Initializing Input: {}", name);
}

void Input::start() {
  log::i("Starting Input: {}", name);
  TickConsumer::start();
}

void Input::stop() {
  log::i("Stopping Input: {}", name);
  TickConsumer::stop();
}

} // namespace swav
