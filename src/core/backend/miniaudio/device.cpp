#include <syncwav/backend/miniaudio/device.h>
#include <syncwav/log.h>

namespace swav {

ma_device_id resolveDevice(Device device) {
  ma_context context;
  ma_context_init(NULL, 0, NULL, &context);

  ma_device_info *captureInfo, *playbackInfo;
  ma_uint32 captureCnt, playbackCnt;

  log::i("Resolving device: {}-{}",
         device.type == DeviceType::PLAYBACK ? "Playback" : "Capture",
         device.id);

  ma_context_get_devices(&context, &playbackInfo, &playbackCnt, &captureInfo,
                         &captureCnt);
  ma_device_id id;

  log::i("Getting devices...");
  log::i("got playback devices count: {} and capture devices count: {}",
         playbackCnt, captureCnt);
  if (device.type == DeviceType::PLAYBACK)
    id = playbackInfo[device.id].id;
  if (device.type == DeviceType::CAPTURE)
    id = captureInfo[device.id].id;
  ma_context_uninit(&context);
  return id;
}

} // namespace swav
