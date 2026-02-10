#include "syncwav/context.h"
#include "syncwav/format.h"
#include "syncwav/log.h"
#include "syncwav/utils.h"
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <syncwav/core.h>
#include <syncwav/factories/input-factory.h>
#include <syncwav/factories/output-factory.h>
#include <syncwav/platform/device.h>

enum class INPUT_MODE {
  LOOPBACK,
  CAPTURE,
  FILE,
  TCP,
  INPUT_MODE_COUNT, // Always keep these two at the end
  NONE
};

enum class OUTPUT_MODE {
  LOCAL,
  TCP,
  UDP,
  OUTPUT_MODE_COUNT, // Always keep this at end
};

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
    if (device.type == swav::DeviceType::PLAYBACK) {
      if (device.isDefault)
        std::cout << "\t" << i << ". " << device.name
                  << "\033[32m  -> (default)\033[m\n";
      else
        std::cout << "\t" << i << ". " << device.name << "\n";
    }
  }

  devices = swav::getCaptureDevices();
  deviceCnt = devices.size();
  std::cout << "Capture Devices: \n";
  for (int i = 0; i < deviceCnt; i++) {
    auto device = devices[i];
    if (device.type == swav::DeviceType::CAPTURE) {
      if (device.isDefault)
        std::cout << "\t" << i << ". " << device.name
                  << "\033[32m  -> (default)\033[m\n";
      else
        std::cout << "\t" << i << ". " << device.name << "\n";
    }
  }
  std::exit(0);
}

void start(INPUT_MODE iMode, std::vector<OUTPUT_MODE> oModes, int iDevice,
           std::vector<int> oDevice, std::string file, std::string ip,
           int port) {
  swav::ContextConfig config;
  config.channels = 2;
  config.format = swav::AudioFormat::S16;

  swav::Context context = swav::init(config);

  swav::CaptureInputFactory factory =
      swav::CaptureInputFactory(swav::getDefaultCaptureDevice());

  swav::setInput(context, factory);

  for (auto oMode : oModes) {
    if (oMode == OUTPUT_MODE::LOCAL)
      for (auto device : oDevice) {

        swav::LocalOutputFactory factory =
            swav::LocalOutputFactory(swav::getPlaybackDevices()[device]);
        swav::addOutput(context, factory);
      }
  }

  swav::start(context);

  std::cin.get();

  swav::log::i("got the stop signal... stopping...");

  swav::uninit(context);
}

