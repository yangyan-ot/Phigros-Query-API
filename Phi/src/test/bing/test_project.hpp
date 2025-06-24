#pragma once

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <string>
#include <common/utils/sql_handle.hpp>
#include "common/utils/other_util.hpp"
#include <span>
#include <jwt-cpp/jwt.h>
#include <bcrypt.h>
#include <sqlite_modern_cpp.h>
#include "crow.h"
#include <qrencode.h>
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>
#include <common/utils/http_util.hpp>
#include <service/impl/phi_taptap_api.hpp>

#ifndef TEST_PROJECT_HPP
#define TEST_PROJECT_HPP  
using namespace std::string_literals;

//#define WARNING_CONTENT  
using _uint64 = unsigned long long int;
using StatusCodeHandle = HTTPUtil::StatusCodeHandle;

struct KeyComparator {
	bool operator()(const auto& lhs, const auto& rhs) const {
		return lhs > rhs;
	}
};

struct personalPhiSongInfo{
	std::string sid;
	int id;
	std::string title;
	std::string song_illustration_path;
	float level;
	std::string difficulty{};
	float rks;
	float acc;
	bool is_fc;
	unsigned int score;
};

struct PhiSongInfo {
	std::string sid;
	int id;
	std::string title; 
	std::string song_illustration_path;
	float rating[5]{0.0f,0.0f,0.0f,0.0f,0.0f};
};

class TestProject final{
public:
	inline static void SongAllData() {
		// umyckc74rluncpn7mtxkcanxn
		// yc443mp6cea7xozb3e0kxvvid
		// qdpliq0laha53lfzfptyimz1j
		// v6yitajqe20ceim211502r3h0
		// 496bcu67y7j65oo900f9oznja
		self::PhiTaptapAPI phiAPI("yc443mp6cea7xozb3e0kxvvid");
		auto phi{ phiAPI.getPlayerRecord() };

		/*
		for (const auto& [key, value] : phi) {
			std::cout << key << std::endl;
			for (const auto& [key1, val] : value) {
				std::cout << val.acc << ", " << val.difficulty << ", " << val.score << std::endl;
			}
		}
		*/

	}

