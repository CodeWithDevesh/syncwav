#include <syncwav/context.h>
#include <syncwav/io/local-output.h>
#include <syncwav/log.h>

namespace swav {
LocalOutput::LocalOutput(Context &context, ma_device_id *id,
                         ma_uint32 bufferSizeInFrames)
    : Output("Local Output", context, bufferSizeInFrames) {
  log::i("Configuring local output device");
  device = new ma_device();
  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.pDeviceID = id;
  config.playback.format = toMiniaudioFormat(context.format);
  config.playback.channels = context.channels;
  config.sampleRate = context.sampleRate;
  config.dataCallback = &LocalOutput::staticLoopback;
  config.pUserData = this;

  ma_result result = ma_device_init(NULL, &config, device);
  if (result != MA_SUCCESS) {
    log::e("Error while initializing local output device");
    std::exit(-1);
  }

  log::i("Local Output successfully initialized");
}

void LocalOutput::loopback(ma_device *pDevice, void *pOutput,
                           const void *pInput, uint32_t frameCount) {
  read(pOutput, frameCount);
}

void LocalOutput::staticLoopback(ma_device *pDevice, void *pOutput,
                                 const void *pInput, uint32_t frameCount) {
  LocalOutput *instance = static_cast<LocalOutput *>(pDevice->pUserData);
  if (instance) {
    instance->loopback(pDevice, pOutput, pInput, frameCount);
  }
}

void LocalOutput::stop() {
  Output::stop();
  ma_device_stop(device);
}

void LocalOutput::start() {
  Output::start();
  ma_device_start(device);
}

LocalOutput::~LocalOutput() {
  if (device) {
    log::i("Uninitializing the output device");
    ma_device_uninit(device);
    delete device;
  }
}
} // namespace swav
