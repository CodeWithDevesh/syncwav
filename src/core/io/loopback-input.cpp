#include "syncwav/format.h"
#include <syncwav/context.h>
#include <syncwav/io/loopback-input.h>
#include <syncwav/log.h>

namespace swav {
LoopbackInput::LoopbackInput(Context &context, ma_device_id *deviceId)
    : Input("Loopback Input", context) {
  log::i("Configuring loopback input device");
  device = new ma_device();
  ma_device_config config = ma_device_config_init(ma_device_type_loopback);
  config.capture.pDeviceID = deviceId;
  config.capture.format = toMiniaudioFormat(context.format);
  config.capture.channels = context.channels;
  config.sampleRate = context.sampleRate;
  config.dataCallback = &LoopbackInput::staticLoopback;
  config.pUserData = this;

  ma_backend backends[] = {ma_backend_wasapi};
  ma_result result = ma_device_init(NULL, &config, device);
  if (result != MA_SUCCESS) {
    log::e("Error while initializing input loopback device");
    throw std::runtime_error("input loopback not initialized");
  }

  log::i("Loopback Input successfully initialized");
}

void LoopbackInput::loopback(ma_device *pDevice, void *pOutput,
                             const void *pInput, ma_uint32 frameCount) {
  for (std::shared_ptr<Output> output : context.outputs) {
    output->write(pInput, frameCount);
  }
}

void LoopbackInput::staticLoopback(ma_device *pDevice, void *pOutput,
                                   const void *pInput, ma_uint32 frameCount) {
  LoopbackInput *instance = static_cast<LoopbackInput *>(pDevice->pUserData);
  if (instance) {
    instance->loopback(pDevice, pOutput, pInput, frameCount);
  }
}

void LoopbackInput::stop() {
  log::i("Stopping input: {}", name);
  ma_device_stop(device);
}

void LoopbackInput::start() {
  Input::start();
  ma_device_start(device);
}

LoopbackInput::~LoopbackInput() {
  Input::stop();
  if (device) {
    ma_device_uninit(device);
    delete device;
  }
}
} // namespace swav
