#pragma once
#include "../miniaudio.h"
#include "../export.h"
#include <thread>

namespace swav {
	class SWAV_API Output {
	public:
		// This method will be called by input to write to the buffer
		void write(const void* data, int noOfFrames);

		Output(const char* name, ma_uint32 bufferSizeInFrames);

		virtual ~Output();

		virtual void start() = 0;
		virtual void stop() = 0;

	public:
		const char* name;

	protected:
		ma_pcm_rb* buffer;

	};

	class SWAV_API Input {
	public:
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual ~Input() = default;
		Input(const char* name);

	public:
		const char* name;
	};

}
