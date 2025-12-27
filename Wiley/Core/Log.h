#pragma once
#include "Timer.h"

#include <array>
#include <string>
#include <print>
#include <iostream>
#include <string_view>

namespace Wiley {

#define WILEY_LOG_INFO(fmt, ...)  Log::info(__FILE__,  __LINE__, fmt, ##__VA_ARGS__)
#define WILEY_LOG_DEBUG(fmt,...)  Log::debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WILEY_LOG_WARN(fmt, ...)  Log::warn(__FILE__,  __LINE__, fmt, ##__VA_ARGS__)
#define WILEY_LOG_ERROR(fmt,...)  Log::error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WILEY_LOG_FATAL(fmt,...)  Log::fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


	class Log
	{
	public:
		enum class LogLevel
		{
			FATAL,  //something that will cause the application to actually crash
			ERROR,  //something that will cause an action not to happen at all due to something
			WARNING,//triggered when performing an acting that could lead to any of the actions above
			INFO,   //Just general status stuff that has to be known
			DEBUG,  //Just temp debug stuff
		};

		std::array <std::string_view, 5> kLogLevelStrings{ "[FATAL]:","[ERROR]:","[WARNING]:","[INFO]:","[DEBUG]:" };

		void setLogLevel(LogLevel level) { mLogLevel = level; }

		static Log& logger();
		/// @brief base logging function
		template <typename... Args>
		void log(LogLevel level, const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			if (level > mLogLevel)
				return;

			// Get timestamp
			std::string t;
			WILEY_SYS_TIME(t);

			std::string formattLogLeveledMessage;

#if __cplusplus >= 202002L
			// ✅ C++20 and newer: use {} placeholders
			if constexpr (sizeof...(args) > 0)
			{
				formattLogLeveledMessage = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
			}
			else
			{
				formattLogLeveledMessage = fmt;
			}
#else
			// ✅ Pre-C++20: fallback to printf-style formatting (%s, %d, etc.)
			if constexpr (sizeof...(args) > 0)
			{
				char buffer[4096];
				std::snprintf(buffer, sizeof(buffer), fmt.c_str(), std::forward<Args>(args)...);
				formattLogLeveledMessage = buffer;
			}
			else
			{
				formattLogLeveledMessage = fmt;
			}
#endif

			std::string logLevelString = kLogLevelStrings[(int)level].data();
			std::string finalMessage = t + logLevelString + file + ":" + std::to_string(line) + " " + formattLogLeveledMessage + "\n";

			std::cout << finalMessage.c_str() << std::endl;
		}


		template <typename... Args>
		static void fatal(const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			logger().log(LogLevel::FATAL, file, line, fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static void error(const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			logger().log(LogLevel::ERROR, file, line, fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static void warn(const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			logger().log(LogLevel::WARNING, file, line, fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static void info(const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			logger().log(LogLevel::INFO, file, line, fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static void debug(const std::string& file, int line, const std::string& fmt, Args&&... args)
		{
			logger().log(LogLevel::DEBUG, file, line, fmt, std::forward<Args>(args)...);
		}

		LogLevel mLogLevel = LogLevel::DEBUG;
	};
}