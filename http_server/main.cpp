#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <memory>

#include "http_connection.h"
#include <Windows.h>

#include "../include/parcer_INI.h"
#include "../include/search_engine_DB.h"

void httpServer(tcp::acceptor& acceptor, tcp::socket& socket,
				 std::shared_ptr<SeacrhEngineDB>& db)
{
	acceptor.async_accept(socket,
		[&](beast::error_code ec)
		{
			if (!ec)
				std::make_shared<HttpConnection>(std::move(socket), db)->start();
			httpServer(acceptor, socket, db);
		});
}

int main(int argc, char* argv[]) {

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

		auto db = std::make_shared<SeacrhEngineDB>(dbConnection);

		std::cout << "\n[HttpServer]" << std::endl;
		unsigned short port = config.getValue<int>("HttpServer.port");
		std::cout << "port=" << port << std::endl << std::endl;

		auto const address = net::ip::make_address("0.0.0.0");

		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, { address, port }};
		tcp::socket socket{ioc};

		httpServer(acceptor, socket, db);

		std::cout << "Open browser and connect to http://localhost:8080 to see the web server operating" << std::endl;

		ioc.run();

	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}