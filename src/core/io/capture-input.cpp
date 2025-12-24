#include <syncwav/context.h>
#include <syncwav/io/capture-input.h>
#include <syncwav/log.h>

namespace swav {
CaptureInput::CaptureInput(Context &context, ma_device_id *id)
    : Input("Capture Input", context) {
  log::i("Configuring capture input device");
  device = new ma_device();
  ma_device_config config = ma_device_config_init(ma_device_type_capture);
  config.capture.pDeviceID = id;
  config.capture.format = toMiniaudioFormat(context.format);
  config.capture.channels = context.channels;
  config.sampleRate = context.sampleRate;
  config.dataCallback = &CaptureInput::staticLoopback;
  config.pUserData = this;

  ma_result result = ma_device_init(NULL, &config, device);
  if (result != MA_SUCCESS) {
    log::e("Error while initializing input capture device");
    throw std::runtime_error("input capture not initialized");
  }

  log::i("Capture Input successfully initialized");
}

void CaptureInput::loopback(ma_device *pDevice, void *pOutput,
                            const void *pInput, ma_uint32 frameCount) {
  for (std::shared_ptr<Output> output : context.outputs) {
    output->write(pInput, frameCount);
  }
}

void CaptureInput::staticLoopback(ma_device *pDevice, void *pOutput,
                                  const void *pInput, ma_uint32 frameCount) {
  CaptureInput *instance = static_cast<CaptureInput *>(pDevice->pUserData);
  if (instance) {
    instance->loopback(pDevice, pOutput, pInput, frameCount);
  }
}

void CaptureInput::stop() {
  Input::stop();
  ma_device_stop(device);
}

void CaptureInput::start() {
  Input::start();
  ma_device_start(device);
}

CaptureInput::~CaptureInput() {
  log::i("Uninitializing the Capture Input");
  if (device) {
    ma_device_uninit(device);
    delete device;
  }
}
} // namespace swav
