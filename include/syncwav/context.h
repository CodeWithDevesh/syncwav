#pragma once
#include "miniaudio.h"
#include <memory>
#include "export.h"
#include "io/sinks.h"
#include <vector>

namespace swav {
	struct SWAV_API Context {
		ma_uint32 framesSizeInBytes;
		ma_uint32 channels;
		ma_format format;
		ma_uint32 sampleRate;
		ma_context* maContext;
		std::vector<std::shared_ptr<Output>> outputs;
		std::shared_ptr<Input> input;
	};

	struct SWAV_API ContextConfig {
		ma_uint32 channels = 2;
		ma_format format = ma_format_f32;
		ma_uint32 sampleRate = 48000;
	};

	SWAV_API void init(const ContextConfig& config);
	SWAV_API void uninit();
	SWAV_API const Context& getGlobalContext();
	SWAV_API void setInput(std::shared_ptr<Input>);
	SWAV_API void addOutput(std::shared_ptr<Output>);
	SWAV_API void removeOutput(std::shared_ptr<Output>);

	SWAV_API void stop();
	SWAV_API void start();
	SWAV_API bool isStopped();

	SWAV_API bool isInitialized();
}