	inline static void ConnectPhiSQLite(void) {
		StartWatch();
		
		sqlite::database db("PhigrosInfo.db");

		std::unordered_map<std::string, PhiSongInfo> phis;

		SQL_Util::PhiDB << "select sid,id,title,song_illustration_path,rating_ez,rating_hd,rating_in,rating_at,rating_lg from phigros;"
			>> [&](std::string sid, int id, std::string title, std::string song_illustration_path,
				float rating_ez, float rating_hd, float rating_in, float rating_at, float rating_lg) {
					PhiSongInfo phi;
					if (!sid.empty())
					{
						phi.sid = sid;
						phi.id = id;
						phi.title = title;
						phi.song_illustration_path = song_illustration_path;
						phi.rating[0] = rating_ez;
						phi.rating[1] = rating_hd;
						phi.rating[2] = rating_in;
						phi.rating[3] = rating_at;
						phi.rating[4] = rating_lg;

						phis[sid] = phi;
					}
		};
		StopWatch();
		StartWatch();
		unsigned int count{ 0 };
		
		/*
		for (const auto& [key, value] : phis) {
			++count;
			std::cout << "[ " << key << " / " << count << " ]  ->  ";
			std::cout << value.id << ',' << value.title << ',' << value.song_illustration_path
				<< ',' << value.rating[0] << ',' << value.rating[1] << ',' << value.rating[2] << ',' << value.rating[3] << ',' << value.rating[4] << '\n';
		}
		*/

		std::cout.flush();
		StopWatch();

		StartWatch();
		// umyckc74rluncpn7mtxkcanxn (我的)
		// yc443mp6cea7xozb3e0kxvvid
		// qdpliq0laha53lfzfptyimz1j
		// v6yitajqe20ceim211502r3h0
		self::PhiTaptapAPI phiAPI("yc443mp6cea7xozb3e0kxvvid");

		auto phi{ phiAPI.getPlayerRecord() };

		//auto content{ phi["DESTRUCTION321.Normal1zervsBrokenNerdz.0"] };

		std::multimap<float, personalPhiSongInfo, KeyComparator> rksSort;
		std::multimap<float, personalPhiSongInfo, KeyComparator> singlePhi;

		for (const auto& [key, value] : phis) {
			std::string levels[]{ "EZ", "HD", "IN", "AT", "Legacy" };
			//std::cout << "\n=====================\n( " << key << " )\n";
			auto content{ phi[key] };
			// <= 4为有LG但是在计算b19并不会包含lg
			for (size_t i = 0; i < 4; i++)
			{
				try
				{
					unsigned int score{ content.at(i).score };
					float
						acc{ content.at(i).acc },
						rks{ 0.0f },
						rate{ value.rating[i] };
					std::string difficulty{ content.at(i).difficulty };
					bool is_fc{ content.at(i).is_fc };
					if (acc >= 70) rks = (std::pow((acc - 55) / 45, 2)) * rate;

					personalPhiSongInfo info;
					info.acc = acc;
					info.sid = key;
					info.id = value.id;
					info.is_fc = is_fc;
					info.difficulty = difficulty;
					info.score = score;
					info.level = rate;
					info.rks = rks;
					info.title = value.title;
					info.song_illustration_path = value.song_illustration_path;
					
					if (score >= 1000000)
					{
						singlePhi.insert(std::make_pair(rks, info));
					}
					rksSort.insert(std::make_pair(rks, info));

					/*std::cout << "[" << difficulty << " / " << rate << "] Score: " << score
						<< ", Acc: " << acc << ", RankingSocre: " << rks
						<< ", Is FC: " << is_fc
						<< '\n';*/
				}
				catch (const std::out_of_range e)
				{
					std::cout << "[" << levels[i] << "] No Record\n";
				}
				catch (const std::exception& e) {
					std::cout << e.what() << std::endl;
				}
			}

		}

		/*
		std::cout << "\n=====================\n";

		count = 0;
		for (const auto& [key, value] : singlePhi) {
			++count;
			std::cout << "(phi) -- [" << value.title << " / " << value.difficulty << " / " << value.level << "] Score: " << value.score
				<< ", Acc: " << value.acc << ", RankingSocre: " << value.rks
				<< ", Is FC: " << value.is_fc
				<< '\n';
			std::cout << "\n=====================\n";
			if (count >= 1)break;
		}


		count = 0;
		for (const auto& [key, value] : rksSort) {
			++count;
			std::cout << "(" << count << ") -- [" << value.title << " / " << value.difficulty << " / " << value.level << "] Score: " << value.score
				<< ", Acc: " << value.acc << ", RankingSocre: " << value.rks
				<< ", Is FC: " << value.is_fc
				<< '\n';
			std::cout << "\n=====================\n";
			if (count >= 19)break;
		}
		*/
		std::cout.flush();

		auto [profile, avatar, background] {phiAPI.getUserData()};

		std::cout << profile << "\n=====================================\n";
		std::cout << avatar << "\n=====================================\n";
		std::cout << background << "\n=====================================\n";

		auto summary{ phiAPI.GetSummary() };
		std::cout << "AT:";
		for (const auto& item : summary.AT)
		{
			std::cout << static_cast<int>(item) << "   ";
		}
		std::cout << "IN:";
		for (const auto& item : summary.IN)
		{
			std::cout << static_cast<int>(item) << "   ";
		}
		std::cout << "HD:";
		for (const auto& item : summary.HD)
		{
			std::cout << static_cast<int>(item) << "   ";
		}
		std::cout << "EZ:";
		for (const auto& item : summary.EZ)
		{
			std::cout << static_cast<int>(item) << "   ";
		}
		std::cout << std::endl;
		std::cout << summary.ChallengeModeRank << std::endl;
		std::cout << summary.updatedAt << std::endl;
		std::cout << summary.RankingScore << std::endl;
		std::cout << summary.nickname << std::endl;
		std::cout << std::endl;
		StopWatch();

	}

	inline static void OtherTest(void) {
	}

	inline static void WidthStringTest() {
		std::string str = "Hello こんにちは 你好";
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring wstr = converter.from_bytes(str);

		wstr.pop_back();

		str = converter.to_bytes(wstr);

		std::cout << str << std::endl;
	}

	inline static void redisppTest(void) {
		using namespace sw::redis;
		using std::cin;

		try {
			// Create an Redis object, which is movable but NOT copyable.
			auto redis = Redis("tcp://bkhra571@127.0.0.1:6379/1?keep_alive=true");

			while (true)
			{
				auto val = redis.get("newKey");    // val is of type OptionalString. See 'API Reference' section for details.
				if (!val) {
					redis.set("newKey", "0");
					redis.expire("newKey", 60L);
					val = redis.get("newKey");
				}
				redis.set("newKey", std::to_string(std::stoi(*val) + 1),true);
				auto ttl{ redis.ttl("newKey") };
				spdlog::info("value :"s + *val + ",ttl :"s + std::to_string(ttl));
				int temp;
				cin >> temp;
				val.reset();
			}
		}
		catch (const Error& e) {
			spdlog::error(e.what());
		}
	};

