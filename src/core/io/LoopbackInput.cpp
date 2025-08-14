#include <syncwav/io/LoopbackInput.h>
#include <syncwav/log.h>
#include <syncwav/context.h>

namespace swav {
	LoopbackInput::LoopbackInput(ma_device_id* deviceId) : Input("Loopback Input") {
		log::i("Configuring loopback input device");
		const Context& globalContext = getGlobalContext();
		device = new ma_device();
		ma_device_config config = ma_device_config_init(ma_device_type_loopback);
		config.capture.pDeviceID = deviceId;
		config.capture.format = globalContext.format;
		config.capture.channels = globalContext.channels;
		config.sampleRate = globalContext.sampleRate;
		config.dataCallback = &LoopbackInput::staticLoopback;
		config.pUserData = this;
		// config.wasapi.noAutoConvertSRC = true;

		ma_backend backends[] = { ma_backend_wasapi };
		ma_result result = ma_device_init(globalContext.maContext, &config, device);
		if (result != MA_SUCCESS) {
			log::e("Error while initializing input loopback device");
			throw std::runtime_error("input loopback not initialized");
		}

		log::i("Loopback Input successfully initialized");
	}

	void LoopbackInput::loopback(ma_device* pDevice, void* pOutput,
		const void* pInput, ma_uint32 frameCount) {
		const Context& globalContext = getGlobalContext();
		for (std::shared_ptr<Output> output : globalContext.outputs) {
			output->write(pInput, frameCount);
		}
	}

	void LoopbackInput::staticLoopback(ma_device* pDevice, void* pOutput,
		const void* pInput, ma_uint32 frameCount) {
		LoopbackInput* instance = static_cast<LoopbackInput*>(pDevice->pUserData);
		if (instance) {
			instance->loopback(pDevice, pOutput, pInput, frameCount);
		}
	}

	void LoopbackInput::stop() {
		log::i("Stopping input: {}", name);
		ma_device_stop(device);
	}

	void LoopbackInput::start() {
		log::i("Starting input: {}", name);
		ma_device_start(device);
	}

	LoopbackInput::~LoopbackInput() {
		log::i("Uninitializing the looback input");
		if (device) {
			ma_device_uninit(device);
			delete device;
		}
	}
}
