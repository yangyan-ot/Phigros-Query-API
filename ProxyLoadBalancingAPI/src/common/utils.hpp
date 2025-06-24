/*
 * @File	  : utils.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/05/28 14:11
 * @Introduce : 各种工具
*/

#pragma once

#include <sqlite_modern_cpp.h>
#include <vector>
#include <unistd.h>
#include <thread>
#include <random>

#ifndef UTILS_HPP
#define UTILS_HPP

namespace self {
	struct DB {
		inline static sqlite::database LocalDB{ sqlite::database("./localDB.db")};
		inline static sqlite::database PhiDB{ sqlite::database("./PhigrosInfo.db")};
	};

	struct Tools {
		inline static std::string charset { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" }; // 可选字符集

		inline static bool exec_simple(const char* cmd) {
			FILE* pipe{ popen(cmd, "r") };
			if (!pipe) {
				pclose(pipe);
				return false;
			}
			pclose(pipe);
			return true;
		}

		// 运行shell脚本并获取字符串
		inline static const std::string exec(const char* cmd) {
			FILE* pipe = popen(cmd, "r");
			if (!pipe) {
				pclose(pipe);
				return "ERROR";
			}
			char buffer[128];
			std::string result = "";
			while (!feof(pipe)) {
				if (fgets(buffer, 128, pipe) != NULL)
					result += buffer;
			}
			pclose(pipe);
			return result;
		}

		// 生成token
		inline static std::string generateToken(size_t length = 32) {
			std::random_device seeder; // 用于生成随机种子    mt19937 gen(rd()); // 以随机种子初始化随机数生成器

			const auto seed{ seeder.entropy() ? seeder() : time(nullptr) };

			std::mt19937 engine{ static_cast<std::mt19937::result_type>(seed) };

			std::uniform_int_distribution<size_t> distribution{ 0, charset.length() - 1 }; // 均匀分布

			std::string randomStr;
			for (size_t i = 0; i < length; i++) {
				randomStr += charset[distribution(engine)]; // 从字符集中随机选择一个字符    
			}
			return randomStr;
		}
	};
}

#endif // !UTILS_HPP

