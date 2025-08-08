#include <syncwav/io/CaptureInput.h>
#include <syncwav/log.h>
#include <syncwav/context.h>

namespace swav {
	CaptureInput::CaptureInput(ma_device_id* id) : Input("Capture Input") {
		log::i("Configuring capture input device");
		const Context& globalContext = getGlobalContext();
		device = new ma_device();
		ma_device_config config = ma_device_config_init(ma_device_type_capture);
		config.capture.pDeviceID = id;
		config.capture.format = globalContext.format;
		config.capture.channels = globalContext.channels;
		config.sampleRate = globalContext.sampleRate;
		config.dataCallback = &CaptureInput::staticLoopback;
		config.pUserData = this;
		config.periodSizeInFrames = globalContext.sampleRate / 100;
		// config.wasapi.noAutoConvertSRC = true;

		ma_result result = ma_device_init(globalContext.maContext, &config, device);
		if (result != MA_SUCCESS) {
			log::e("Error while initializing input capture device");
			throw std::runtime_error("input capture not initialized");
		}

		log::i("Capture Input successfully initialized");
	}

	void CaptureInput::loopback(ma_device* pDevice, void* pOutput,
		const void* pInput, ma_uint32 frameCount) {
		const Context& globalContext = getGlobalContext();
		for (std::shared_ptr<Output> output : globalContext.outputs) {
			output->write(pInput, frameCount);
		}
	}

	void CaptureInput::staticLoopback(ma_device* pDevice, void* pOutput,
		const void* pInput, ma_uint32 frameCount) {
		CaptureInput* instance = static_cast<CaptureInput*>(pDevice->pUserData);
		if (instance) {
			instance->loopback(pDevice, pOutput, pInput, frameCount);
		}
	}

	void CaptureInput::stop() {
		log::i("Stopping input: {}", name);
		ma_device_stop(device);
	}

	void CaptureInput::start() {
		log::i("Starting input: {}", name);
		ma_device_start(device);
	}

	CaptureInput::~CaptureInput() {
		log::i("Uninitializing the Capture Input");
		if (device) {
			ma_device_uninit(device);
			delete device;
		}
	}
}