#include <iostream>
#include <memory>
#include <Windows.h>
#pragma execution_character_set("utf-8")

#include "../include/parcer_INI.h"
#include "../include/search_engine_DB.h"

#include "spider.h"

int main()
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try {
		ParcerINI config("../config.ini");

		std::cout << "[DataBase]" << std::endl;
		std::string host = config.getValue<std::string>("DataBase.host");
		std::string dbPort = config.getValue<std::string>("DataBase.port");
		std::string dbname = config.getValue<std::string>("DataBase.dbname");
		std::string user = config.getValue<std::string>("DataBase.user");
		std::string password = config.getValue<std::string>("DataBase.password");
		std::string dbConnection = "host=" + host + " " + "port=" + dbPort + " " +
			"dbname=" + dbname + " " + "user=" + user + " " + "password=" + password + " ";
		std::cout << dbConnection << std::endl;

		std::cout << "\n[Spider]" << std::endl;
		std::string start = config.getValue<std::string>("Spider.start");
		std::cout << "start=" << start << std::endl;
		int depth = config.getValue<int>("Spider.depth");
		std::cout << "depth=" << depth << std::endl;

		std::cout << "\n[HttpServer]" << std::endl;
		std::string port = config.getValue<std::string>("HttpServer.port");
		std::cout << "port=" << port << std::endl;

		auto db = std::make_unique<SeacrhEngineDB>(dbConnection);
		
		//db->clearDB();

		auto spider = std::make_unique<Spider>(std::move(db));

		std::cout << "\nStart parsing links\n" << std::endl;

		spider->parseLink(start, depth);

	}
	catch (const std::exception& e) 
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