	static void testModernSqlite() {
		try {
			// creates a database file 'dbfile.db' if it does not exists.
			sqlite::database db("localDB.db");
			
			/*
				std::string s;
				std::cin >> s;
			*/

			// executes the query and creates a 'user' table
			//db << "select id,bottleMainId,thrower,throwGroup,content,filePath,timeStamp,reportCount,available from bottle;"

			///* 
			//	where bottleMainId = ? ;"
			//					<< s
			//*/
			//	>> [&](int id, int bottleMainId, std::string thrower, std::string throwGroup,
			//		std::string content, std::string filePath, _uint64 timeStamp, int reportCount,
			//		bool available
			//		) {
			//			BottleDao bottle{
			//			id,bottleMainId,thrower,throwGroup,content,
			//			filePath,timeStamp,reportCount,available
			//			};

			//	std::cout << bottle.getJson().dump() << std::endl;
			//};

		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}

	static int testRedis(void) {
		const char* redis_password = "bkhra571";
		redisContext* redis_conn = redisConnectWithTimeout("127.0.0.1", 6379, { 1, 500000 });
		if (redis_conn == nullptr || redis_conn->err) {
			if (redis_conn) {
				fprintf(stderr, "Redis connection error: %s\n", redis_conn->errstr);
			}
			else {
				fprintf(stderr, "Failed to allocate redis context\n");
			}
			redisFree(redis_conn);
			exit(1);
		}

		// 身份验证
		redisReply* redis_reply = static_cast<redisReply*>(redisCommand(redis_conn, "AUTH %s", redis_password));

		if (redis_reply == nullptr) {
			std::cerr << "Failed to authenticate with Redis: " << redis_conn->errstr << std::endl;
			redisFree(redis_conn);
			return 1;
		}

		const char* key = "mykey";
		const char* value = "hello world";

		redisReply* reply = (redisReply*)redisCommand(redis_conn, "SET %s %s", key, value);
		if (reply == NULL) {
			fprintf(stderr, "Failed to execute SET command\n");
			redisFree(redis_conn);
			exit(1);
		}

		printf("SET %s %s\n", key, value);

		freeReplyObject(reply);
		redisFree(redis_conn);
		return 0;
	}

	//检测文件是否存在
	static void testMethod1(void) {
		bool b1{ std::filesystem::exists("config.yaml") };
		bool b2{ std::filesystem::exists("config.yml") };
		std::cout << "ymal -> " << b1 << std::endl;
		std::cout << "yml  -> " << b2 << std::endl;
	}

#ifdef WARNING_CONTENT
	// SQL查询测试
	static void testMethod2(void) {
		std::cout << "More:\n";
		TestProject::testMoreSQLQuery();

		std::cout << "Simple:\n";
		TestProject::testSimpleSQLQuery();
	}
#endif // WARNING_CONTENT
	// jwt测试
	static void testMethod3(void) {
		std::string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJpc3MiOiJhdXRoMCJ9.AbIJTDMFc7yUa5MhvcP03nJPyCPzZtQcGEp-zWfOkEE";
		auto decoded = jwt::decode(token);

		for (auto& e : decoded.get_payload_json())
			std::cout << e.first << " = " << e.second << std::endl;
	}

	// bcrypt测试
	static void testMethod4(void) {
		std::string password = "250kaijie";

		//生成
		std::string hash = bcrypt::generateHash(password,10);

		std::cout << "250kaijie: " << hash << std::endl;

		//效验
		std::cout << "\"" << password << "\" : " << bcrypt::validatePassword(password, hash) << std::endl;
		std::cout << "\"wrong\" : " << bcrypt::validatePassword("wrong", hash) << std::endl;
		std::cout << "\"database\" : " << bcrypt::validatePassword(password, "$2a$10$2MKm.McjEdc/O.PXArxEeOh4dBROZ1BoLswMP8lG8bwRBDZdqhRoe") << std::endl;
	}
	// crow框架测试
	static void testMethod6(void) {
		crow::SimpleApp app;
		app.bindaddr("0.0.0.0").port(9961);

		CROW_ROUTE(app, "/hello_test_wtf")
			.methods("GET"_method)([](const crow::request& req) {

			json data;
			data["message"] = "Hello World!";

			// 设置响应头为 application/json
			crow::response resp;
			resp.set_header("Content-Type", "application/json");

			// 将 JSON 数据作为响应体返回
			resp.body = data.dump();


			return resp;
				});
		CROW_ROUTE(app, "/testPage/<string>")([](std::string name) { // 
			auto page = crow::mustache::load("index.html"); // 
			crow::mustache::context ctx({ {"person", name} }); // 
			return page.render(ctx); //
			});

		CROW_ROUTE(app, "/hello")
			.methods("GET"_method)
			([](const crow::request& req) {
			std::string par = req.url_params.get("par");
			return "Hello, World!"s + par;
				});

		CROW_ROUTE(app, "/postTest")
			.methods("POST"_method)
			([](const crow::request& req) {
			std::string data = req.body;
#if 0
			for (const auto& item : req.headers) {
				std::cout << item.first <<
					"\t" << item.second << std::endl;
			}
#endif
			std::cout << "token : " << req.get_header_value("token") << std::endl;
			return "result -> "s + data;
				});

		// websocket
		CROW_ROUTE(app, "/ws")
			.websocket()
			.onopen([&](crow::websocket::connection& conn) {
			std::cout << "建立ws连接" << std::endl;
				})
			.onclose([&](crow::websocket::connection& conn, const std::string& reason) {
					std::cout << "关闭ws连接" << std::endl;
				})
					.onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
					if (is_binary)
						std::cout << "binary -> ";
					else
						std::cout << "normal -> ";
					std::cout << "receive: " << data << std::endl;
					conn.send_text("HelloTo");
	});

