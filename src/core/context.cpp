#include "syncwav/format.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <syncwav/context.h>
#include <syncwav/io/sinks.h>
#include <syncwav/log.h>

namespace swav {

SWAV_API Context init(const ContextConfig &config) {
  log::i("[context] Initializing...");
  log::d("[context] channels: {}, sampleRate: {}, format: {}", config.channels,
         config.sampleRate, toCString(config.format));

  Context context;
  context.channels = config.channels;
  context.format = config.format;
  context.sampleRate = config.sampleRate;
  context.framesSizeInBytes =
      ma_get_bytes_per_frame(toMiniaudioFormat(config.format), config.channels);

  log::i("[context] Initialized successfully");
  log::d("[context] frameSizeInBytes: {}", context.framesSizeInBytes);
  return context;
}

SWAV_API void uninit(Context &context) {
  log::i("[context] Uninitializing...");
  stop(context);
  log::i("[context] Uninitialized successfully");
}

SWAV_API void setInput(Context &context, std::shared_ptr<Input> input) {
  if (!context.stopped) {
    log::d("[engine] Input change detected, restarting engine...");
    stop(context);
    context.input = input;
    start(context);
  } else {
    context.input = input;
  }
  log::i("[engine] Input set: {}", context.input->name);
}

SWAV_API void addOutput(Context &context, std::shared_ptr<Output> output) {
  for (auto &o : context.outputs) {
    if (o.get() == output.get()) {
      log::d("[engine] Output already present, skipping add");
      return;
    }
  }
  if (!context.stopped) {
    log::d("Adding new output, restarting engine...");
    stop(context);
    context.outputs.push_back(output);
    start(context);
  } else {
    context.outputs.push_back(output);
  }
  log::i("[engine] Output added: {}", output->name);
}

SWAV_API void removeOutput(Context &context, std::shared_ptr<Output> output) {
  auto &vec = context.outputs;
  auto remover = [&]() {
    vec.erase(
        std::remove_if(vec.begin(), vec.end(),
                       [&](const auto &o) { return o.get() == output.get(); }),
        vec.end());
  };
  if (!context.stopped) {
    log::d("[engine] Removing output, restarting engine...");
    stop(context);
    remover();
    start(context);
  } else {
    remover();
  }
  log::i("[engine] Output removed: {}", output->name);
}

SWAV_API void start(Context &context) {

  if (context.stopped) {
    log::i("[engine] Starting syncwav engine...");
    log::d("[engine] Input: {}", context.input->name);

    std::string outputs;
    for (auto &o : context.outputs) {
      outputs += o->name;
      outputs += ",";
    }
    outputs = outputs.substr(0, outputs.size() - 1);
    log::d("[engine] Outputs: {}", outputs);

    if (!context.input) {
      log::e("[engine] Start failed: no input set \n\toutputsCount: {}",
             context.outputs.size());
      throw std::runtime_error("start called before input was set");
    }
    if (context.outputs.empty()) {
      log::e("[engine] Start failed: no outputs added");
      throw std::runtime_error("start called before adding an output");
    }

    auto t0 = std::chrono::steady_clock::now();

    for (auto &o : context.outputs) {
      o->start();
    }
    context.input->start();
    context.stopped = false;

    auto elapsed = std::chrono::steady_clock::now() - t0;
    log::i("[engine] Started successfully");
    log::d(
        "Elapsed time: {}",
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
  }
}

SWAV_API void stop(Context &context) {
  if (!context.stopped) {
    log::i("[engine] Stopping syncwav engine...");
    auto t0 = std::chrono::steady_clock::now();

    if (context.input)
      context.input->stop();
    for (auto &o : context.outputs) {
      o->stop();
    }
    context.stopped = true;

    auto elapsed = std::chrono::steady_clock::now() - t0;
    log::i("[engine] Stopped successfully");
    log::d(
        "Elapsed time: {}",
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
  }
}

} // namespace swav
