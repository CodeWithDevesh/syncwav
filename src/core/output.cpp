#include <syncwav/sinks.h>
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



	// ----------------------------- LocalOutput ----------------------------------------

	LocalOutput::LocalOutput(ma_device_id* id, ma_uint32 bufferSizeInFrames) :
		Output("Local Output", bufferSizeInFrames) {
		log::i("Configuring local output device");
		const Context& globalContext = getGlobalContext();
		device = new ma_device();
		ma_device_config config = ma_device_config_init(ma_device_type_playback);
		config.playback.pDeviceID = id;
		config.playback.format = globalContext.format;
		config.playback.channels = globalContext.channels;
		config.sampleRate = globalContext.sampleRate;
		config.dataCallback = &LocalOutput::staticLoopback;
    config.periodSizeInFrames = globalContext.sampleRate / 100;
		config.pUserData = this;

		ma_result result = ma_device_init(globalContext.maContext, &config, device);
		if (result != MA_SUCCESS) {
			log::e("Error while initializing local output device");
			std::exit(-1);
		}

		log::i("Local Output successfully initialized");
	}

	void LocalOutput::loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint64 frameCount) {
		const Context& globalContext = getGlobalContext();
		void* bufferIn;
		ma_uint32 framesToRead = frameCount;
		if (ma_pcm_rb_acquire_read(buffer, &framesToRead, &bufferIn) !=
			MA_SUCCESS) {
			log::t("Failed to acquire read on buffer");
			return;
		}
		if (framesToRead < frameCount) {
			log::t("Buffer empty, dropped {} frames.", (frameCount - framesToRead));
		}

		memcpy(pOutput, bufferIn,
			static_cast<size_t>(framesToRead) * globalContext.framesSizeInBytes);
		ma_pcm_rb_commit_read(buffer, framesToRead);

		(void)pInput;
	}

	void LocalOutput::staticLoopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
		LocalOutput* instance = static_cast<LocalOutput*>(pDevice->pUserData);
		if (instance) {
			instance->loopback(pDevice, pOutput, pInput, frameCount);
		}
	}

	void LocalOutput::stop() {
		log::i("Stopping output device");
		ma_device_stop(device);
	}

	void LocalOutput::start() {
		log::i("Starting output device");
		ma_device_start(device);
	}

	LocalOutput::~LocalOutput() {
		if (device) {
			log::i("Uninitializing the output device");
			ma_device_uninit(device);
			delete device;
		}
	}
}
