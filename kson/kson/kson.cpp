
#include "stdafx.h"
#include "kson.h"
#include <fstream>
#include <algorithm>

#define INT_MAX_STR_NO_SIGN "2147483647"
#define INT_MIN_STR_NO_SIGN "2147483648"
#define END_OF_FILE '\0'
#define F (format + "    ")
#define DEBUG(arg) { std::cout<<format<<arg<<std::endl; }

using namespace kson;

//============================================================
//  ksonValue
//============================================================

// KsonValue
KsonValue::KsonValue() :
	m_object{}, m_array{}, m_str(),
	m_num(true, 0, 0.0), m_bool(false),
	m_null(nullptr), m_type(KsonType::OBJECT) {}

//============================================================
//  kson解析器
//============================================================

// Kson
Kson::Kson(const std::string& fileName) {
	std::ifstream file(fileName);
	char ic;
	while (file >> ic) {
		m_str.push_back(ic);
	}
	file.close();
	m_str.push_back(END_OF_FILE);  // 末尾增加空白符 '\0' 来标记结束
}

// parse
std::pair<bool, KsonObject> Kson::parse()
{
	skipWS();
	return std::move(parseObject(""));
}

// parseObject
std::pair<bool, KsonObject> Kson::parseObject(const std::string& format) {
	DEBUG("parseObject");

	KsonObject object;
	if (isChar('{')) {
		++m_idx;
		skipWS();
		
		// parse key/value
		while (!isChar('}') && !isChar(END_OF_FILE)) {

			// get key
			auto ret = parseKey(F);
			skipWS();

			if (!ret.first) {
				addError("expect: key");
				return { false, std::move(object) };
			}

			// get value
			if (isChar(':')) {
				++m_idx;
				skipWS();

				auto val = parseValue(F);
				skipWS();

				// 没有成功解析到 value，则直接退出
				if (!val.first) {
					addError("expect: value");
					return { false, std::move(object) };
				}

				// 向 object 中写入 key/value
				object[ret.second] = std::move(val.second);
			}
			else {
				addError("expect ':'");
				return { false, std::move(object) };
			}

			if (!isChar('}')){
				if (isChar(',')) {
					++m_idx;
					skipWS();
				}
				else {
					addError("expect ','");
					return { false, std::move(object) };
				}
			}
		}

		if (isChar('}')) {
			++m_idx;
			skipWS();

			return { true, std::move(object) };
		}

		// 文件结束
		else {
			addError("expect '}'");
			return { false, std::move(object) };
		}
	}

	return { false, std::move(object) };
}

// parseArray
std::pair<bool, KsonArray> Kson::parseArray(const std::string& format) {
	DEBUG("parseArray");

	KsonArray arr;
	if (isChar('[')) {
		++m_idx;
		skipWS();

		// parse value
		while (!isChar(']') && !isChar(END_OF_FILE)) {

			auto val = parseValue(F);
			skipWS();

			// 没有成功解析到 value，则直接退出
			if (!val.first) {
				addError("expect: value");
				return { false, std::move(arr) };
			}

			// 向 object 中写入 key/value
			arr.push_back(std::move(val.second));
			
			if (!isChar(']')) {
				if (isChar(',')) {
					++m_idx;
					skipWS();
				}
				else {
					addError("expect ','");
					return { false, std::move(arr) };
				}
			}
		}

		if (isChar(']')) {
			++m_idx;
			skipWS();

			return { true, std::move(arr) };
		}

		// 文件结束
		else {
			addError("expect ']'");
			return { false, std::move(arr) };
		}
	}

	return { false, std::move(arr) };
}

// parseStr
std::pair<bool, KsonStr> Kson::parseStr(const std::string& format) {
	DEBUG("parseStr");

	++m_idx;  // 跳过开始的 '"'

	int begIdx = m_idx;
	while (isValidStrChar(m_str[m_idx])) {
		++m_idx;
	}
	
	if (isChar('"')) {
		auto begIter = m_str.begin();
		auto endIter = begIter;
		std::advance(begIter, begIdx);
		std::advance(endIter, m_idx);
		return { true, std::move(std::string(begIter, endIter)) };
	}
	
	addError("expect: '\"'");
	return { false, "" };
}