int setup(int argc, char **argv) {
  CLI::App app(
      R"(SyncWave - Audio routing & streaming made simple

Examples:
  syncwav --input loopback --output local --device 2
  syncwav --input capture --output tcp --output udp
  syncwav --list-devices

Options:)");
  app.footer(R"(----------------------------------------------------------------
GitHub: https://github.com/syncwav/syncwav)");
  argv = app.ensure_utf8(argv);

  app.set_help_flag("-h,--help", "Show this help message and exit");
  app.usage("syncwav [OPTIONS]");

  bool show_version = false;
  bool list_devices = false;
  int inputDevice = -1;
  swav::log::LogLevel logLevel = swav::log::LogLevel::WARN;
  std::vector<int> outputDevices;
  std::string file;
  std::string ip = "0.0.0.0";
  int port = 9001;

  INPUT_MODE inputMode = INPUT_MODE::NONE;
  std::map<std::string, INPUT_MODE> inputMap{{"loopback", INPUT_MODE::LOOPBACK},
                                             {"capture", INPUT_MODE::CAPTURE},
                                             {"file", INPUT_MODE::FILE},
                                             {"tcp", INPUT_MODE::TCP}};

  CLI::Transformer inputTransformer(inputMap, CLI::ignore_case);
  inputTransformer.name("");
  inputTransformer.description("");
  app.add_option("--input,-i", inputMode)
      ->type_name("<mode>")
      ->description("Input mode: 'loopback' (system audio) or 'capture' "
                    "(microphone) or 'file'")
      ->transform(inputTransformer);

  app.add_option("--file,-f", file)
      ->type_name("file")
      ->description("File to play the audio from if file input mode is chosen");

  std::vector<OUTPUT_MODE> outputModes;
  std::map<std::string, OUTPUT_MODE> outputMap{{"local", OUTPUT_MODE::LOCAL},
                                               {"tcp", OUTPUT_MODE::TCP},
                                               {"udp", OUTPUT_MODE::UDP}};
  CLI::Transformer outputTransformer(outputMap, CLI::ignore_case);
  outputTransformer.name("");
  outputTransformer.description("");
  app.add_option("--output,-o", outputModes)
      ->description("One or more output modes: 'local', 'tcp', 'udp'")
      ->type_name("<mode>")
      ->transform(outputTransformer)
      ->expected(0, (int)OUTPUT_MODE::OUTPUT_MODE_COUNT);

  app.add_option("--inputDevice,-I", inputDevice,
                 "ID of input device (optional; defaults to system default)")
      ->type_name("<id>");
  app.add_option("--device,-d", outputDevices,
                 "IDs of output devices to play from (only works if --output "
                 "includes 'local')")
      ->type_name("<id>");

  app.add_flag("--list,-l", list_devices,
               "Print all available input/output devices and exit");
  app.add_flag("--version,-v", show_version,
               "Print version information and exit");

  app.add_option("--ip", ip, "IP for TCP input/output");
  app.add_option("--port,-p", port, "Port for TCP input/output");

  std::map<std::string, swav::log::LogLevel> logMap{
      {"trace", swav::log::LogLevel::TRACE},
      {"debug", swav::log::LogLevel::DEBUG},
      {"info", swav::log::LogLevel::INFO},
      {"warn", swav::log::LogLevel::WARN},
      {"error", swav::log::LogLevel::ERR}};
  CLI::Transformer logTransformer(logMap, CLI::ignore_case);
  logTransformer.name("");
  logTransformer.description("");
  app.add_option("--log", logLevel)
      ->description("Set Log level")
      ->type_name("<level>")
      ->transform(logTransformer);

  app.callback([&]() {
    if (list_devices)
      return;
    if (show_version)
      return;

    if (inputMode == INPUT_MODE::NONE) {
      throw CLI::RequiredError("--input");
    }

    if (inputMode == INPUT_MODE::TCP ||
        std::find(outputModes.begin(), outputModes.end(), OUTPUT_MODE::TCP) !=
            outputModes.end()) {
      if (ip.size() == 0) {
        throw CLI::RequiredError(
            "ip to connect to required!! Specify with --ip");
      }
      if (port == -1)
        throw CLI::RequiredError("port required!! Specify with -p");
    }

    if (outputModes.empty()) {
      throw CLI::RequiredError(
          "--output (specify at least one: 'local', 'tcp', or 'udp')");
    }

    if (inputMode == INPUT_MODE::FILE && file.size() <= 0) {
      throw CLI::RequiredError("--file (specify the file to play from)");
    }

    if (std::find(outputModes.begin(), outputModes.end(), OUTPUT_MODE::LOCAL) !=
            outputModes.end() &&
        outputDevices.size() <= 0) {
      throw CLI::ValidationError(
          "Missing --device: Required when using 'local' output mode.\n"
          "Run with --list-devices to see available device IDs.");
    }
  });

  CLI11_PARSE(app, argc, argv);

  swav::log::setLogLevel(logLevel);

  if (list_devices)
    listDevices();
  if (show_version)
    printVersion();

  start(inputMode, outputModes, inputDevice, outputDevices, file, ip, port);

  return 0;
}

int main(int argc, char **argv) {
  return setup(argc, argv);
  // start(INPUT_MODE::FILE, { OUTPUT_MODE::LOCAL }, -1, {1}, "test.mp3");
}
