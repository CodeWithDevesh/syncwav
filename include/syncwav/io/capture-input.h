#pragma once
#include "../utils.h"
#include "input.h"
#include <syncwav/backend/miniaudio/device.h>


namespace swav {
class SWAV_API CaptureInput : public Input {
public:
  CaptureInput(Context &, Device);
  ~CaptureInput();
  static void staticLoopback(ma_device *pDevice, void *pOutput,
                             const void *pInput, ma_uint32 frameCount);
  void stop() override;
  void start() override;

private:
  ma_device *device;

private:
  void loopback(ma_device *pDevice, void *pOutput, const void *pInput,
                ma_uint32 frameCount);
};
} // namespace swav
