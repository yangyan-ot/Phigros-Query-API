/*
 * @File	  : phigros_controller.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/05/17 21:35
 * @Introduce : Phigros的controller
*/
#pragma once

#ifndef PHIGROS_CONTROLLER_HPP
#define PHIGROS_CONTROLLER_HPP  
#include <regex>
#include <configuration/config.hpp>
#include <service/phigros_service.hpp>

constexpr int amount_spaces{ 2 };

class PhigrosController {
private:
	PhigrosController() = delete;
	CrowApp& m_app;
	std::unique_ptr<PhigrosService> m_phigros;

	user getUser(std::string_view authHeader) {
		UserData u;

		std::string bearer{ "Bearer " };
		std::string token{ "" };

		size_t pos = authHeader.find(bearer);
		if (pos != std::string::npos) {
			token = authHeader.substr(pos + bearer.length());
		}

		//std::cout << token << std::endl;

		SQL_Util::LocalDB << "select sid,user,api_calls,token,authority from User where token = ?;"
			<< std::move(token)
			>> [&](unsigned int sid, std::string  user, unsigned int api_calls, std::string token, unsigned char authority) {
			u.api_calls = api_calls;
			u.authority = authority;
			u.sid = sid;
			u.token = token;
			u.username = user;
			};

		SQL_Util::LocalDB << "UPDATE User SET api_calls = api_calls + 1 WHERE sid = ?"
			<< u.sid;

		//std::cout << u.sid << "/" << u.username << "/" << u.token << "/" << u.api_calls << "/" << u.authority << std::endl;

		return u;
	};


public:
	explicit PhigrosController(CrowApp& app, std::unique_ptr<PhigrosService> phigros) :
		m_app{ app }, m_phigros{ std::move(phigros) } {};
	virtual ~PhigrosController() = default;

	const inline void controller(void) {
		CROW_ROUTE(m_app, "/phi/all")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// umyckc74rluncpn7mtxkcanxn
				// yc443mp6cea7xozb3e0kxvvid
				// qdpliq0laha53lfzfptyimz1j
				// v6yitajqe20ceim211502r3h0
				// 496bcu67y7j65oo900f9oznja
				std::string sessionToken{ req.get_header_value("SessionToken") };

				if (sessionToken.empty())
				{
					throw self::HTTPException("SessionToken is empty.", 401, 4);
				}

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0)
				{
					throw self::HTTPException("", 401, 6);
				}

				bool enable_ap{ false };

				if (OtherUtil::verifyParam(req, "is_aplist")) {
					enable_ap = std::stoi(req.url_params.get("is_aplist"));
				}

