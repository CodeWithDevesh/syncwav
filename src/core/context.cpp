#include "syncwav/format.h"
#include <algorithm>
#include <stdexcept>
#include <syncwav/context.h>
#include <syncwav/io/sinks.h>
#include <syncwav/log.h>

namespace swav {

SWAV_API Context init(const ContextConfig &config) {
  log::i("Initializing the context...");
  Context context;
  context.channels = config.channels;
  context.format = config.format;
  context.sampleRate = config.sampleRate;
  context.framesSizeInBytes =
      ma_get_bytes_per_frame(toMiniaudioFormat(config.format), config.channels);
  context.maContext = new ma_context();
  ma_context_init(NULL, 0, NULL, (context.maContext));
  log::i("Context Initialized successfully");

  return context;
}

SWAV_API void uninit(Context &context) {
  log::i("Uninitializing the context...");
  stop(context);
  ma_context_uninit(context.maContext);
  log::i("Context uninitialized successfully");
}

SWAV_API void setInput(Context &context, std::shared_ptr<Input> input) {
  if (!context.stopped) {
    stop(context);
    context.input = input;
    start(context);
  } else {
    context.input = input;
  }
  log::i("Input set successfully.");
}

SWAV_API void addOutput(Context &context, std::shared_ptr<Output> output) {
  for (auto &o : context.outputs) {
    if (o.get() == output.get()) {
      log::w("Output already added... skipping add.");
      return;
    }
  }
  if (!context.stopped) {
    stop(context);
    context.outputs.push_back(output);
    start(context);
  } else {
    context.outputs.push_back(output);
  }
  log::i("Output added successfully");
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
    stop(context);
    remover();
    start(context);
  } else {
    remover();
  }
  log::i("Output removed successfully");
}

SWAV_API void start(Context &context) {
  if (!context.input) {
    log::e("start called before input was set");
    throw std::runtime_error("start called before input was set");
  }
  if (context.outputs.size() == 0) {
    log::e("start called before adding an output");
    throw std::runtime_error("start called before adding an output");
  }
  if (context.stopped) {
    log::i("Starting syncwav engine...");
    for (auto &o : context.outputs) {
      o->start();
    }
    context.input->start();
    context.stopped = false;
    log::i("Started the engine successfully!!...");
  }
}

SWAV_API void stop(Context &context) {
  if (!context.stopped) {
    log::i("Stopping syncwav engine...");
    if (context.input)
      context.input->stop();
    for (auto &o : context.outputs) {
      o->stop();
    }
    context.stopped = true;
    log::i("Stopped the engine successfully!!...");
  }
}

} // namespace swav
