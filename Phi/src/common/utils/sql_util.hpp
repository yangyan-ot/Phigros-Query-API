/*
 * @File	  : sql_utils.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/11 14:35
 * @Introduce : SQLite工具
*/

#pragma once

#include <memory>
#include <regex>
#include <filesystem>
#include <sqlite_modern_cpp.h>
#include "common/utils/log_system.hpp"
#include "configuration/config.hpp"
#include "common/exception/self_exception.hpp"
#include "sqlite3.h"

#ifndef SQL_UTIL_HPP
#define SQL_UTIL_HPP  

namespace SQL_Util {
	inline sqlite::database LocalDB(Config::getConfig()["db"]["path"]["local"].as<std::string>());
	inline sqlite::database PhiDB(Config::getConfig()["db"]["path"]["phi"].as<std::string>());
	inline sqlite::database PlayerRdDB(Config::getConfig()["db"]["path"]["phi-player"].as<std::string>());
	inline void initialized(void) {
		// PhiDB << "PRAGMA journal_mode=WAL;";
	};
};
#endif