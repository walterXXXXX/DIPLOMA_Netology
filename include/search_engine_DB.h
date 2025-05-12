#ifndef SEARCH_ENGINE_DB_H
#define SEARCH_ENGINE_DB_H

#include <mutex>
#include <map>

#include <pqxx/pqxx>

class SeacrhEngineDB {
public:
	SeacrhEngineDB(const std::string connectionDB);
	~SeacrhEngineDB();
	SeacrhEngineDB(const SeacrhEngineDB& other) = delete;
	SeacrhEngineDB(SeacrhEngineDB&& other) noexcept;
	SeacrhEngineDB& operator=(const SeacrhEngineDB& other) = delete;
	SeacrhEngineDB& operator=(const SeacrhEngineDB&& other) noexcept;

	bool checkDB();
	
	void addDataToDB(const std::string& url, const std::string& word, const int frequency);
	void clearDB();
	std::vector<std::string> getUrlsByFrequency(const std::string& searchQuery, const int maxCount);
	int getUrlsCount();
	int getWordsCount();
	bool isExistUrl(const std::string& url);
	std::vector<std::string> getUrls();

private:
	void createDB();

	pqxx::connection* conn;
	std::mutex mtxDB;

};

#endif SEARCH_ENGINE_DB_H