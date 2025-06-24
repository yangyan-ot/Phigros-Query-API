#pragma once

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <string>
#include <span>
#include <jwt-cpp/jwt.h>
#include <sqlite_modern_cpp.h>
#include "crow.h"
#include <qrencode.h>
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>

#ifndef TEST_PROJECT_HPP
#define TEST_PROJECT_HPP  
//#define WARNING_CONTENT  
using _uint64 = unsigned long long int;

class TestProject final{
public:
private:
	inline static std::chrono::system_clock::time_point start, end;

	static void StartWatch(void) {
		std::cout << "\033[42mstart watch\033[0m\n";
		start = std::chrono::high_resolution_clock::now();
	};

	static void StopWatch(void) {
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cout << "\033[42mtime consumed " << elapsed.count() << " ms\033[0m\n";
		std::cout.flush();
	};
};

#endif