				// 设置响应头为 application/json

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getAll(authentication, sessionToken, enable_ap).dump(amount_spaces));


				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]all ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]all ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]all ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]all ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/best")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// umyckc74rluncpn7mtxkcanxn
				// yc443mp6cea7xozb3e0kxvvid
				// qdpliq0laha53lfzfptyimz1j
				// v6yitajqe20ceim211502r3h0
				// 496bcu67y7j65oo900f9oznja
				std::string sessionToken{ req.get_header_value("SessionToken") };

				if (sessionToken.empty())
				{
					throw self::HTTPException("SessionToken is empty.", 403, 4);
				}

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0)
				{
					throw self::HTTPException("", 401, 6);
				}
				/* EZ:0, HD:1, IN:2, AT:3, Auto: 4 */
				unsigned char difficulty{ 4 };
				bool info{ false };
				std::string songid{ "" };

				if (OtherUtil::verifyParam(req, "songid")) {
					songid = req.url_params.get("songid");
				}
				else {
					throw self::HTTPException("parameter 'songid' required and parameter cannot be empty.", 400, 7);
				}

				try {
					songid = Global::Phis.at(std::stoi(songid));
				}
				catch (...) {}

				if (OtherUtil::verifyParam(req, "level")) {
					difficulty = std::stoul(req.url_params.get("level"));
				}
				if (OtherUtil::verifyParam(req, "info")) {
					info = static_cast<bool>(std::stoi(req.url_params.get("info")));
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getBest(authentication, sessionToken, songid, difficulty, info).dump(amount_spaces));


				return resp;
			}
			catch (const std::out_of_range& e) {
				LogSystem::logError("[PhigrosAPI]best ------ 不存在的曲目id或难度");
				resp.write(StatusCodeHandle::getSimpleJsonResult(400, "Invalid songid or level", 3).dump(amount_spaces));
				resp.code = 400;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]best ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]best ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]best ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]best ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			return resp;
				});

		CROW_ROUTE(m_app, "/phi/record")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// umyckc74rluncpn7mtxkcanxn
				// yc443mp6cea7xozb3e0kxvvid
				// qdpliq0laha53lfzfptyimz1j
				// v6yitajqe20ceim211502r3h0
				// 496bcu67y7j65oo900f9oznja
				std::string sessionToken{ req.get_header_value("SessionToken") };

				if (sessionToken.empty())
				{
					throw self::HTTPException("SessionToken is empty.", 401, 4);
				}

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0)
				{
					throw self::HTTPException("", 401, 6);
				}


				// 设置响应头为 application/json

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getRecords(authentication, sessionToken).dump(amount_spaces));


				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]record ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]record ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]record ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]record ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/documentSongidByAlias")
			.methods("POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				std::string alias{};
				bool is_nocase{ false };
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				Json body{ Json::parse(req.body) };
				std::exchange(body, body[0]);

				if (body.count("alias")) {
					alias = body.at("alias").get<std::string>();
				}
				else {
					throw self::HTTPException("parameter 'alias' required and parameter cannot be empty.", 400, 7);
				}

				if (body.count("nocase")) {
					is_nocase = body.at("nocase").get<bool>();
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->documentSongidByAlias(authentication, alias, is_nocase).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]documentSongidByAlias ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]documentSongidByAlias ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]documentSongidByAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]documentSongidByAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/alias")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				int32_t songid{};
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				if (OtherUtil::verifyParam(req, "id")) {
					songid = std::stoi(req.url_params.get("id"));
				}
				else {
					throw self::HTTPException("parameter 'id' required and parameter cannot be empty.", 400, 7);
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getAlias(authentication, songid).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]Alias ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]Alias ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]Alias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]Alias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/song")
			.methods("GET"_method, "POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				// 0: sid, 1: id(default), 2: songname
				defined::PhiInfoParamStruct info_param{ .mode = 0,.is_nocase = false };

				if (req.method == crow::HTTPMethod::Get) {
					std::string songid{ "" }, title{ "" };
					bool is_used{ false };
					std::string parameter{ "songid" };
					if (OtherUtil::verifyParam(req, parameter)) {
						songid = req.url_params.get(parameter);
						is_used = true;
					}

					if (is_used)
					{
						try {
							std::stoi(songid);
							info_param.mode = 1;
						}
						catch (...) {
							info_param.mode = 0;
						}
						info_param.song_id = std::move(songid);
					}
					else {
						parameter = "title";
						if (OtherUtil::verifyParam(req, parameter)) {
							title = req.url_params.get(parameter);
							is_used = true;
							info_param.mode = 2;
							info_param.title = std::move(title);
						}
					}

					if (!is_used)throw self::HTTPException("parameter 'songid' or 'title' required and parameter cannot be empty.", 400, 7);
				}
				else if (req.method == crow::HTTPMethod::Post) {
					Json data{ Json::parse(req.body) };
					std::exchange(data, data[0]);

					info_param.mode = 2;

					if (data.count("title")) {
						info_param.title = data.at("title").get<std::string>();
						if (info_param.title.empty())throw self::HTTPException("'title' is empty", 400, 7);
					}
					else throw self::HTTPException("request body missing 'title'", 400, 7);
					if (data.count("is_nocase")) {
						info_param.is_nocase = data.at("is_nocase").get<bool>();
					}
				}
				else {
					throw self::HTTPException("", 405, 13);
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getSongInfo(authentication, info_param).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty()) {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]SongInfo ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]SongInfo ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]SongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]SongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/getFuzzyQuerySongInfo")
			.methods("POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				std::string title{ "" };
				try {
					Json data{ Json::parse(req.body) };
					std::exchange(data, data[0]);
					if (data.count("title")) {
						title = data.at("title").get<std::string>();
					}
					else throw self::HTTPException("request body missing 'title'", 400, 7);
				}
				catch (const Json::parse_error&) {
					// 不处理异常
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getFuzzyQuerySongInfo(authentication, title).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty()) {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]FuzzyQuerySongInfo ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]FuzzyQuerySongInfo ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]FuzzyQuerySongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]FuzzyQuerySongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/fuzzyQueryByAliasGetSongID")
			.methods("POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				std::string alias{ "" };

				Json data{ Json::parse(req.body) };
				std::exchange(data, data[0]);
				if (data.count("alias")) {
					alias = data.at("alias").get<std::string>();
				}
				else throw self::HTTPException("request body missing 'alias'", 400, 7);

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->fuzzyQueryByAliasGetSongID(authentication, alias).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty()) {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]FuzzyQuerySongInfo ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]FuzzyQuerySongInfo ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]FuzzyQuerySongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]FuzzyQuerySongInfo ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/addAlias")
			.methods("POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				Json data{ Json::parse(req.body) };
				std::exchange(data, data[0]);

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				defined::PhiAliasAddParam alias_param;

				alias_param.related_song_id = data.at("relatedId").get<int32_t>();
				alias_param.alias = data.at("alias").get<std::string>();

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->addAlias(authentication, alias_param));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty()) {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]addAlias ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]addAlias ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]addAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]addAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/delAlias")
			.methods("POST"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				Json data{ Json::parse(req.body) };
				std::exchange(data, data[0]);

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority == 0) {
					throw self::HTTPException("", 401, 6);
				}

				defined::PhiAliasAddParam alias_param;

				alias_param.sid = data.at("sid").get<int32_t>();

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->delAlias(authentication, alias_param));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty()) {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]delAlias ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]delAlias ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]delAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]delAlias ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
				});

		CROW_ROUTE(m_app, "/phi/rating")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority < 1)
				{
					throw self::HTTPException("", 401, 6);
				}
				float rating1{}, rating2{ MIN_RATING_CHECK - 1.0f };

				if (OtherUtil::verifyParam(req, "rating1")) {
					rating1 = std::stof(req.url_params.get("rating1"));
				}
				else {
					throw self::HTTPException("parameter 'rating1' required and parameter cannot be empty.", 400, 7);
				}
				if (OtherUtil::verifyParam(req, "rating2")) {
					rating2 = std::stof(req.url_params.get("rating2"));
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getRating(authentication, rating1, rating2).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]best ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]best ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]best ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]best ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			return resp;
				});

		CROW_ROUTE(m_app, "/phi/getBatch")
			.methods("GET"_method)([&](const crow::request& req) {
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				std::string sessionToken{ req.get_header_value("SessionToken") };

				if (sessionToken.empty())
				{
					throw self::HTTPException("SessionToken is empty.", 401, 4);
				}

				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority < 1)
				{
					throw self::HTTPException("", 401, 6);
				}
				float rating1{}, rating2{ -1.0f };

				if (OtherUtil::verifyParam(req, "rating1")) {
					rating1 = std::stof(req.url_params.get("rating1"));
				}
				else {
					throw self::HTTPException("parameter 'rating1' required and parameter cannot be empty.", 400, 7);
				}
				if (OtherUtil::verifyParam(req, "rating2")) {
					rating2 = std::stof(req.url_params.get("rating2"));
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->getBatch(authentication, sessionToken, rating1, rating2).dump(amount_spaces));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]batch ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]batch ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]batch ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]batch ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			return resp;
				});

		if (Global::Meilisearch::IsOpen) {
			CROW_ROUTE(m_app, "/phi/syncMatch")
				.methods("POST"_method)([&](const crow::request& req) {
				crow::response resp;
				resp.set_header("Content-Type", "application/json");
				try {
					// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
					auto authentication{ getUser(req.get_header_value("Authorization")) };
					if (authentication.authority < 5) {
						throw self::HTTPException("", 401, 6);
					}

					// 将 JSON 数据作为响应体返回
					resp.write(this->m_phigros->syncMatch());
					return resp;
				}
				catch (const self::HTTPException& e) {
					if (e.getMessage().empty()) {
						resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
					}
					else {
						resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
					}
					LogSystem::logError(std::format("[Phigros]async match ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
					resp.code = e.getCode();
				}
				catch (const self::TimeoutException& e) {
					LogSystem::logError("[PhigrosAPI]async match ------ API请求超时");
					resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
					resp.code = 408;
				}
				catch (const std::runtime_error& e) {
					LogSystem::logError(std::format("[PhigrosAPI]async match ------ msg: {} / code: {}", e.what(), 500));
					resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
					resp.code = 500;
				}
				catch (const std::exception& e) {
					LogSystem::logError(std::format("[PhigrosAPI]async match ------ msg: {} / code: {}", e.what(), 500));
					resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
					resp.code = 500;
				}

				return resp;
					});

			CROW_ROUTE(m_app, "/phi/matchAlias")
				.methods("POST"_method)([&](const crow::request& req) {
				crow::response resp;
				resp.set_header("Content-Type", "application/json");
				try {
					defined::PhiMatchAlias match_alias;
					Json data{ Json::parse(req.body) };
					std::exchange(data, data[0]);

					auto authentication{ getUser(req.get_header_value("Authorization")) };
					if (authentication.authority < 1) {
						throw self::HTTPException("", 401, 6);
					}

					if (data.count("query")) match_alias.query = data.at("query").get<std::string>();
					if (data.count("limit")) match_alias.limit = data.at("limit").get<uint16_t>();
					if (data.count("offset")) match_alias.offset = data.at("offset").get<uint16_t>();
					if (data.count("hitsPerPage")) match_alias.hitsPerPage = data.at("hitsPerPage").get<uint16_t>();
					if (data.count("page")) match_alias.page = data.at("page").get<uint16_t>();
					if (data.count("showRankingScore")) match_alias.showRankingScore = data.at("showRankingScore").get<bool>();

					// 将 JSON 数据作为响应体返回
					resp.write(this->m_phigros->matchAlias(match_alias));
					return resp;
				}
				catch (const self::HTTPException& e) {
					if (e.getMessage().empty()) {
						resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
					}
					else {
						resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
					}
					LogSystem::logError(std::format("[Phigros]match alias ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
					resp.code = e.getCode();
				}
				catch (const self::TimeoutException& e) {
					LogSystem::logError("[PhigrosAPI]match alias ------ API请求超时");
					resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
					resp.code = 408;
				}
				catch (const std::runtime_error& e) {
					LogSystem::logError(std::format("[PhigrosAPI]match alias ------ msg: {} / code: {}", e.what(), 500));
					resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
					resp.code = 500;
				}
				catch (const std::exception& e) {
					LogSystem::logError(std::format("[PhigrosAPI]match alias ------ msg: {} / code: {}", e.what(), 500));
					resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
					resp.code = 500;
				}

				return resp;
					});
		}


		CROW_ROUTE(m_app, "/phi/upload")
		.methods("POST"_method)([&](const crow::request& req) {
			constexpr const char* filename{ "file" };
			crow::response resp;
			resp.set_header("Content-Type", "application/json");
			try {
				int32_t songid{};
				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm

				std::string sessionToken{ req.get_header_value("SessionToken") };

				if (sessionToken.empty())
				{
					throw self::HTTPException("SessionToken is empty.", 401, 4);
				}

				// Bearer gOzXb0WUtjK6bkv17dybAoyrxIS15srm
				auto authentication{ getUser(req.get_header_value("Authorization")) };
				if (authentication.authority < 5){
					throw self::HTTPException("", 401, 6);
				}

				if (req.body.empty()) { throw self::HTTPException("form-data is empty"s, 400, 7); }

				crow::multipart::message file_message(req);

				std::string dataBody{};

				for (const auto& part : file_message.part_map)
				{
					const auto& part_name = part.first;
					const auto& part_value = part.second;
					CROW_LOG_DEBUG << "Part: " << part_name;
					if (filename == part_name){
						// Extract the file name
						auto headers_it = part_value.headers.find("Content-Disposition");
						if (headers_it == part_value.headers.end())
						{
							throw self::HTTPException("No Content-Disposition found"s, 400, 7);
						}
						auto params_it = headers_it->second.params.find("filename");
						if (params_it == headers_it->second.params.end())
						{
							throw self::HTTPException(std::format("Part with name \"{}\" should have a file", filename), 400, 7);
						}
						const std::string outfile_name = params_it->second;

						for (const auto& part_header : part_value.headers)
						{
							const auto& part_header_name = part_header.first;
							const auto& part_header_val = part_header.second;
							LogSystem::logInfo(std::format("Header: {}={}", part_header_name, part_header_val.value));
							for (const auto& param : part_header_val.params)
							{
								const auto& param_key = param.first;
								const auto& param_val = param.second;
								LogSystem::logInfo(std::format("Param: {},{}", param_key, param_val));
							}
						}

						// LogSystem::logInfo(std::format("Contents written to {},{}", param_key, param_val));
						// CROW_LOG_INFO << " Contents written to " << outfile_name;
						// part_value.body
						dataBody = part_value.body;
					}
					else throw self::HTTPException(std::format("argu passed by {} is empty", filename), 400, 7);
				}

				// 将 JSON 数据作为响应体返回
				resp.write(this->m_phigros->upload(authentication, sessionToken, dataBody));

				return resp;
			}
			catch (const self::HTTPException& e) {
				if (e.getMessage().empty())
				{
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), "", e.getStatus()).dump(amount_spaces));
				}
				else {
					resp.write(StatusCodeHandle::getSimpleJsonResult(e.getCode(), e.getMessage(), e.getStatus()).dump(amount_spaces));
				}
				LogSystem::logError(std::format("[Phigros]Upload ------ msg: {} / code: {} / status: {}", e.what(), e.getCode(), e.getStatus()));
				resp.code = e.getCode();
			}
			catch (const self::TimeoutException& e) {
				LogSystem::logError("[PhigrosAPI]Upload ------ API请求超时");
				resp.write(StatusCodeHandle::getSimpleJsonResult(408, "Data API request timeout", 2).dump(amount_spaces));
				resp.code = 408;
			}
			catch (const std::runtime_error& e) {
				LogSystem::logError(std::format("[PhigrosAPI]Upload ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}
			catch (const std::exception& e) {
				LogSystem::logError(std::format("[PhigrosAPI]Upload ------ msg: {} / code: {}", e.what(), 500));
				resp.write(StatusCodeHandle::getSimpleJsonResult(500, e.what(), 1).dump(amount_spaces));
				resp.code = 500;
			}

			return resp;
	});
};
};


#endif // !PHIGROS_CONTROLLER_HPP
