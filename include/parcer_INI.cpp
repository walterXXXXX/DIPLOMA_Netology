#include <fstream>
#include <sstream>
#include <exception>
#include <string>
#include "parcer_INI.h"

ParcerINI::ParcerINI(std::string fileName) {
	std::ifstream fin(fileName);
	if (!fin.is_open()) throw std::runtime_error("Файл " + fileName + " не найден\n");
	_fileName = fileName;

	int countStr = 0;			// счетчик строк
	std::string str = "";		// считываемая строка
	std::string secName = "";	// имя текущей секции
	bool isEmpty = true;		// флаг отсутствия считанных секций в парсере

	while (std::getline(fin, str)) {
		countStr++; // номер текущей строки

		// убираем из строки комментарий - все, начиная с символа ';'
		str.erase(find(str.cbegin(), str.cend(), ';'), str.end()); 
		trimStr(str); // и пробелы в начале и в конце строки 
//		std::cout << str << std::endl;
		if (str == "") continue; // если в итоге стока осталась пустой, переходим на следующую
		
		auto it = str.cbegin();
		if (*it != '[' && isEmpty)
			throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
				". Не найдено объявление первой секции\n");

		if (*it == '[') {
			// считываем имя секции
			auto it1 = find(it, str.cend(), ']');
			if (it1 == str.cend())
				throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
					". Не найден символ ']' в объявлении имени секции\n");
			if (it1 != str.cend()-1)
				throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
					". Недопустимые символы после имени секции []..\n"); // возможный комментарий и пробелы уже удалены
			
			secName = std::string(it + 1, it1);
			if (secName == "")
				throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
					". Отсутствует имя секции\n");
			addSection(secName);
			isEmpty = false;
		}
		else {
			// считываем имя и значение переменной
			auto it2 = find(it, str.cend(), '=');
			if (it2 == str.cend())
				throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
					". Отсутствует знак равенства '=' в объявлении переменной\n");
			if (it2 == str.cbegin())
				throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
					". Отсутствует имя переменной слева от знака '='\n");

			// получаем имя и значение переменной слева и справа от =
			std::string varName(str.cbegin(), (it2));
			std::string varValue((it2 + 1), str.cend());
			// удаляем пробелы и табуляции в начале и конце полученных значений
			trimStr(varName);
			trimStr(varValue);
			// проверяем допустимость имени переменной
			if (find(varName.cbegin(), varName.cend(), ' ') != varName.cend() || 
				find(varName.cbegin(), varName.cend(), '\t') != varName.cend())
					throw std::logic_error("Ошибка в строке " + std::to_string(countStr) +
						". Недопустимый символ в имени переменной\n");
			addVariable(secName, varName, varValue); // сохраняем значение переменной в секции 
		}
	}	
	if (isEmpty)
		throw std::logic_error("Файл " + fileName + " пуст\n");
	fin.close();
}

void ParcerINI::addSection(std::string secName) {
	// добавляем секцию secName, если она еще не существует, иначе игнор
	sections.insert(std::make_pair(secName, std::map<std::string, std::string>()));
}

void ParcerINI::addVariable(std::string secName, std::string varName, std::string variable) {
	if (sections.find(secName) == sections.cend())
		throw std::invalid_argument("Невозможно добавить переменную, т.к. секция с именем " + secName + " не найдена\n");
	// добавляем переменную varName в найденную секцию с перезаписью значения, если она уже существует
	sections[secName].insert_or_assign(varName, variable);
}	

void ParcerINI::trimStr(std::string& str) {
	// вспомогательная функция, удаляет пробелы и табуляции в начале и конце строки
	auto it1 = str.cbegin();
	auto it2 = str.cend();
	while ((it1 != it2) && (*it1 == ' ' || *it1 == '\t'))
		it1++;
	while ((it1 != it2) && (*(it2-1) == ' ' || *(it2-1) == '\t'))
		it2--;
	str = std::string(it1, it2);
}

std::string ParcerINI::getValueString(std::string varName) { // varName = "section.value"
	auto it = std::find(varName.cbegin(), varName.cend(), '.');
	if (it == varName.cend() || it == varName.cbegin() || it == (varName.cend() - 1))
		throw std::logic_error("Ошибка в запросе переменной переменной <" + varName + ">\n");

	std::string _sec = std::string(varName.cbegin(), it);
	std::string _var(it + 1, varName.cend());

	if (sections.find(_sec) == sections.cend()) {
		std::string res = "Ошибка! Отсутствует секция с именем " + _sec + "\n Допустимые имена секций: \n";
		for (const auto& section : sections)
			res += (section.first + '\n');
		throw std::invalid_argument(res);
	}

	for (const auto& variable : sections[_sec])
		if (variable.first == _var)
			return variable.second;

	std::string res = "В секции отсутствует значение для запрашиваемой переменной " + _var +
		". Доступные имена переменных секции: \n";

	for (const auto& variable : sections[_sec]) {
		res += (variable.first + '\n');
	}
	throw std::invalid_argument(res);

}

void ParcerINI::print() {	// печать содержимого
	std::cout << std::endl;
	for (const auto& sec : sections) {
		std::cout << "[" << sec.first << "]" << std::endl;
		for (const auto& var : sec.second) {
			std::cout << var.first << "=" << var.second << std::endl;
		}
		std::cout << std::endl;
	}
	std::cout << "--------------\n";
}
