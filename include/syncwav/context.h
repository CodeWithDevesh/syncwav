#pragma once
#include "export.h"
#include "miniaudio.h"
#include <cstdint>
#include <syncwav/format.h>
#include <sys/types.h>
#include <vector>

namespace swav {

class Output;
class Input;
class InputFactory;
class OutputFactory;

struct SWAV_API Context {
  uint32_t framesSizeInBytes;
  uint32_t channels;
  uint32_t bufferSizeInFrames;
  AudioFormat format;
  ma_uint32 sampleRate;
  bool stopped = true;
  std::vector<Output *> outputs;
  Input *input;
};

struct SWAV_API ContextConfig {
  ma_uint32 channels = 2;
  AudioFormat format = AudioFormat::F32;
  ma_uint32 sampleRate = 48000;
  uint32_t bufferSizeInFrames = 5000;
};

SWAV_API Context init(const ContextConfig &config);
SWAV_API void uninit(Context &context);

// Can be used while the engine is running
// will restart the engine automatically
SWAV_API void setInput(Context &context, InputFactory &factory);
SWAV_API void addOutput(Context &context, OutputFactory &factory);
// TODO: Implement this function
// SWAV_API void removeOutput(Context &context, std::shared_ptr<Output>);

// Manages all the input and outputs start/stop state
SWAV_API void stop(Context &context);
SWAV_API void start(Context &context);

SWAV_API bool isStopped(Context &context);
} // namespace swav
