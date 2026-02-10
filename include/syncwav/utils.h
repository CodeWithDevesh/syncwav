#pragma once
#include <string>
#include <syncwav/export.h>
#include <miniaudio.h>
#define SYNCWAV_VERSION "1.0.0"


namespace swav {
	inline std::string getVersion() {
		return SYNCWAV_VERSION;
	}
}
