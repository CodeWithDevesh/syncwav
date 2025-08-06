#include <syncwav/context.h>
#include <syncwav/log.h>
#include <algorithm>

namespace swav {

	std::unique_ptr<Context> globalContext = nullptr;
	bool stopped = true;

	SWAV_API void init(const ContextConfig& config) {
		log::i("Initializing the context...");
		if (isInitialized()) {
			log::e("Context is already initialized!");
			return;
		}
		globalContext = std::make_unique<Context>();
		globalContext->channels = config.channels;
		globalContext->format = config.format;
		globalContext->sampleRate = config.sampleRate;
		globalContext->framesSizeInBytes = ma_get_bytes_per_frame(config.format, config.channels);
		ma_context_init(NULL, 0, NULL, (globalContext->maContext));
		log::i("Context Initialized successfully");
	}

	SWAV_API void uninit() {
		log::i("Uninitializing the context...");
		stop();
		if (!isInitialized()) {
			return;
		}
		ma_context_uninit(globalContext->maContext);
		globalContext.reset();
		log::i("Context uninitialized successfully");
	}

	SWAV_API void setInput(std::shared_ptr<Input> input) {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return;
		}
		if (!stopped) {
			stop();
			globalContext->input = input;
			start();
		}
		else {
			globalContext->input = input;
		}
		log::i("Input set successfully.");
	}

	SWAV_API void addOutput(std::shared_ptr<Output> output) {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return;
		}
		for (auto o : globalContext->outputs) {
			if (o.get() == output.get()) {
				log::w("Output already added... skipping add.");
				return;
			}
		}
		if (!stopped) {
			stop();
			globalContext->outputs.push_back(output);
			start();
		}
		else {
			globalContext->outputs.push_back(output);
		}
		log::i("Output added successfully");
	}

	SWAV_API void removeOutput(std::shared_ptr<Output> output) {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return;
		}
		auto& vec = globalContext->outputs;
		auto remover = [&]() {
			vec.erase(std::remove_if(vec.begin(), vec.end(),
				[&](const auto& o) { return o.get() == output.get(); }), vec.end());
			};
		if (!stopped) {
			stop();
			remover();
			start();
		}
		else {
			remover();
		}
		log::i("Output removed successfully");
	}

	SWAV_API void start() {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return;
		}
		if (!globalContext->input) {
			log::e("Input not set... call setInput first");
			return;
		}
		if (globalContext->outputs.size() == 0) {
			log::e("No output added yet... call addOutput first");
			return;
		}
		if (stopped) {
			log::i("Starting the engine...");
			for (auto o : globalContext->outputs) {
				o->start();
			}
			globalContext->input->start();
			stopped = false;
		}
	}

	SWAV_API void stop() {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return;
		}
		if (!stopped) {
			if (!globalContext->input) {
				return;
			}
			globalContext->input->stop();
			for (auto o : globalContext->outputs) {
				o->stop();
			}
			stopped = true;
		}
	}

	SWAV_API bool isStopped() {
		if (!isInitialized()) {
			log::e("Context not Initialized!... Did you call init?");
			return true;
		}
		return stopped;
	}

	SWAV_API const Context& getGlobalContext() {
		if (!isInitialized()) {
			log::e("Attempted to access global context before initialization!");
			throw std::runtime_error("Context not initialized");
		}
		return *globalContext;
	}

	SWAV_API bool isInitialized() {
		return globalContext != nullptr;
	}
}
