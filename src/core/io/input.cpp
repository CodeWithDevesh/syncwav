#include <syncwav/io/Sinks.h>
#include <syncwav/Log.h>

namespace swav {
Input::Input(const char *name, Context &context)
    : name(name), context(context) {
  log::i("Initializing Input: {}", name);
}

void Input::start() { log::i("Starting Input: {}", name); }

void Input::stop() { log::i("Stopping Input: {}", name); }
} // namespace swav
