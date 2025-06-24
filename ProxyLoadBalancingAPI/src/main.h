#pragma once

// 设置1为开启跨域访问(想要性能问题的话建议关闭,使用反向代理)
#include "httplib.h"
#include <iostream>
#include <thread>
#include <chrono> 
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <array>
#include <vector>
#include "crow.h"

#include "spdlog/spdlog.h"
#include "fmt/format.h"

namespace std {
    using fmt::format;
    using fmt::format_error;
    using fmt::formatter;
}

#include <configuration/config.hpp>
#include "common/utils.hpp"
#include "common/self_exception.hpp"
#include "common/http_util.hpp"
#include "common/crow_middleware.hpp"
#include "common/log_system.hpp"

// #include <restbed>
#include <ctime>
#include <cpprest/http_client.h>

#ifndef MAIN_H
#define MAIN_H

// O3优化
#pragma GCC optimize(3)
#pragma G++ optimize(3)

using namespace std::chrono_literals;

UserData getUser(std::string_view);

//初始化
inline void init(void) {
    Config::initialized();
    LogSystem::initialized(); 

    self::DB::LocalDB << "DELETE FROM ProcessInfo;";

    int port_min { Config::config_yaml["other"]["exec-port-min"].as<int>() },
        port_max { Config::config_yaml["other"]["exec-port-max"].as<int>() },
        unified_threads{ Config::config_yaml["other"]["unified-threads"].as<int>() };
    std::int64_t start_delay{ Config::config_yaml["other"]["start-delay"].as<std::int64_t>() };

    for (int port{ port_min }; port <= port_max; ++port) {
        std::string command{ std::format("{}/start --port={} --concurrency={} --sid={} &",std::filesystem::current_path().c_str(),port,unified_threads,Global:: proxy_count) };
        auto status_code { std::system(command.c_str()) };

        if (status_code != 0)
        {
            throw std::runtime_error("Startup failed with error code "s + std::to_string(status_code) + " .");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(start_delay));
        ++Global::proxy_count;
    }

    self::DB::LocalDB << "select sid,pid,port,path from ProcessInfo;"
        >> [&](uint32_t sid, uint32_t pid, uint16_t port, std::string path) {
        ProcessInfo pi;
        pi.p_pid = pid;
        pi.p_path = path;
        pi.p_port = port;
        pi.p_sid = sid;
        
        Global::process_info.emplace_back(pi);
    };
}

std::atomic<int> backend_index(0);

namespace fs = std::filesystem;

