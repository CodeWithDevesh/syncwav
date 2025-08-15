#pragma once
#include "export.h"
#include "miniaudio.h"
#include <memory>
#include <vector>
#include <syncwav/format.h>

namespace swav {

class Output;
class Input;

struct SWAV_API Context {
  ma_uint32 framesSizeInBytes;
  ma_uint32 channels;
  AudioFormat format;
  ma_uint32 sampleRate;
  ma_context *maContext;
  bool stopped = true;
  std::vector<std::shared_ptr<Output>> outputs;
  std::shared_ptr<Input> input;
};

struct SWAV_API ContextConfig {
  ma_uint32 channels = 2;
  AudioFormat format = AudioFormat::F32;
  ma_uint32 sampleRate = 48000;
};

SWAV_API Context init(const ContextConfig &config);
SWAV_API void uninit(Context &context);
SWAV_API void setInput(Context &context, std::shared_ptr<Input>);
SWAV_API void addOutput(Context &context, std::shared_ptr<Output>);
SWAV_API void removeOutput(Context &context, std::shared_ptr<Output>);

SWAV_API void stop(Context &context);
SWAV_API void start(Context &context);
SWAV_API bool isStopped(Context &context);
} // namespace swav
