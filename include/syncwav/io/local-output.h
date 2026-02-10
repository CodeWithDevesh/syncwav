#pragma once
#include "output.h"
#include <syncwav/backend/miniaudio/device.h>

namespace swav {
class SWAV_API LocalOutput : public Output {
public:
  LocalOutput(Context &, Device);
  ~LocalOutput();
  static void staticLoopback(ma_device *pDevice, void *pOutput,
                             const void *pInput, ma_uint32 frameCount);
  void start() override;
  void stop() override;

private:
  ma_device *device;

private:
  void loopback(ma_device *pDevice, void *pOutput, const void *pInput,
                uint32_t frameCount);
};
} // namespace swav
