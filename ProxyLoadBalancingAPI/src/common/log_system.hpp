/*
 * @File	  : log_system.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/05 21:17
 * @Introduce : 处理日志相关
*/

#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "configuration/config.hpp"
#include <iostream>

#ifndef LOG_SYSTEM_HPP
#define LOG_SYSTEM_HPP  


class LogSystem final {
public:
	static void initialized(){
		spdlog::info("日志初始化");
		//初始化日志信息
		s_logger = spdlog::daily_logger_mt(MS_LOGGER_NAME, MS_FILENAME, MS_HOUR, MS_MINUTE);
		//设置全局日志等级
		s_logger->set_level(spdlog::level::info);
		spdlog::set_level(spdlog::level::info);
		spdlog::info("日志初始化完成"); 

#ifdef DEBUG
		std::cout << "MS_LOGGER_NAME->" << MS_LOGGER_NAME << "\n";
		std::cout << "MS_FILENAME->" << MS_FILENAME << "\n";
		std::cout << "MS_HOUR->" << MS_HOUR << "\n";
		std::cout << "MS_MINUTE->" << MS_MINUTE << std::endl;
#endif	// DEBUG
	};

	enum logType :int {
		info = 0,
		debug = 1,
		warn = 2,
		critical = 3,
		error = 4
	};

#ifdef DEBUG
	static void getTestMethod(void) {
		spdlog::info("测试");
	};
#endif	// DEBUG

	static void dailyLogRecord(
		std::string_view record_literal,
		logType type = logType::info) {

		switch (type) {
		case info:
			spdlog::info(record_literal);
			s_logger->info(record_literal);
			break;
		case debug:
			spdlog::debug(record_literal);
			s_logger->debug(record_literal);
			break;
		case warn:
			spdlog::warn(record_literal);
			s_logger->warn(record_literal);
			break;
		case critical:
			spdlog::critical(record_literal);
			s_logger->critical(record_literal);
			break;
		case error:
			spdlog::error(record_literal);
			s_logger->error(record_literal);
			break;
		}
	};

	static void logInfo(std::string_view record_literal) {
		dailyLogRecord(record_literal);
	};
	
	static void logDebug(std::string_view record_literal) {
		dailyLogRecord(record_literal, logType::debug);
	};
	
	static void logWarn(std::string_view record_literal) {
		dailyLogRecord(record_literal, logType::warn);
	};

	static void logCritical(std::string_view record_literal) {
		dailyLogRecord(record_literal, logType::critical);
	};

	static void logError(std::string_view record_literal) {
		dailyLogRecord(record_literal, logType::error);
	};
private:
	LogSystem(void) = delete;
	~LogSystem(void) = delete;
	LogSystem(const LogSystem&) = delete;
	LogSystem(LogSystem&&) = delete;
	LogSystem& operator=(const LogSystem&) = delete;
	LogSystem& operator=(LogSystem&&) = delete;
	inline static std::shared_ptr<spdlog::logger> s_logger{ NULL };
	inline const static auto MS_LOGGER_NAME = Config::config_yaml["log"]["logger-name"].as<std::string>();
	inline const static auto MS_FILENAME = Config::config_yaml["log"]["filename"].as<std::string>();
	inline const static auto MS_HOUR = Config::config_yaml["log"]["hour"].as<int>();
	inline const static auto MS_MINUTE = Config::config_yaml["log"]["minute"].as<int>();
};

#endif