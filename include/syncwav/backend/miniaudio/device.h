#pragma once
#include <miniaudio.h>
#include <syncwav/platform/device.h>

namespace swav {
ma_device_id resolveDevice(Device);
} // namespace swav
