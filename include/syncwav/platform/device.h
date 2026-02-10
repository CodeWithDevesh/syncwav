#pragma once
#include <miniaudio.h>
#include <string>
#include <syncwav/export.h>
#include <vector>

namespace swav {
enum class DeviceType { CAPTURE, PLAYBACK };

struct Device {
  int id;
  std::string name;
  bool isDefault;
  DeviceType type;
};

SWAV_API std::vector<Device> getPlaybackDevices();
SWAV_API std::vector<Device> getCaptureDevices();
SWAV_API Device getDefaultPlaybackDevice();
SWAV_API Device getDefaultCaptureDevice();
SWAV_API std::vector<Device> getAllDevices();
} // namespace swav
