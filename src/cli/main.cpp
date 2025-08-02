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
    ma_device_id *id = NULL;
    std::vector<swav::Device> devices;
    if (device > -1) {
      devices = swav::getPlaybackDevices();
      id = &(devices[device].id);
    }

    input = std::make_shared<swav::LoopbackInput>(id);
  } else if (mode == INPUT_MODE::CAPTURE) {
    ma_device_id *id = NULL;
    std::vector<swav::Device> devices;
    if (device > -1) {
      devices = swav::getCaptureDevices();
      id = &(devices[device].id);
    }

    input = std::make_shared<swav::CaptureInput>(id);
  } else {
    std::exit(0);
  }
  return input;
}

std::shared_ptr<swav::Output> getOutput(OUTPUT_MODE mode, int device) {
  std::shared_ptr<swav::Output> output;
  if (mode == OUTPUT_MODE::LOCAL) {
    ma_device_id *id = NULL;
    std::vector<swav::Device> devices;
    if (device > -1) {
      devices = swav::getPlaybackDevices();
      id = &(devices[device].id);
    }
    output = std::make_shared<swav::LocalOutput>(
        id, 4 * (swav::getGlobalContext().sampleRate / 100));
  } else {
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

int setup(int argc, char **argv) {
  CLI::App app("Syncwav");
  argv = app.ensure_utf8(argv);

  bool show_version = false;
  bool list_devices = false;
  int inputDevice = -1;
  int outputDevice = -1;

  INPUT_MODE inputMode = INPUT_MODE::NONE;
  std::map<std::string, INPUT_MODE> inputMap{{"loopback", INPUT_MODE::LOOPBACK},
                                             {"capture", INPUT_MODE::CAPTURE}};
  app.add_option("--input", inputMode, "Input mode")
      ->transform(CLI::CheckedTransformer(inputMap));

  std::vector<OUTPUT_MODE> outputModes;
  std::map<std::string, OUTPUT_MODE> outputMap{{"local", OUTPUT_MODE::LOCAL},
                                               {"tcp", OUTPUT_MODE::TCP},
                                               {"udp", OUTPUT_MODE::UDP}};
  app.add_option("--output", outputModes, "Output modes")
      ->transform(CLI::CheckedTransformer(outputMap))
      ->expected(0, (int)OUTPUT_MODE::OUTPUT_MODE_COUNT);

  app.add_option("--inputDevice", inputDevice, "Optional input device id");
  app.add_option("--device", outputDevice,
                 "Output device id (required if --outputs includes 'local')");

  app.add_flag("--list-devices,-l", list_devices, "List all devices");
  app.add_flag("--version,-v", show_version, "Print the version");

  app.callback([&]() {
    if (list_devices)
      return;
    if (show_version)
      return;

    if (inputMode == INPUT_MODE::NONE) {
      throw CLI::RequiredError("--input");
    }

    if (outputModes.empty()) {
      throw CLI::RequiredError("--output");
    }

    if (std::find(outputModes.begin(), outputModes.end(), OUTPUT_MODE::LOCAL) !=
            outputModes.end() &&
        outputDevice < 0) {
      throw CLI::ValidationError(
          "You must provide --device when using 'local' output.");
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

int main(int argc, char **argv) {
  return setup(argc, argv);
  // start(INPUT_MODE::CAPTURE, { OUTPUT_MODE::LOCAL }, 2, 1);
}
