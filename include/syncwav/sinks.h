#pragma once
#include "miniaudio.h"
#include "export.h"
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

	class SWAV_API LocalOutput : public Output {
	public:
		LocalOutput(ma_device_id*, ma_uint32 bufferSizeInFrames);
		~LocalOutput();
		static void staticLoopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
		void start() override;
		void stop() override;

	private:
		ma_device* device;

	private:
		void loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint64 frameCount);
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

	class SWAV_API LoopbackInput : public Input {
	public:
		LoopbackInput(ma_device_id*);
		~LoopbackInput();
		static void staticLoopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
		void stop() override;
		void start() override;

	private:
		ma_device* device;

	private:
		void loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
	};

	class SWAV_API CaptureInput : public Input {
	public:
		CaptureInput(ma_device_id*);
		~CaptureInput();
		static void staticLoopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
		void stop() override;
		void start() override;

	private:
		ma_device* device;

	private:
		void loopback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
	};

	class SWAV_API TCPInput : public Input {
	public:
		TCPInput(const char* host, int port);
		~TCPInput();
		void stop() override;
		void start() override;

	private:
		std::thread *clientThread;
	};

}
