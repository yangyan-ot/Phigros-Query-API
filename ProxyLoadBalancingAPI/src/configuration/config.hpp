/*
 * @File	  : config.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/05 21:14
 * @Introduce : 配置类(解析yaml)
*/

#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "yaml-cpp/yaml.h"
#include "fmt/format.h"
#include <limits>

#ifndef CONFIG_HPP
#define CONFIG_HPP  

using namespace std::string_literals;
using ubyte = unsigned char;
using ushort = unsigned short;

namespace std {
	using fmt::format;
	using fmt::format_error;
	using fmt::formatter;
}

#define Json nlohmann::json
#define Ubyte ubyte

struct ProcessInfo {
	uint32_t p_sid;
	uint32_t p_pid;
	uint16_t p_port;
	std::string p_path;
};

struct user {
	unsigned int sid{ std::numeric_limits<unsigned int>::max() };
	std::string username{ "" };
	unsigned int api_calls{ 0 };
	std::string token{ "" };
	unsigned char authority{ 0 };
};

#define UserData user

class Config final{
private:
	Config() = delete;
	~Config() = delete;
	Config(const Config&) = delete;
	Config(Config&&) = delete;
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&) = delete;
public:
	inline static YAML::Node config_yaml{ YAML::LoadFile("./config.yaml")};

	static void initialized(){

	}
};

class Global final {
	friend class Config;
private:
	Global() = delete;
	~Global() = delete;
	Global(const Global&) = delete;
	Global(Global&&) = delete;
	Global& operator=(const Global&) = delete;
	Global& operator=(Global&&) = delete;
public:
	inline static std::vector<ProcessInfo> process_info;
	inline static std::vector<std::thread> exec_list;
	inline static size_t proxy_count{ 0 };
	inline static uint64_t cyclic_query_value{ 0 };
	inline static std::string resource_path{ Config::config_yaml["other"]["resource-path"].as<std::string>()};
};

#define BODY_SELECT if (data_body.contains("detail") && data_body.contains("status")) { \
	throw self::HTTPException( \
		data_body.at("detail").get<std::string>(), \
		response.status_code(), \
		data_body.at("status").get<uint16_t>()); \
} \
else if (data_body.contains("status")) { \
	throw self::HTTPException( \
		"", \
		response.status_code(), \
		data_body.at("status").get<uint16_t>()); \
} \
else if (data_body.contains("detail")) { \
throw self::HTTPException( \
	data_body.at("detail").get<std::string>(), \
	response.status_code(), \
	1); \
} \

#endif