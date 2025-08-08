#include <memory>
#include <syncwav/log.h>
#include <syncwav/io/sinks.h>

namespace swav {
	Input::Input(const char* name) : name(name) {
		log::i("Initializing Input: {}", name);
	}
} // namespace swav
