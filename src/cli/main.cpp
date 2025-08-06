#include "syncwav/utils.h"
#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <syncwav/core.h>

enum class INPUT_MODE {
	LOOPBACK,
	CAPTURE,
	INPUT_MODE_COUNT, // Always keep these two at the end
	NONE
};

enum class OUTPUT_MODE {
	LOCAL,
	TCP,
	UDP,
	OUTPUT_MODE_COUNT, // Always keep this at end
};

std::shared_ptr<swav::Input> getInput(INPUT_MODE mode, int device) {
	std::shared_ptr<swav::Input> input;
	if (mode == INPUT_MODE::LOOPBACK) {
		ma_device_id* id = NULL;
		std::vector<swav::Device> devices;
		if (device > -1) {
			devices = swav::getPlaybackDevices();
			id = &(devices[device].id);
		}

		input = std::make_shared<swav::LoopbackInput>(id);
	}
	else if (mode == INPUT_MODE::CAPTURE) {
		ma_device_id* id = NULL;
		std::vector<swav::Device> devices;
		if (device > -1) {
			devices = swav::getCaptureDevices();
			id = &(devices[device].id);
		}

		input = std::make_shared<swav::CaptureInput>(id);
	}
	else {
		std::exit(0);
	}
	return input;
}

std::shared_ptr<swav::Output> getOutput(OUTPUT_MODE mode, int device) {
	std::shared_ptr<swav::Output> output;
	if (mode == OUTPUT_MODE::LOCAL) {
		ma_device_id* id = NULL;
		std::vector<swav::Device> devices;
		if (device > -1) {
			devices = swav::getPlaybackDevices();
			id = &(devices[device].id);
		}
		output = std::make_shared<swav::LocalOutput>(
			id, 4 * (swav::getGlobalContext().sampleRate / 100));
	}
	else {
		std::exit(0);
	}
	return output;
}

void printVersion() {
	std::string version = swav::getVersion();
	std::cout << "SyncWav version: " << version << std::endl;
	std::exit(0);
}

void listDevices() {
	std::vector<swav::Device> devices = swav::getPlaybackDevices();
	int deviceCnt = devices.size();

	std::cout << "Playback Devices: \n";
	for (int i = 0; i < deviceCnt; i++) {
		auto device = devices[i];
		if (device.type == swav::DeviceType::PLAYBACK)
			if (device.isDefault)
				std::cout << "\t" << i << ". " << device.name
				<< "\033[32m  -> (default)\033[m\n";
			else
				std::cout << "\t" << i << ". " << device.name << "\n";
	}

	devices = swav::getCaptureDevices();
	deviceCnt = devices.size();
	std::cout << "Capture Devices: \n";
	for (int i = 0; i < deviceCnt; i++) {
		auto device = devices[i];
		if (device.type == swav::DeviceType::CAPTURE)
			if (device.isDefault)
				std::cout << "\t" << i << ". " << device.name
				<< "\033[32m  -> (default)\033[m\n";
			else
				std::cout << "\t" << i << ". " << device.name << "\n";
	}
	std::exit(0);
}

void start(INPUT_MODE iMode, std::vector<OUTPUT_MODE> oModes, int iDevice,
	int oDevice) {
	swav::ContextConfig config;
	swav::init(config);
	swav::log::i("Input device is: {} and output device is: {}", iDevice,
		oDevice);
	swav::setInput(getInput(iMode, iDevice));
	for (auto oMode : oModes) {
		swav::addOutput(getOutput(oMode, oDevice));
	}

	swav::start();

	std::cin.get();

	swav::log::i("got the stop signal... stopping...");

	swav::uninit();
}

int setup(int argc, char** argv) {
	CLI::App app(
		R"(SyncWave - Audio routing & streaming made simple

Examples:
  syncwav --input loopback --output local --device 2
  syncwav --input capture --output tcp --output udp
  syncwav --list-devices

Options:)"
);
	app.footer(R"(----------------------------------------------------------------
GitHub: https://github.com/syncwav/syncwav)");
	argv = app.ensure_utf8(argv);


	app.set_help_flag("-h,--help", "Show this help message and exit");
	app.usage("syncwav [OPTIONS]");

	bool show_version = false;
	bool list_devices = false;
	int inputDevice = -1;
	int outputDevice = -1;

	INPUT_MODE inputMode = INPUT_MODE::NONE;
	std::map<std::string, INPUT_MODE> inputMap{ {"loopback", INPUT_MODE::LOOPBACK},
											   {"capture", INPUT_MODE::CAPTURE} };

	CLI::Transformer inputTransformer(inputMap, CLI::ignore_case);
	inputTransformer.name("");
	inputTransformer.description("");
	app.add_option("--input,-i", inputMode)->type_name("<mode>")
		->description("Input mode: 'loopback' (system audio) or 'capture' (microphone)")
		->transform(inputTransformer);

	std::vector<OUTPUT_MODE> outputModes;
	std::map<std::string, OUTPUT_MODE> outputMap{ {"local", OUTPUT_MODE::LOCAL},
												 {"tcp", OUTPUT_MODE::TCP},
												 {"udp", OUTPUT_MODE::UDP} };
	CLI::Transformer outputTransformer(outputMap, CLI::ignore_case);
	outputTransformer.name("");
	outputTransformer.description("");
	app.add_option("--output,-o", outputModes)
		->description("One or more output modes: 'local', 'tcp', 'udp'")
		->type_name("<mode>")
		->transform(outputTransformer)
		->expected(0, (int)OUTPUT_MODE::OUTPUT_MODE_COUNT);

	app.add_option("--inputDevice,-I", inputDevice,
		"ID of input device (optional; defaults to system default)")->type_name("<id>");
	app.add_option("--device,-d", outputDevice,
		"ID of output device (only required if --output includes 'local')")->type_name("<id>");

	app.add_flag("--list-devices,-l", list_devices,
		"Print all available input/output devices and exit");
	app.add_flag("--version,-v", show_version,
		"Print version information and exit");


	app.callback([&]() {
		if (list_devices)
			return;
		if (show_version)
			return;

		if (inputMode == INPUT_MODE::NONE) {
			throw CLI::RequiredError("--input (choose 'loopback' or 'capture')");
		}

		if (outputModes.empty()) {
			throw CLI::RequiredError("--output (specify at least one: 'local', 'tcp', or 'udp')");
		}

		if (std::find(outputModes.begin(), outputModes.end(), OUTPUT_MODE::LOCAL) != outputModes.end() &&
			outputDevice < 0) {
			throw CLI::ValidationError("Missing --device: Required when using 'local' output mode.\n"
				"Run with --list-devices to see available device IDs.");
		}
		});

	CLI11_PARSE(app, argc, argv);

	if (list_devices)
		listDevices();
	if (show_version)
		printVersion();

	start(inputMode, outputModes, inputDevice, outputDevice);

	return 0;
}

int main(int argc, char** argv) {
	return setup(argc, argv);
	// start(INPUT_MODE::CAPTURE, { OUTPUT_MODE::LOCAL }, 2, 1);
}