// 启动项
inline void start(void) {
    uint16_t port{ Config::config_yaml["other"]["port"].as<uint16_t>() };

    std::string proxy_url{ Config::config_yaml["other"]["proxy-uri"].as<std::string>() };

    auto proxy_count{ Global::proxy_count };
    crow::SimpleApp app;

    // 反向代理phi的api
    CROW_ROUTE(app, "/proxy/<path>").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post, 
        crow::HTTPMethod::Patch, crow::HTTPMethod::Delete, crow::HTTPMethod::Head)
        ([&](const crow::request req,const std::string& uri) {;
        crow::response resp;
        std::vector<web::http::uri> backends;
        try {
            uint8_t retry{ 0 };
            size_t max_retry{ (proxy_count + 1) / 2 };

            // 负载均衡url
            for(const auto& info : Global::process_info){
                std::string s_url{ proxy_url + ":"s + std::to_string(info.p_port) + '/' + uri};

                auto url{ web::http::uri_builder(U(s_url)) };
                for (const auto& key : req.url_params.keys())
                {
                    url.append_query(U(key),U(req.url_params.get(key)));
                }
                backends.emplace_back(url.to_uri());

            }

            std::vector<web::http::client::http_client> clients;
            for (const auto& uri : backends) {
                clients.emplace_back(uri);
            }

            std::string body, result_body;
            while (1) {
                ++retry;
                auto& client = clients.at(Global::cyclic_query_value % clients.size());
                auto& u = backends.at(Global::cyclic_query_value % backends.size());
                ++Global::cyclic_query_value;
                web::http::http_request request;

                // 选择请求方式
                switch (req.method)
                {
                case crow::HTTPMethod::Get:
                    request.set_method(web::http::methods::GET);
                    break;
                case crow::HTTPMethod::Post:
                    request.set_method(web::http::methods::POST);
                    break;
                case crow::HTTPMethod::Patch:
                    request.set_method(web::http::methods::PATCH);
                    break;
                case crow::HTTPMethod::Delete:
                    request.set_method(web::http::methods::DEL);
                    break;
                case crow::HTTPMethod::Head:
                    request.set_method(web::http::methods::HEAD);
                    break;
                default:
                    throw self::HTTPException("", 405, 1);
                    break;
                }
                for (const auto& header : req.headers) {
                    if (header.first == "SessionToken" || header.first == "Authorization") {
                        LogSystem::logInfo(std::format("[Header]{}: {}", header.first, header.second));
                    }
                    request.headers().add(header.first, header.second);
                }
                // 设置request的body
                request.set_body(U(req.body));

                web::http::http_response response{ client.request(request).get() };

                for (const auto& header : response.headers()){
                    resp.set_header(header.first, header.second);
                };

                LogSystem::logInfo(std::format("Proxy server: {}", client.base_uri().to_string()));
                if (response.status_code() != web::http::status_codes::OK && retry <= max_retry){
                    LogSystem::logError(std::format("connect fail, code: {}, retry: {}, err_port: {}",
                        response.status_code(), retry, client.base_uri().port()));

                    if (response.status_code() < 500 && response.status_code() >= 400) {
                        try {
                            concurrency::streams::stringstreambuf buffer;
                            response.body().read_to_end(buffer).get();
                            body = buffer.collection();

                             Json data_body = Json::parse(body);
                            BODY_SELECT;
                        }
                        catch (const self::HTTPException&) {
                            throw;
                        }
                        catch (const std::exception&){}
                        
                        throw self::HTTPException("", response.status_code(), 1);
                    }

                    continue;
                }
                else if (retry > max_retry) {
                    concurrency::streams::stringstreambuf buffer;
                    response.body().read_to_end(buffer).get();
                    body = buffer.collection();

                    Json data_body = Json::parse(body);

                    BODY_SELECT;
                }

                concurrency::streams::stringstreambuf buffer;
                response.body().read_to_end(buffer).get();
                result_body = buffer.collection();
                break;
            }
            resp.code = 200;
            resp.write(result_body);
            LogSystem::logInfo("query success");
            return resp;
        }catch (const self::HTTPException& e) {
            LogSystem::logError(std::format("code: {},detail: {},status: {}",e.getCode(),e.getMessage(), e.getStatus()));
            resp.code = e.getCode();
            resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(2));
        }catch(const std::runtime_error& e){
            LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
            resp.code = 500;
            resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(2));
        }catch (const std::exception& e) {
            LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
            resp.code = 500;
            resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, "", 1).dump(2));
        }
        resp.set_header("Content-Type", "application/json");
        return resp;
        });
    
    // 添加用户
    CROW_ROUTE(app, "/manage/add").methods(crow::HTTPMethod::Post)([&](const crow::request req) {
            crow::response resp;
            resp.set_header("Content-Type", "application/json");
            LogSystem::logInfo("添加授权用户");
            try {
                auto authentication{ getUser(req.get_header_value("Authorization")) };
                if (authentication.authority < 5)
                {
                    throw self::HTTPException("", 401);
                }

                Json data{ Json::parse(req.body) };
                std::exchange(data, data[0]);

                self::DB::LocalDB << "insert into User (user,token,authority) values (?,?,?);"
                    << data.at("user").get<std::string>()
                    << self::Tools::generateToken()
                    << data.at("authority").get<uint8_t>();

                LogSystem::logInfo("添加授权用户成功");
                resp.write("ok");
                return resp;
            }catch (const self::HTTPException& e) {
                LogSystem::logError(std::format("code: {},detail: {}", e.getCode(), e.getMessage()));
                resp.code = e.getCode();
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage()).dump(2));
            }catch (const std::runtime_error& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }catch (const std::exception& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }
            return resp;
        });

    // 授权列表
    CROW_ROUTE(app, "/manage/userlist").methods(crow::HTTPMethod::Get)([&](const crow::request req) {
            crow::response resp;
            resp.set_header("Content-Type", "application/json");
            LogSystem::logInfo("用户列表获取");
            try {
                auto authentication{ getUser(req.get_header_value("Authorization")) };
                if (authentication.authority < 5)
                {
                    throw self::HTTPException("", 401);
                }

                Json result;
                
                self::DB::LocalDB << "select sid,user,api_calls,token,authority from User;"
                    >> [&](unsigned int sid, std::string user, unsigned int api_calls, std::string token, unsigned char authority) {
                    result[user] = {
                        {"sid", sid},
                        {"apiCalls", api_calls},
                        {"token", token},
                        {"authority", authority}
                    };
                };

                LogSystem::logInfo("用户列表获取成功");
                resp.write(result.dump(2));
                return resp;
            }catch (const self::HTTPException& e) {
                LogSystem::logError(std::format("code: {},detail: {}", e.getCode(), e.getMessage()));
                resp.code = e.getCode();
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage()).dump(2));
            }catch (const std::runtime_error& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }catch (const std::exception& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }
            return resp;
        });

    // 授权
    CROW_ROUTE(app, "/manage/permissions/<int>/<int>").methods(crow::HTTPMethod::Put)([&](const crow::request req,int sid, int auth) {
            crow::response resp;
            resp.set_header("Content-Type", "application/json");
            LogSystem::logInfo("修改权限");
            try {
                auto authentication{ getUser(req.get_header_value("Authorization")) };
                if (authentication.authority < 5)
                {
                    throw self::HTTPException("", 401);
                }

                self::DB::LocalDB << "UPDATE User SET authority = ? WHERE sid = ?"
                    << auth
                    << sid;

                LogSystem::logInfo("修改权限成功");
                resp.write("ok");
                return resp;
            }catch (const self::HTTPException& e) {
                LogSystem::logError(std::format("code: {},detail: {}", e.getCode(), e.getMessage()));
                resp.code = e.getCode();
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage()).dump(2));
            }catch (const std::runtime_error& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }catch (const std::exception& e) {
                LogSystem::logError(std::format("code: {},detail: {}", 500, e.what()));
                resp.code = 500;
                resp.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(500, e.what()).dump(2));
            }
            return resp;
        });

    // 资源
    CROW_ROUTE(app, "/<path>")([&](const std::string& p) {
        crow::response response;

        std::string path{ Global::resource_path + p };

        if (!std::filesystem::exists(path)) {
            response.set_header("Content-Type", "application/json");
            response.code = 404;
            response.write(HTTPUtil::StatusCodeHandle::getSimpleJsonResult(404).dump(2));
            return response;
        }

        // 获取当前时间
        auto now{ std::chrono::system_clock::now() };

        // 计算七天后的时间
        auto seven_days_later{ now + std::chrono::hours(24 * 7) };

        // 将时间转换为时间戳（秒数）
        auto timestamp{ std::chrono::system_clock::to_time_t(seven_days_later) };

        // 将时间戳转换为 Crow 框架中的 Expires 数值
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&timestamp), "%a, %d %b %Y %H:%M:%S GMT");
        std::string expires{ ss.str() };
        response.set_static_file_info(path);
        response.set_header("Cache-Control", "public");
        response.set_header("Expires", expires);
        return response;
    });

    app.port(port).multithreaded().run_async();
}


UserData getUser(std::string_view authHeader) {
    UserData u;

    std::string bearer{ "Bearer " };
    std::string token{ "" };

    size_t pos = authHeader.find(bearer);
    if (pos != std::string::npos) {
        token = authHeader.substr(pos + bearer.length());
    }

    //std::cout << token << std::endl;

    self::DB::LocalDB << "select sid,user,api_calls,token,authority from User where token = ?;"
        << std::move(token)
        >> [&](unsigned int sid, std::string  user, unsigned int api_calls, std::string token, unsigned char authority) {
        u.api_calls = api_calls;
        u.authority = authority;
        u.sid = sid;
        u.token = token;
        u.username = user;
    };

    self::DB::LocalDB << "UPDATE User SET api_calls = api_calls + 1 WHERE sid = ?"
        << u.sid;

    //std::cout << u.sid << "/" << u.username << "/" << u.token << "/" << u.api_calls << "/" << u.authority << std::endl;

    return u;
};

#endif