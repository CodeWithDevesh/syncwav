#pragma once

#include "sinks.h"

namespace swav {
	class SWAV_API LocalOutput : public Output {
	public:
		LocalOutput(Context& context, ma_device_id*, ma_uint32 bufferSizeInFrames);
		~LocalOutput();
		static void staticLoopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
		void start() override;
		void stop() override;

	private:
		ma_device* device;

	private:
		void loopback(ma_device* pDevice, void* pOutput, const void* pInput, uint32_t frameCount);
	};
}