				// 全局异常处理
				CROW_ROUTE(app, "/")([]() {
					crow::response res("404 Not Found has been changed to this message.");
					res.code = 404;
					return res;
					});
				//set the port, set the app to run on multiple threads, and run the app

		app.multithreaded().run();
	}
#ifdef WARNING_CONTENT
	static void testMoreSQLQuery(void) {
		StartWatch();
		SQL_Handle handle;
		auto contents{ handle.moreQuery("SELECT * from bottle") };
		printMaps(contents);

		std::vector<BottleDao> result;
		std::string jsonList {"["};
		bool flag{false};
		for (const auto& content : contents) {
			BottleDao bottle{
				std::stoi(content.at("id").second),
				std::stoi(content.at("bottleMainId").second),
				content.at("thrower").second,
				content.at("throwGroup").second,
				content.at("content").second,
				content.at("filePath").second,
				std::stoull(content.at("timeStamp").second),
				std::stoi(content.at("reportCount").second),
				std::stoi(content.at("available").second)
			};
			jsonList += (flag ? "," : "") + std::move(bottle.getJson().dump());
			flag = true;
			std::cout << "json -> " << bottle.getJson() << "\n";
			result.push_back(std::move(bottle));
		};
		jsonList += "]";

		std::cout << "jsons -> " << jsonList << "\n";
		StopWatch();
	}
	static void testSimpleSQLQuery(void) {
		StartWatch();

		SQL_Handle handle;
		auto content{ handle.simpleQuery("SELECT * from bottle where bottleMainId = 1000000") };
		printMap(content);
		
		BottleDao bottle{ 
			std::stoi(content.at("id").second),
			std::stoi(content.at("bottleMainId").second),
			content.at("thrower").second,
			content.at("throwGroup").second,
			content.at("content").second,
			content.at("filePath").second,
			std::stoull(content.at("timeStamp").second),
			std::stoi(content.at("reportCount").second),
			std::stoi(content.at("available").second)
		};

		std::cout << bottle.toString();
		std::cout << bottle.getJson() << "\n";

		StopWatch();
	}
#endif // WARNING_CONTENT
private:
	static void printMap(const auto& m) {
		std::cout << "========================\n";
		for (auto& [key, value] : m) {
			std::cout << std::format("key:{}, type:{}, value:{}\n", key, value.first, value.second);
		}
		std::cout << "========================\n";
		std::cout.flush();
	}
	
	static void printMaps(const auto& vec) {
		std::cout << "========================\n";
		for (const auto& item : vec) {
			for (auto& [key, value] : item) {
				std::cout << std::format("key:{}, type:{}, value:{}\n", key, value.first, value.second);
			}
			std::cout << "========================\n";
		}
		std::cout.flush();
	}

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