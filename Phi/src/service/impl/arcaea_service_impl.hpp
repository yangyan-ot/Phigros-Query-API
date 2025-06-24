/*
 * @File	  : arcaea_service_impl.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/05/08 19:00
 * @Introduce : ArcaeaAPI的简单封装
*/

#pragma once

#include <chrono>
#include <bcrypt.h>
#include "service/arcaea_service.hpp"
#include "jwt-cpp/jwt.h"
#include "common/utils/http_util.hpp"

#ifndef ARCAEA_SERVICE_IMPL_HPP 
#define ARCAEA_SERVICE_IMPL_HPP  

#define OLD  0  

using json = nlohmann::json;
class ArcaeaServiceImpl : public ArcaeaService {
public:
	ArcaeaServiceImpl() {
		++ms_count;
	};
	~ArcaeaServiceImpl() = default;
private:
	inline static int ms_count{ 0 };
};

#endif // !ARCAEA_SERVICE_IMPL_HPP