// paseInt : integer / floating
std::pair<bool, KsonNum> Kson::parseNum(const std::string& format) {
	DEBUG("parseNum");

	int intNum = 0;
	double doubleNum = 0;
	bool isNeg = false;   // 是否为负数
	bool isInt = true;    // 是否为整数
	bool hasNum = false;  // 是否解析到数字

	if (isChar('+')) {
		++m_idx;
	}
	else if (isChar('-')) {
		++m_idx;
		isNeg = true;
	}
	if (isNum()) hasNum = true;
	else {
		addError("expect: number.");
		return { false, KsonNum(true, 0, 0.0) };
	}
	
	// 由 kson 文件定义者自己注意整型值的大小，过大则会溢出
	while (isNum()) {
		intNum = intNum * 10 + m_str[m_idx] - 0x30;
		++m_idx;
	}
	
	if (isChar('.')) {

		// 小数点后必须有数字才行
		if (!isNum(1)) {
			return { false, KsonNum(true, 0, 0.0) };
		}

		isInt = false;
		++m_idx;

		int tail = 0;        // 记录小数点后的数字
		int count = 0;       // 记录小数点后的数字的位数
		while (isNum()) {
			tail = tail * 10 + m_str[m_idx] - 0x30;
			++count;
			++m_idx;
		}
		
		double doubleTail = tail;
		while (--count >= 0) {
			doubleTail /= 10;
		}
		doubleNum = intNum + doubleTail;
	}

	if (isChar('e') || isChar('E')) {

		// 科学计数法字符 E 后面必须有数字
		if (!isNum(1)) {
			addError("expect: number after 'E'");
			return { false, std::move(KsonNum(false, 0, 0.0)) };
		}

		++m_idx;
		
		int tail = 0;      // 记录 E 后面的数字
		while (isNum()) {
			tail = tail * 10 + m_str[m_idx] - 0x30;
			++m_idx;
		}
		
		if (isInt) {
			while (tail > 0) {
				intNum *= 10;
			}
		}
		else {
			while (tail > 0) {
				doubleNum *= 10;
			}
		}
	}
	
	if (isNeg) {
		if (isInt) intNum = -intNum;
		else doubleNum = -doubleNum;
	}

	return { hasNum, std::move(KsonNum(isInt, intNum, doubleNum)) };
}

// parseBool: true / TRUE / false / FALSE
std::pair<bool, KsonBool> Kson::parseBool(const std::string& format) {
	DEBUG("parseBool");

	// true
	bool isTL =
		isChar(0, 't')
		&& isChar(1, 'r')
		&& isChar(2, 'u')
		&& isChar(3, 'e');

	// TRUE
	bool isTU =
		isChar(0, 'T')
		&& isChar(1, 'R')
		&& isChar(2, 'U')
		&& isChar(3, 'E');

	// false
	bool isFL = 
		isChar(0, 'f')
		&& isChar(1, 'a')
		&& isChar(2, 'l')
		&& isChar(3, 's')
		&& isChar(4, 'e');

	// FALSE
	bool isFU =
		isChar(0, 'F')
		&& isChar(1, 'A')
		&& isChar(2, 'L')
		&& isChar(3, 'S')
		&& isChar(4, 'E');

	// true / TRUE
	if (isTL || isTU) {
		m_idx += 4;
		return { true, true };
	}

	// false / FALSE
	if (isFL || isFU) {
		m_idx += 5;
		return { true, false };
	}

	addError("expect true/TRUE/false/FALSE.");
	return { false, false };
}

// parseNull: null / NULL
std::pair<bool, KsonNull> Kson::parseNull(const std::string& format) {
	DEBUG("parseNull");

	// null
	bool isNL =
		isChar(0, 'n')
		&& isChar(1, 'u')
		&& isChar(2, 'l')
		&& isChar(3, 'l');

	// NULL
	bool isNU =
		isChar(0, 'N')
		&& isChar(1, 'U')
		&& isChar(2, 'L')
		&& isChar(3, 'L');

	// null / NULL
	if (isNL || isNU){
		m_idx += 4;
		return { true, nullptr };
	}

	addError("expect: null/NULL");
	return { false, nullptr };
}

// parseKey
std::pair<bool, std::string> Kson::parseKey(const std::string& format) {
	DEBUG("parseKey");

	std::string str;
	// 第一个字符不能是数字
	if (!isAlpha(m_str[m_idx])) {
		return { false, std::move(str) };
	}

	// 支持 字母/数字/下划线
	while (isAlpha(m_str[m_idx]) || isNumber(m_str[m_idx]) || isChar('_')) {
		str.push_back(m_str[m_idx]);
		++m_idx;
	}
	return { true, std::move(str) };
}

