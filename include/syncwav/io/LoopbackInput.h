#pragma once

#include "Sinks.h"

namespace swav {
class SWAV_API LoopbackInput : public Input {
public:
  LoopbackInput(Context &context, ma_device_id *);
  ~LoopbackInput();
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
