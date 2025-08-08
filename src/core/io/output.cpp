#include <syncwav/io/sinks.h>
#include <syncwav/context.h>
#include <syncwav/log.h>

namespace swav {

	// --------------------------- Output ----------------------------------------------
	void Output::write(const void* data, int noOfFrames) {
		const Context& globalContext = getGlobalContext();
		void* bufferOut;
		ma_uint32 framesToWrite = noOfFrames;
		if (ma_pcm_rb_acquire_write(buffer, &framesToWrite, &bufferOut) !=
			MA_SUCCESS) {
			log::e("Failed to aquire write on {}'s buffer", name);
			return;
		}
		if (framesToWrite < noOfFrames) {
			log::t("{}'s buffer full, dropped {} frames.", name, (noOfFrames - framesToWrite));
		}

		memcpy(bufferOut, data,
			static_cast<size_t>(framesToWrite) * globalContext.framesSizeInBytes);
		ma_pcm_rb_commit_write(buffer, framesToWrite);
	}

	Output::Output(const char* name, ma_uint32 bufferSizeInFrames) : name(name) {
		log::i("Initializing Output: {}", name);
		const Context& globalContext = getGlobalContext();
		log::i("Allocating buffer for {}", name);
		buffer = new ma_pcm_rb();
		if (ma_pcm_rb_init(globalContext.format, globalContext.channels, bufferSizeInFrames, NULL, NULL, buffer) != MA_SUCCESS) {
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
}
