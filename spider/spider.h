#ifndef SPIDER_H
#define SPIDER_H

#include "iostream"
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <regex>
#include <map>

#include <boost/locale.hpp>

#include "../include/thread_pool.h"
#include "../include/search_engine_DB.h"

#include "link.h"
#include "http_utils.h"

class Spider {
public:
	Spider(std::unique_ptr<SeacrhEngineDB> _db);
	~Spider();
	void parseLink(const Link& link, int depth);
	void parseLink(std::string url, int depth);

private:
	void getURLs(std::vector<Link>& links, const std::string& html);
	void clearHtml(std::string& html);
	void parseToDB(const std::string& text, const std::string url);

	ThreadPool threadPool;
	std::unique_ptr<SeacrhEngineDB> db;
	std::mutex mtxCout;
	std::mutex mtxUrlsCount;
	int urlsCount = 0;
	int urlsSkiped = 0;

	int minWordLength = 3;
	int maxWordLength = 32;
};

#endif SPIDER_H