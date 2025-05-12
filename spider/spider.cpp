#include "spider.h"

#include <pqxx/pqxx>

Spider::Spider(std::unique_ptr<SeacrhEngineDB> _db) {
	db = std::move(_db);
	if (!db->checkDB())
		throw std::exception("SPIDER: DB connection error");
}

Spider::~Spider() {
	threadPool.join();
	std::cout << "\nParsed URL's to DB: " << db->getUrlsCount() << std::endl;
	std::cout << "Parsed words to DB: " << db->getWordsCount() << std::endl << std::endl;
}

void Spider::parseLink(const Link& link, int depth) {
	
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	std::string parceURL = ((link.protocol == ProtocolType::HTTP) ? "http://" : "https://") +
		link.hostName + link.query;

	if (parceURL.length() > 2000) return;	// ограничене длины URL для совместимости с большинством браузеров
	if (db->isExistUrl(parceURL)) return;	// пропускаем URL, который уже есть в БД

	try {
	
		std::string html = getHtmlContent(link);
	
		if (html.size() == 0)
			throw("Failed to get HTML Content");

		std::vector<Link> links;

		getURLs(links, html);
	
		if (depth > 0) {
			for (auto& subLink : links) {
				threadPool.newTask([this, subLink, depth]() {this->parseLink(subLink, depth - 1); });
				//parseLink(subLink, depth - 1);
			}
		}

		clearHtml(html);
		parseToDB(html, parceURL);

	}
	catch (const std::exception& e) {
		mtxCout.lock();
		std::cout << "Exception: " << e.what() << " during parsing url: " << std::endl;
		std::cout << parceURL << std::endl;
		mtxCout.unlock();
	}

	catch (...) {
		mtxCout.lock();
		std::cout << "Unknown exception" << " during parsing url: " << std::endl;
		std::cout << parceURL << std::endl;
		mtxCout.unlock();
	}

}

void Spider::parseLink(std::string url, int depth) {
	
	std::regex urlRegex(R"((https?://)([^/]+)(/.*)?)");
	std::smatch matches;
	if (std::regex_search(url, matches, urlRegex)) {
		Link link;
		std::string protocol = matches[1].str();
		if (protocol == "https://")
			link.protocol = ProtocolType::HTTPS;
		else
			link.protocol = ProtocolType::HTTP;
		link.hostName = matches[2].str();
		link.query = matches[3].str();
		parseLink(link, depth);
	}
	else {
		throw std::exception("Uncorrect URL");
	}
}

void Spider::getURLs(std::vector<Link>& links, const std::string& html) {
	// возвращает в links все url ссылки, найденные в html
	std::regex linkRegex(R"(<a href=\"(.*?)\")");

	auto linksBegin = std::sregex_iterator(html.begin(), html.end(), linkRegex);
	auto linksEnd = std::sregex_iterator();

	for (auto it = linksBegin; it != linksEnd; it++) {

		std::string matchUrl = (*it).str();
		std::regex urlRegex(R"((https?://)([^/]+)(/.*)?")");
		std::smatch matches;

		if (std::regex_search(matchUrl, matches, urlRegex)) {
			Link newLink;
			std::string protocol = matches[1].str();
			if (protocol == "https://")
				newLink.protocol = ProtocolType::HTTPS;
			else
				newLink.protocol = ProtocolType::HTTP;
			newLink.hostName = matches[2].str();
			newLink.query = matches[3].str();
			links.push_back(newLink);
		}
	}
}

void Spider::clearHtml(std::string& html) {

	html = std::regex_replace(html, std::regex("<[^>]*>"), " ");		// удаление тегов
	html = std::regex_replace(html, std::regex("[^a-zA-Z0-9 ]*"), "");	// удаление знаков препинания
	html = std::regex_replace(html, std::regex("[ ]+"), " ");			// удаление лишних пробелов

	boost::locale::generator gen;
	std::locale loc = gen("");
	html = boost::locale::to_lower(html, loc);							// перевод текста в нижний регистр

}

void Spider::parseToDB(const std::string& text, const std::string url) {
	std::unordered_map<std::string, int> frequency;
	std::istringstream iss(text);
	std::string word;

	while (iss >> word) {
		if ((word.length() >= 3) && (word.length() <= 32)) {
			frequency[word]++;
		}
	}

	for (const auto& pair : frequency) {
		db->addDataToDB(url, pair.first, pair.second);
	}

}


