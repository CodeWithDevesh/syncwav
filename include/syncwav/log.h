#pragma once
#include "export.h"
#include <memory>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#ifndef SWAV_LOG_LEVEL
#define SWAV_LOG_LEVEL spdlog::level::trace
#endif // !SW_LOG_LEVEL

namespace swav {
	namespace log {

		class Logger {
		public:
			static std::shared_ptr<spdlog::logger>& get() {
				static std::shared_ptr<spdlog::logger> logger = create();
				return logger;
			}

			static void setLogLevel(spdlog::level::level_enum level) {
				get()->set_level(level);
			}

		private:
			static std::shared_ptr<spdlog::logger> create() {
				spdlog::init_thread_pool(8192, 1);

				auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				sink->set_pattern("[%T.%e] [%^%l%$] %v");

				auto logger = std::make_shared<spdlog::async_logger>(
					"syncwave", sink, spdlog::thread_pool(),
					spdlog::async_overflow_policy::block);

				logger->set_level(SWAV_LOG_LEVEL);
				spdlog::register_logger(logger);
				return logger;
			}
		};

		template <typename... Args>
		inline void  d(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->debug(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void  i(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->info(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void  w(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->warn(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void  e(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->error(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void  c(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->critical(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		inline void  t(fmt::format_string<Args...> fmt, Args &&...args) {
			Logger::get()->trace(fmt, std::forward<Args>(args)...);
		}

	}; // namespace log
}; // namespace swav
