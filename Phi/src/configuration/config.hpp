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
#include "common/utils/sql_util.hpp"
#include "common/exception/self_exception.hpp"
#include "yaml-cpp/yaml.h"
#include "crow.h"
#include "crow/middlewares/cors.h"
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

struct user {
	unsigned int sid{ std::numeric_limits<unsigned int>::max() };
	std::string username{ "" };
	unsigned int api_calls{ 0 };
	std::string token{ "" };
	unsigned char authority{ 0 };
};

namespace defined {
	struct PhiInfoParamStruct {
		uint8_t mode;
		std::string song_id;
		std::string title;
		bool is_nocase;
	};

	struct PhiAliasAddParam {
		int32_t sid;
		std::string alias;
		int32_t related_song_id;
	};

	struct PhiMatchAlias {
		std::string query;
		int32_t limit{ 20 };
		int32_t offset{ 0 };
		int32_t hitsPerPage{ 20 };
		int32_t page{ 1 };
		bool showRankingScore{ true };
	};
}

#if CORS_OPEN
using CrowApplication = crow::App<crow::CORSHandler>;
#else
using CrowApplication = crow::SimpleApp;
#endif

#define UserData user
#define CrowApp CrowApplication
#define Json nlohmann::json
#define Ubyte ubyte

namespace DefinedStruct{
	struct PhiSongInfo {
		std::string sid;
		int id;
		std::string title;
		std::string artist;
		std::string song_illustration_path;
		float rating[5]{ 0.0f,0.0f,0.0f,0.0f,0.0f };
	};

	struct PhiAvatar {
		uint32_t sid;
		std::string avatar_path;
	};
}

namespace Global {
	inline static std::unordered_map<std::string, DefinedStruct::PhiSongInfo> PhigrosSongInfo{};
	inline static std::unordered_map<std::string, DefinedStruct::PhiAvatar> PhigrosPlayerAvatar{};
	inline static std::filesystem::path ExecutableFilePath;
	inline static std::unordered_map<int, std::string> Phis{};
	inline static bool IsPlanB{ false };
	inline static std::string PlayerSavePath{};
	namespace Meilisearch{
		bool IsOpen{ false };
		std::string Url{};
		std::string Authorization{};
		namespace Phi {
			std::string SearchNamespace {"phigros"};
		}
	}
};

class Config final{
public:
	// 获取参数
	class Parameter final {
		friend class Config;
	protected:
		// inline static std::string ms_secret{}, ms_issuer{};
		inline static ushort ms_port{};
		inline static int ms_concurrency{};
	public:
		/*
		static const std::string& getSecret(void){
			return ms_secret;
		}

		static const std::string& getIssuer(void){
			return ms_issuer;
		}
		*/

		static const ushort& getPort(void){
			return ms_port;
		}

		static const int& getConcurrency(void){
			return ms_concurrency;
		}
	private:
		Parameter(void) = delete;
		~Parameter(void) = delete;
		Parameter(const Parameter&) = delete;
		Parameter(Parameter&&) = delete;
		Parameter& operator=(const Parameter&) = delete;
		Parameter& operator=(Parameter&&) = delete;
	};

	static void initialized(){
		const bool 
			yaml_whether_exists { std::filesystem::exists(yaml_path) },
			yal_whether_exists  { std::filesystem::exists(yml_path)  };

		if (yal_whether_exists) {
			ms_public_config = YAML::LoadFile(yml_path);
		}
		else if (yaml_whether_exists){
			ms_public_config = YAML::LoadFile(yaml_path);
		}
		else{
			throw self::FileException("YAML file doesn't exist.");
		}
		
		/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

		// Parameter::ms_secret = getConfig()["server"]["token"]["secret"].as<std::string>();
		// Parameter::ms_issuer = getConfig()["server"]["token"]["issuer"].as<std::string>();
		Parameter::ms_port = getConfig()["server"]["port"].as<ushort>();
		Parameter::ms_concurrency = getConfig()["server"]["concurrency"].as<int>();
		Global::IsPlanB = Config::getConfig()["other"]["plan-b"].as<bool>();
		Global::PlayerSavePath = Config::getConfig()["other"]["player-save-path"].as<std::string>();

		if (Config::getConfig()["search-engine"]) {
			if (Config::getConfig()["search-engine"]["meilisearch"]) {
				Global::Meilisearch::IsOpen = true;
				Global::Meilisearch::Authorization = Config::getConfig()["search-engine"]["meilisearch"]["authorization"].as<std::string>();
				Global::Meilisearch::Url = Config::getConfig()["search-engine"]["meilisearch"]["url"].as<std::string>();
			}
			if (Config::getConfig()["search-engine"]["elasticsearch"]) {

			}
		}
	}
	//得到一个YAML配置文件
	static const YAML::Node& getConfig(void) {
		return ms_config;
	}

	inline static YAML::Node ms_public_config;
private:
	Config(void) = delete;
	~Config(void) = delete;
	Config(const Config&) = delete;
	Config(Config&&) = delete;
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&) = delete;

	inline static const std::filesystem::path
		yaml_path{ "config.yaml" },
		yml_path { "config.yml"  };
	inline static YAML::Node ms_config{ YAML::LoadFile(yaml_path) };
};

#endif