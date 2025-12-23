#pragma once
#include "Sinks.h"

namespace swav {
class SWAV_API CaptureInput : public Input {
public:
  CaptureInput(Context &context, ma_device_id *);
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
