#include <stdexcept>
#include <syncwav/context.h>
#include <syncwav/io/loopback-input.h>
#include <syncwav/io/output.h>
#include <syncwav/log.h>

#ifdef SWAV_ENABLE_LOOPBACK
#include "syncwav/backend/miniaudio/format.h"
#endif

namespace swav {
LoopbackInput::LoopbackInput(Context &context, Device dev)
    : Input("Loopback Input", context) {
  log::i("Configuring loopback input device");
#ifdef SWAV_ENABLE_LOOPBACK
  device = new ma_device();
  ma_device_id id = resolveDevice(dev);
  ma_device_config config = ma_device_config_init(ma_device_type_loopback);
  config.capture.pDeviceID = &id;
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
#else
  throw std::runtime_error("syncwav was compiled without loopback support. "
                           "loopback input is disabled.");
#endif
}

void LoopbackInput::loopback(ma_device *pDevice, void *pOutput,
                             const void *pInput, ma_uint32 frameCount) {
#ifdef SWAV_ENABLE_LOOPBACK
  write(pInput, frameCount);
#else
  throw std::runtime_error("syncwav was compiled without loopback support. "
                           "loopback input is disabled.");
#endif
}

void LoopbackInput::staticLoopback(ma_device *pDevice, void *pOutput,
                                   const void *pInput, ma_uint32 frameCount) {
#ifdef SWAV_ENABLE_LOOPBACK
  LoopbackInput *instance = static_cast<LoopbackInput *>(pDevice->pUserData);
  if (instance) {
    instance->loopback(pDevice, pOutput, pInput, frameCount);
  }
#else
  throw std::runtime_error("syncwav was compiled without loopback support. "
                           "loopback input is disabled.");
#endif
}

void LoopbackInput::stop() {
#ifdef SWAV_ENABLE_LOOPBACK
  log::i("Stopping input: {}", name);
  ma_device_stop(device);
#else
  throw std::runtime_error("syncwav was compiled without loopback support. "
                           "loopback input is disabled.");
#endif
}

void LoopbackInput::start() {
#ifdef SWAV_ENABLE_LOOPBACK
  Input::start();
  ma_device_start(device);
#else
  throw std::runtime_error("syncwav was compiled without loopback support. "
                           "loopback input is disabled.");
#endif
}

LoopbackInput::~LoopbackInput() {
#ifdef SWAV_ENABLE_LOOPBACK
  Input::stop();
  if (device) {
    ma_device_uninit(device);
    delete device;
  }
#endif
}
} // namespace swav
