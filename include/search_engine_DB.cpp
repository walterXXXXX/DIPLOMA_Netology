#include "search_engine_DB.h"

#include "iostream"

SeacrhEngineDB::SeacrhEngineDB(const std::string connectionDB) {
	conn = new pqxx::connection(connectionDB);
	createDB();
}

SeacrhEngineDB::~SeacrhEngineDB() {
	delete conn;
}

SeacrhEngineDB::SeacrhEngineDB(SeacrhEngineDB&& other) noexcept {
	conn = other.conn;
	other.conn = nullptr;
}

SeacrhEngineDB& SeacrhEngineDB::operator=(const SeacrhEngineDB&& other) noexcept {
	conn = other.conn;
	return *this;
}

bool SeacrhEngineDB::checkDB() {
	mtxDB.lock();
	bool result = conn->is_open();
	mtxDB.unlock();
	return result;
}

void SeacrhEngineDB::addDataToDB(const std::string& url, const std::string& word, const int frequency) {
	mtxDB.lock();
	
	pqxx::work work(*conn);

	work.exec("INSERT INTO urls (url) VALUES('" + work.esc(url) +
		"') ON CONFLICT (url) DO NOTHING;");

	work.exec("INSERT INTO words (word) VALUES('" + work.esc(word) +
		"') ON CONFLICT (word) DO NOTHING;");

	int urlID = work.query_value<int>("SELECT id FROM urls WHERE url = '" + work.esc(url) + "';");

	int wordID = work.query_value<int>("SELECT id FROM words WHERE word = '" + work.esc(word) + "';");

	work.exec("INSERT INTO urls_words (url_id, word_id, frequency) VALUES('" +
		std::to_string(urlID) + "', '" + std::to_string(wordID) + "', '" + std::to_string(frequency) +
		"') ON CONFLICT (url_id, word_id) DO UPDATE SET frequency = '" + std::to_string(frequency) + "';");

	work.commit();

	mtxDB.unlock();
}

void SeacrhEngineDB::clearDB() {
	mtxDB.lock();

	pqxx::work work(*conn);
	work.exec("DELETE FROM urls_words");
	work.exec("DELETE FROM urls");
	work.exec("DELETE FROM words");
	work.commit();

	mtxDB.unlock();
}

std::vector<std::string> SeacrhEngineDB::getUrlsByFrequency(const std::string& searchQuery, const int maxCount) {
// возвращает вектор значений URL, в которых наиболее часто встречаются слова из запроса searchQuery 
	
	std::string search = "";							// Обрабатываем строку запроса
	for (int i = 0; i < searchQuery.length(); ++i) {
		if (searchQuery[i] == '%')						// Пропускаем символ '%' 
			i += 2;										// и следующие два символа
		else 		
			search += std::tolower(searchQuery[i]);		// Добавляем символ в нижнем регистре
	}

	std::vector<std::string> searchResult;
	std::unordered_map<std::string, int> frequency;
	std::istringstream iss(search);
	std::string word;

	while (std::getline(iss, word, '+')) {				// Забираем из запроса слова, отделенные +
		
		if ((word.length() >= 3) && (word.length() <= 32)) {
			mtxDB.lock();

			pqxx::work work(*conn);						// Получаем таблицу с URL и частотой каждого слова из запроса
			pqxx::result res = work.exec("SELECT u.url, uw.frequency FROM urls u "
										 "JOIN urls_words uw ON u.id = uw.url_id "
										 "JOIN words w ON uw.word_id = w.id "
										 "WHERE w.word = " + work.quote(word) + ";");
			work.commit();
			mtxDB.unlock();

			for (const auto& row : res) {				// Сохраняем результаты в контейнер
				frequency[row[0].as<std::string>()] += row[1].as<int>();
			}
		}
		//std::cout << "Word: " << word << std::endl;
		//for (const auto& pair : frequency) {
		//	std::cout << pair.first << ": " << pair.second << std::endl;
		//}
	}

	// сортировка URL по частоте слов запроса
	std::vector<std::pair<std::string, int>> res(frequency.begin(), frequency.end());
	std::sort(res.begin(), res.end(), [](const auto& a, const auto& b) {
		return a.second > b.second;	
		});

	int count = std::min(maxCount, static_cast<int>(res.size()));
	for (int i = 0; i < count; ++i) {
		searchResult.push_back(res[i].first);			// Сохраняем только ключи, в кол-ве не более maxCount
	}
	return searchResult;
}

int SeacrhEngineDB::getUrlsCount() {
	mtxDB.lock();

	pqxx::work work(*conn);
	pqxx::result res = work.exec("SELECT COUNT(*) FROM urls;");
	work.commit();

	mtxDB.unlock();
	
	if (!res.empty()) 
		return res[0][0].as<int>();

	return 0;
}

int SeacrhEngineDB::getWordsCount() {
	mtxDB.lock();

	pqxx::work work(*conn);
	pqxx::result res = work.exec("SELECT COUNT(*) FROM words;");
	work.commit();

	mtxDB.unlock();

	if (!res.empty())
		return res[0][0].as<int>();

	return 0;
}

bool SeacrhEngineDB::isExistUrl(const std::string& url) {
	mtxDB.lock();

	pqxx::work work(*conn);
	pqxx::result res = work.exec("SELECT * FROM urls WHERE url ='" + url + "';");
	work.commit();
	mtxDB.unlock();

	return !res.empty();
}

std::vector<std::string> SeacrhEngineDB::getUrls() {
	mtxDB.lock();

	std::vector<std::string> urls;

	pqxx::work work(*conn);
	pqxx::result res = work.exec("SELECT url FROM urls");
	for (const auto& row : res)
		urls.push_back(row["url"].c_str());
	work.commit();
	mtxDB.unlock();

	return urls;
}



void SeacrhEngineDB::createDB() {

	pqxx::work work(*conn);

	work.exec("CREATE table IF NOT EXISTS urls ("
		"id serial primary KEY,"
		"url varchar(2000) NOT NULL UNIQUE); ");

	work.exec("CREATE table IF NOT EXISTS words ("
		"id serial primary KEY,"
		"word varchar(32) NOT NULL UNIQUE); ");

	work.exec("CREATE table IF NOT EXISTS urls_words ("
		"url_id integer references urls(id),"
		"word_id integer references words(id),"
		"frequency integer NOT NULL,"
		"constraint pk primary key(url_id, word_id)); ");

	work.commit();

}