// parseValue
std::pair<bool, KsonValue> Kson::parseValue(const std::string& format) {
	DEBUG("parseValue");

	KsonValue value;
	// object
	if (isChar('{')) {
		auto ret = parseObject(F);
		value.m_object = std::move(ret.second);
		value.m_type = KsonType::OBJECT;
		return { ret.first, std::move(value) };
	}

	// array
	else if (isChar('[')) {
		auto ret = parseArray(F);
		value.m_array = std::move(ret.second);
		value.m_type = KsonType::OBJECT;
		return { ret.first, std::move(value) };
	}

	// string
	else if (isChar('"')) {
		auto ret = parseStr(F);
		value.m_str = std::move(ret.second);
		value.m_type = KsonType::STRING;
		return { ret.first, std::move(value) };
	}

	// number
	else if (isChar('+') || isChar('-') || isNum()) {
		auto ret = parseNum(F);
		value.m_num = ret.second;
		value.m_type = KsonType::NUMBER;
		return { ret.first, std::move(value) };
	}

	// bool
	else if (isChar('t') || isChar('T') || isChar('f') || isChar('F')) {
		auto ret = parseBool(F);
		value.m_bool = ret.second;
		value.m_type = KsonType::BOOL;
		return { ret.first, std::move(value) };
	}

	// null
	else if (isChar('n') || isChar('N')) {
		auto ret = parseNull(F);
		value.m_null = nullptr;
		value.m_type = KsonType::NUL;
		return { ret.first, std::move(value) };
	}
	
	// others
	else {
		addError(std::string("unexpect: "), m_str[m_idx]);
		return { false, std::move(value) };
	}
}

// skipWS
void Kson::skipWS() {
	while (m_str[m_idx] == ' ' || m_str[m_idx] == '\n' || m_str[m_idx] == '\t') {
		if (m_str[m_idx] == '\n') {
			++m_line;
		}
		++m_idx;
	}
}

// addError
void Kson::addError(std::string errorInfo) {
	m_error += std::move(std::to_string(m_line));
	m_error += ": ";
	m_error += errorInfo + "\n";
}

// addError
void Kson::addError(std::string errorInfo, char appChar) {
	std::string str(1, appChar);
	errorInfo += appChar;
	m_error += std::move(std::to_string(m_line));
	m_error += ": ";
	m_error += errorInfo + "\n";
}

// isValidStrChar
bool Kson::isValidStrChar(char c) {
	std::string str("~`!@#$%^&*()_-+={[}]|\:;'<,>.?/ \t\n");
	auto valid = [&str, c](char arg) -> bool {
		return arg == c;
	};

	// 可见字符和 '\n' '\t' ' '
	if (isAlpha(c) || isNumber(c) || std::any_of(str.begin(), str.end(), valid)) {
		return true;
	}
	return false;
}




//============================================================
//  ksonTest: Kson解析器的测试类
//============================================================

void KsonTest::printVisualize(const KsonObject& val) {
	printObject(val, "");
	print("\n", true);
}

void KsonTest::printObject(const KsonObject& obj, const std::string& format) {

	print(format + "{\n");
	for (auto p : obj) {
		print(F + p.first + ": ");
		auto val = p.second;
		switch (val.m_type) {
		case KsonType::OBJECT:
			print(F + "[type]: OBJECT\n");
			print(F + "{\n");
			printObject(val.m_object, F);
			print(F + "},\n");
			break;
		case KsonType::ARRAY:
			print(F + "[type]: ARRAY\n");
			print(F + "[\n");
			printArray(val.m_array, F);
			print(F + "],\n");
			break;
		case KsonType::STRING:
			print(F + "[type]: STRING\n");
			print(F + "\"" + val.m_str + "\",\n");
			break;
		case KsonType::NUMBER:
			print(F + "[type]: NUMBER\n");
			auto num = val.m_num;
			if (num.m_isInt) print(F + std::to_string(num.m_int) + ",\n");
			else print(F + std::to_string(num.m_double) + ",\n");
			break;
		case KsonType::BOOL:
			print(F + "[type]: BOOL\n");
			print(F + (val.m_bool ? "true,\n" : "false,\n"));
			break;
		case KsonType::NUL:
			print(F + "[type]: NUL\n");
			print(F + "null,\n");
			break;
		default:
			throw 1;
		}
	}
}

void printArray(const KsonArray& arr, const std::string& format) {
	//TODO 
}

void KsonTest::print(const std::string& info, bool close) {
	static std::ofstream outFile(m_fileName);

	if (m_toFile) outFile << info;
	else std::cout << info;

	if (close) outFile.close();
}
