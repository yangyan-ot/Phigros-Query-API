/*
 * @File	  : source_location_handle.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/05 16:40
 * @Introduce : 调试报错工具类
*/

#pragma once

#include <chrono>
#include <sstream>
#include "fmt/format.h"
#include "source_location.hpp"

#ifndef SOURCE_LOCATION_HANDLE_HPP
#define SOURCE_LOCATION_HANDLE_HPP  

namespace std {
	using fmt::format;
	using fmt::format_error;
	using fmt::formatter;
	using nostd::source_location;
}
using namespace std::chrono;

class SourceUtils final {
public:
	static std::string getTimeNow(system_clock::time_point t_point = system_clock::now()) {
		std::stringstream ss{};
		time_t tt{ system_clock::to_time_t(t_point) };
		tm* t{ localtime(&tt) };
		ss << std::put_time(t, "%Y/%m/%d %X");
		return ss.str();
	};

	static std::string location(
		std::string_view msg = "",
		const std::source_location& sourec = std::source_location::current()
	) {
		std::string str_handle = msg.empty() ? "" : std::format(" -> message: {}", msg);
		return std::format("[{}] [function: {}] (line: {} , column: {}){}"
			, getTimeNow(), sourec.function_name(), sourec.line(), sourec.column(), str_handle);
	};
private:
	SourceUtils(void) = delete;
	~SourceUtils(void) = delete;
	SourceUtils(const SourceUtils&) = delete;
	SourceUtils(SourceUtils&&) = delete;
	SourceUtils& operator=(const SourceUtils&) = delete;
	SourceUtils& operator=(SourceUtils&&) = delete;
};
#endif