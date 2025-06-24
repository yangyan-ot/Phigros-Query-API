/*
 * @File	  : arcaea_controller.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/18 20:23
 * @Introduce : Arcaea的controller
*/

#pragma once

#include <future>
#include <memory>
#include <string_view>
#include <chrono>
#include <configuration/config.hpp>
#include "crow.h"
#include "common/utils/http_util.hpp"
#include "nlohmann/json.hpp"
#include "bcrypt.h"
#include <service/impl/arcaea_service_impl.hpp>
#include <service/arcaea_service.hpp>
#include <configuration/config.hpp>

#ifndef ARCAEA_CONTROLLER
#define ARCAEA_CONTROLLER

using json = nlohmann::json;
using namespace std::chrono;
using namespace std::chrono_literals;
using StatusCode = HTTPUtil::StatusCodeHandle::status;
using StatusCodeHandle = HTTPUtil::StatusCodeHandle;

class ArcaeaController final {
public:
    explicit ArcaeaController(CrowApp& app, std::string_view secret, std::string_view issuer, std::unique_ptr<ArcaeaService> arcaea) :
        m_app{ app }, m_secret{ secret }, m_issuer{ issuer }, m_arcaea{ std::move(arcaea) } {};
    ~ArcaeaController() = default;
    const inline void controller(void) {
        CROW_ROUTE(m_app, "/hello_test")
            .methods("GET"_method)([](const crow::request& req){ 

            json data;
            data["message"] = "Hello World!";

            // 设置响应头为 application/json
            crow::response resp;
            resp.set_header("Content-Type", "application/json");

            // 将 JSON 数据作为响应体返回
            resp.write(data.dump(2));
            
            
            return resp;
                });
    }
private:
    struct verify_token_result {
        bool pass;
        std::string msg;
        int code;
    };
    ArcaeaController() = delete;
    CrowApp& m_app;
    const std::string m_secret;
    const std::string m_issuer;
    std::unique_ptr<ArcaeaService> m_arcaea;
};

#endif // !ARCAEA_CONTROLLER