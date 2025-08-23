#define MINIAUDIO_IMPLEMENTATION
#include <App.h>
#include <miniaudio.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <csignal>
#include <atomic>
#include <logger.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httpLib.h>


uWS::SSLApp* globalApp;
uWS::Loop* globalLoop;
std::atomic<bool> isRunning(true);

typedef struct {
	ma_pcm_rb buffer;
	void* intermediateBuffer;
	ma_uint32 frameSizeInBytes;
} SyncWaveContext;

struct PerSocketData {
	/* Fill with user data */
};

void handle_sigint(int) {
	std::cout << "Received termination signal, shutting down..." << std::endl;
	isRunning = false;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

	SyncWaveContext* ctx = (SyncWaveContext*)pDevice->pUserData;
	void* bufferOut;
	ma_uint32 framesToWrite = frameCount;
	//info("Received {} frames of audio data.", frameCount);
	if (ma_pcm_rb_acquire_write(&ctx->buffer, &framesToWrite, &bufferOut) !=
		MA_SUCCESS) {
		warn("Failed to aquire write on buffer");
		return;
	}
	if (framesToWrite < frameCount) {
		warn("Buffer full, dropped {} frames.", (frameCount - framesToWrite));
	}


	memcpy(bufferOut, pInput,
		static_cast<size_t>(framesToWrite) * ctx->frameSizeInBytes);
	ma_pcm_rb_commit_write(&ctx->buffer, framesToWrite);
	(void)pOutput;
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, handle_sigint); // Ctrl+C on Unix/Windows
	std::signal(SIGTERM, handle_sigint); // kill signals on Unix
	ma_result result;
	ma_encoder_config encoderConfig;
	ma_device_config deviceConfig;
	ma_device device;


	ma_backend backends[] = {
		ma_backend_wasapi
	};

	SyncWaveContext ctxt{};
	result =
		ma_pcm_rb_init(ma_format_f32, 2, 16 * 48000, NULL, NULL, &(ctxt.buffer));
	if (result != MA_SUCCESS) {
		crit("Error: %s", ma_result_description(result));
		return -1;
	}
	ctxt.frameSizeInBytes = ma_get_bytes_per_frame(ma_format_f32, 2);

	ctxt.intermediateBuffer = malloc(16 * 48000 * ctxt.frameSizeInBytes);


	ma_context context;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_context_init(backends, sizeof(backends) / sizeof(backends[0]), NULL, &context);
	ma_context_get_devices(&context, &pCaptureDeviceInfos,
		&captureDeviceCount, NULL,
		NULL);


	deviceConfig = ma_device_config_init(ma_device_type_loopback);
	//deviceConfig.capture.pDeviceID = &(pCaptureDeviceInfos[2].id);
	deviceConfig.capture.format = ma_format_f32;
	deviceConfig.capture.channels = 2;
	deviceConfig.sampleRate = 48000;
	deviceConfig.dataCallback = data_callback;
	deviceConfig.pUserData = &ctxt;

	result = ma_device_init(&context, &deviceConfig, &device);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize loopback device.\n");
		return -2;
	}


	std::thread appThread([] {
		uWS::SSLApp app = uWS::SSLApp({
			.key_file_name = "./misc/key.pem",
			.cert_file_name = "./misc/cert.pem",
			});
		app.ws<PerSocketData>("/*", {
			/* Settings */
			.compression = uWS::DISABLED,
			.maxPayloadLength = 16 * 1024 * 1024,
			.idleTimeout = 16,
			.maxBackpressure = 1 * 1024 * 1024,
			.closeOnBackpressureLimit = false,
			.resetIdleTimeoutOnSend = false,
			.sendPingsAutomatically = true,
			/* Handlers */
			.upgrade = nullptr,
			.open = [](auto* ws) {
				/* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
				ws->subscribe("broadcast");
			},
			.message = [](auto*/*ws*/, std::string_view /*message*/, uWS::OpCode /*opCode*/) {

			},
			.drain = [](auto*/*ws*/) {
				/* Check ws->getBufferedAmount() here */
			},
			.ping = [](auto*/*ws*/, std::string_view) {
				/* Not implemented yet */
			},
			.pong = [](auto*/*ws*/, std::string_view) {
				/* Not implemented yet */
			},
			.close = [](auto*/*ws*/, int /*code*/, std::string_view /*message*/) {
				/* You may access ws->getUserData() here */
			}
			});

		app.listen("0.0.0.0", 9001, [](auto* listen_socket) {
			if (listen_socket) {
				std::cout << "Listening on port " << 9001 << std::endl;
			}
			else {
				std::cerr << "Failed to listen on port 9001" << std::endl;
				isRunning = false;
			}
			});
		globalApp = &app;
		globalLoop = uWS::Loop::get();
		if (isRunning)
			app.run();
		});

	// Wait for the app to start
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::thread broadcastThread([&] {
		using namespace std::chrono;
		auto now = steady_clock::now();
		steady_clock::time_point nextTick = now + 10ms;
		while (isRunning) {
			//send10msAudioChunk();
			std::this_thread::sleep_until(nextTick);

			ma_uint32 framesAvailable = ma_pcm_rb_available_read(&ctxt.buffer);

			if (framesAvailable < 480) {
				//warn("Not enough frames available in buffer: {}. Waiting for more data.", framesAvailable);
				nextTick += 5ms;
				continue;
			}
			framesAvailable /= 480;

			ma_uint32 framesToSend = ((framesAvailable * 4) / 4);
			framesToSend *= 480;
			//info("Gonna send {} frames of audio data.", framesToSend);
			void* buffer;
			if (ma_pcm_rb_acquire_read(&ctxt.buffer, &framesToSend, &buffer) != MA_SUCCESS) {
				warn("Failed to aquire read on buffer");
				return;
			}
			memcpy(ctxt.intermediateBuffer, buffer, framesToSend * ctxt.frameSizeInBytes);
			ma_pcm_rb_commit_read(&ctxt.buffer, framesToSend);

			globalLoop->defer([&] {
				// Modify the publish call to match the expected argument types for uWS::App::publish
				globalApp->publish("broadcast", std::string_view(reinterpret_cast<const char*>(ctxt.intermediateBuffer), framesToSend * ctxt.frameSizeInBytes), uWS::OpCode::BINARY, false);
				//info("Broadcasting at: {}", steady_clock::now().time_since_epoch().count());
				});
			nextTick = steady_clock::now() + 5ms;
		}
		});

	result = ma_device_start(&device);

	httplib::SSLServer svr("./misc/cert.pem", "./misc/key.pem");

	if (!svr.is_valid()) {
		std::cerr << "Server setup failed. Possibly cert/key issue." << std::endl;
		return -1;
	}
	std::thread httpThread([&] {
		svr.set_mount_point("/", "./frontend");
		svr.listen("0.0.0.0", 6384);
		});


	if (result != MA_SUCCESS) {
		ma_device_uninit(&device);
		printf("Failed to start device.\n");
		return -3;
	}

	while (isRunning) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	broadcastThread.join();
	svr.stop();
	httpThread.join();



	if (globalLoop) {
		globalLoop->defer([] {
			globalApp->close();
			});
	}
	appThread.join();


	std::cout << "Stopping gracefully..." << std::endl;

	ma_device_uninit(&device);

	return 0;
}
