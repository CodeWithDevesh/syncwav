#pragma once
#include <string>
#include <syncwav/Export.h>
#include <miniaudio.h>
#include <vector>
#define SYNCWAV_VERSION "1.0.0"


namespace swav {

	enum class DeviceType {
		CAPTURE,
		PLAYBACK
	};

	struct Device {
		ma_device_id id;
		std::string name;
		bool isDefault;
		DeviceType type;
	};

	inline std::string getVersion() {
		return SYNCWAV_VERSION;
	}

	SWAV_API std::vector<Device> getPlaybackDevices();
	SWAV_API std::vector<Device> getCaptureDevices();
	SWAV_API std::vector<Device> getAllDevices();

}
