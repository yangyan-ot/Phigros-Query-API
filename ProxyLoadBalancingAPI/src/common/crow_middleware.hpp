/*
 * @File      : crow_middleware.hpp
 * @Coding    : utf-8
 * @Author    : Bing
 * @Time      : 2023/05/29 20:04
 * @Introduce : crow的一些中间件
 */

#pragma once

#include <chrono>
//#include "crow.h"

#ifndef CROW_MIDDLEWARE
#define CROW_MIDDLEWARE

//using namespace std::literals::chrono_literals;

namespace self::Middleware {
    //template<typename T = std::chrono::seconds, uint64_t TIMEOUT = 5000>
    class TimeoutMiddleware : public crow::ILocalMiddleware
    {
    public:
        struct context
        {};

        void before_handle(crow::request& req, crow::response& res, context& ctx)
        {
            std::cout << "IP: " << req.remote_ip_address << std::endl;
        }

        void after_handle(crow::request& req, crow::response& res, context& ctx)
        {}
    };
}

#endif