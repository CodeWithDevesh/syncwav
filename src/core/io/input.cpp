#include <syncwav/io/sinks.h>
#include <syncwav/log.h>

namespace swav {
Input::Input(const char *name, Context &context)
    : name(name), context(context) {
  log::i("Initializing Input: {}", name);
}
} // namespace swav
