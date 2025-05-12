#ifndef PARCER_INI_H
#define PARCER_INI_H

#include <iostream>
#include <string>
#include <map>
#include <memory>

// Объект класса IniParcer считывает и хранит данные INI файла, имя которого передается при инициализации в конструктор класса. 
// Данные сохраняются в контейнерах std::map для быстрого доступа по ключу: названию секции и названию переменной. 
// Используется контейнер "верхнего" уровня для хранения секций std::map sections:
//    ключ - имя секции std::string, значение - контейнер переменных данной секции std::map.
// Контейнер переменных секции std::map: 
//    ключ - имя переменной std::string, значение - значение переменной std::string.
// Значение переменной возвращается шаблонным методом getValue с указанием типа возвращаемого значения.

class ParcerINI {
public:
	ParcerINI(std::string fileName);

	template <class T>
	T getValue(std::string varName) {  // varName = "section.value"
		// при попытке вызвать тип, не определенный в специализациях шаблона,
		// будет выдана ошибка времени компиляции
		static_assert(sizeof(T) == -1, "not implemented type for getValue()");
	}

	template<>
	std::string getValue(std::string varName) {
		return getValueString(varName);
	}

	template<>
	int getValue(std::string varName) {
		// при неудачной попытке преобразования в int будут выброшены исключения
		// invalid_argument, out_of_range, обрабатываем их при вызове getValue
		return std::stoi(getValueString(varName));
	}

	template<>
	double getValue(std::string varName) {
		// аналогично требуется обработка исключений invalid_argument, out_of_range
		return std::stod(getValueString(varName));
	}

	void print(); // печать содержимого контейнера в консоль (для отладки)

private:
	std::string _fileName = "";

 	std::map <std::string, std::map <std::string, std::string>> sections;

	void addSection(std::string name); // добавляет новую секцию

	void addVariable(std::string secName, std::string varName, std::string variable); // добавляет переменную в контейнер секции secName 

	void trimStr(std::string& str); // вспомогательная функция, удаляет пробелы и табуляции в начале и конце строки

	std::string getValueString(std::string varName); // вытаскиваем из map'ы значение переменной, формат varName = "section.value"
};

#endif PARCER_INI_H