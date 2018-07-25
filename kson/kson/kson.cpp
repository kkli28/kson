
#include "stdafx.h"
#include "kson.h"
#include <fstream>
#include <algorithm>

#define INT_MAX_STR_NO_SIGN "2147483647"
#define INT_MIN_STR_NO_SIGN "2147483648"
#define END_OF_FILE '\0'
#define F (format + "    ")

// DEBUG
#define DEBUG_ENABLE false
#define DEBUG(arg) { if (DEBUG_ENABLE) std::cout<<format<<arg<<std::endl; }

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
// TODO: 增加非法字符的提示
std::pair<bool, KsonObject> Kson::parse()
{
	try {
		skipWS();
		return std::move(parseObject(""));
	}
	catch (KSON_UNEXPECTED_CHARACTOR err) {
		std::cout << m_error << std::endl;
		return { false, {} };
	}
}

// parseObject
std::pair<bool, KsonObject> Kson::parseObject(const std::string& format) {
	DEBUG(std::string("parseObject: '") + std::string(1, m_str[m_idx]) + "'");

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
	DEBUG(std::string("parseArray: '") + std::string(1, m_str[m_idx]) + "'");

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
	DEBUG(std::string("parseStr: '") + std::string(1, m_str[m_idx]) + "'");

	++m_idx;  // 跳过开始的 '"'
	std::string result;
	while (isValidStrChar(m_str[m_idx])) {

		// 转义字符 \n \t \\ \' \"
		char c = m_str[m_idx];
		if (isChar('\\')) {
			char c1 = '1';
			if (isChar(1, 'n'))        c1 = '\n';
			else if (isChar(1, 't'))   c1 = '\t';
			else if (isChar(1, '\\'))  c1 = '\\';
			else if (isChar(1, '\''))  c1 = '\'';
			else if (isChar(1, '\"'))  c1 = '\"';
			if (c1 != '1') {
				c = c1;
				++m_idx;
			}
		}
		result.push_back(m_str[m_idx]);
		++m_idx;
	}
	
	if (isChar('"')) {

		// 跳过结尾的 '"'
		++m_idx;
		skipWS();

		return { true, std::move(result) };
	}
	
	addError("expect: '\"'");
	return { false, "" };
}

// paseInt : integer / floating
// TODO: 增加二进制/八进制/十六进制 的支持
std::pair<bool, KsonNum> Kson::parseNum(const std::string& format) {
	DEBUG(std::string("parseNum: '") + std::string(1, m_str[m_idx]) + "'");

	int intNum = 0;
	double doubleNum = 0;
	bool isNeg = false;   // 是否为负数
	bool isInt = true;    // 是否为整数
	bool hasNum = false;  // 是否解析到数字

	if (isChar('+')) {
		++m_idx;
		skipWS();
	}
	else if (isChar('-')) {
		++m_idx;
		skipWS();
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
			skipWS();
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
		skipWS();
		
		double doubleTail = tail;
		while (--count >= 0) {
			doubleTail /= 10;
		}
		doubleNum = intNum + doubleTail;
	}

	if (isChar('e') || isChar('E')) {

		// 科学计数法字符 E 后面必须有数字
		if (!isNum(1)) {
			skipWS();
			addError("expect: number after 'E'");
			return { false, std::move(KsonNum(false, 0, 0.0)) };
		}

		++m_idx;
		
		int tail = 0;      // 记录 E 后面的数字
		while (isNum()) {
			tail = tail * 10 + m_str[m_idx] - 0x30;
			++m_idx;
		}
		skipWS();
		
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
	skipWS();

	return { hasNum, std::move(KsonNum(isInt, intNum, doubleNum)) };
}

// parseBool: true / TRUE / false / FALSE
std::pair<bool, KsonBool> Kson::parseBool(const std::string& format) {
	DEBUG(std::string("parseBool: '") + std::string(1, m_str[m_idx]) + "'");

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

	skipWS();

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
	DEBUG(std::string("parseNull: '") + std::string(1, m_str[m_idx]) + "'");

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

	skipWS();

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
	DEBUG(std::string("parseKey: '") + std::string(1, m_str[m_idx]) + "'");

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
	skipWS();

	return { true, std::move(str) };
}

// parseValue
std::pair<bool, KsonValue> Kson::parseValue(const std::string& format) {
	DEBUG(std::string("parseValue: '") + std::string(1, m_str[m_idx]) + "'");

	KsonValue value;
	// object
	if (isChar('{')) {
		auto ret = parseObject(F);
		skipWS();
		value.m_object = std::move(ret.second);
		value.m_type = KsonType::OBJECT;
		return { ret.first, std::move(value) };
	}

	// array
	else if (isChar('[')) {
		auto ret = parseArray(F);
		skipWS();
		value.m_array = std::move(ret.second);
		value.m_type = KsonType::ARRAY;
		return { ret.first, std::move(value) };
	}

	// string
	else if (isChar('"')) {
		auto ret = parseStr(F);
		skipWS();
		value.m_str = std::move(ret.second);
		value.m_type = KsonType::STRING;
		return { ret.first, std::move(value) };
	}

	// number
	else if (isChar('+') || isChar('-') || isNum()) {
		auto ret = parseNum(F);
		skipWS();
		value.m_num = ret.second;
		value.m_type = KsonType::NUMBER;
		return { ret.first, std::move(value) };
	}

	// bool
	else if (isChar('t') || isChar('T') || isChar('f') || isChar('F')) {
		auto ret = parseBool(F);
		skipWS();
		value.m_bool = ret.second;
		value.m_type = KsonType::BOOL;
		return { ret.first, std::move(value) };
	}

	// null
	else if (isChar('n') || isChar('N')) {
		auto ret = parseNull(F);
		skipWS();
		value.m_null = nullptr;
		value.m_type = KsonType::NUL;
		return { ret.first, std::move(value) };
	}
	
	// others
	else {
		skipWS();
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

	auto valid = [this](char arg) -> bool {
		return m_str[m_idx] == arg;
	};

	// charactor: '0' (ASCII: 48) not supported!
	if (!std::any_of(VALID_CHARACTOR.cbegin(), VALID_CHARACTOR.cend(), valid)) {

		// '\0' 不报异常
		if (m_str[m_idx] == END_OF_FILE) return;

		char uc = m_str[m_idx];
		std::string errInfo = "charactor: '";
		errInfo.push_back(uc);
		errInfo += "' (ASCII: " + std::to_string(int(uc)) + ") not supported!";
		addError(errInfo);
		throw KSON_UNEXPECTED_CHARACTOR();
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
	auto valid = [this, c] (char arg) -> bool {
		return arg == c;
	};

	std::string str = VALID_CHARACTOR;
	str.pop_back();
	return std::any_of(str.begin(), str.end(), valid);
}




//============================================================
//  ksonTest: Kson解析器的测试类
//============================================================

void KsonTest::runAllTest(const std::string& fileName) {
	bool tempToFile = m_toFile;
	m_toFile = !fileName.empty();

	std::vector<std::string> files = {
		"test_case/test_normal.kson",
		"test_case/test_space.kson"
	};

	for (auto file : files) {
		print(std::string("======== ") + file + " ========\n");
		Kson ks(file);
		auto obj = ks.parse();
		if (!obj.first) {
			print("ERROR OCCURED!\n");
		}
		printVisualize(obj.second);
		print("\n");
	}

	m_toFile = tempToFile;
}

void KsonTest::printVisualize(const KsonObject& val) {
	print("{\n");
	printObject(val, "    ");
	print("}\n", true);
}

void KsonTest::printObject(const KsonObject& obj, const std::string& format) {

	int size = obj.size();
	int index = 0;

	for (auto p : obj) {
		print(format + p.first + ": ");
		auto val = p.second;

		printValue(val, format, true);

		if (index < size-1) print(",");
		print("\n");
		++index;
	}
}

void KsonTest::printArray(const KsonArray& arr, const std::string& format) {

	int size = arr.size();
	int index = 0;

	for (auto val : arr) {

		printValue(val, format, false);

		if (index < size-1) print(",");
		print("\n");
		++index;
	}
}

void KsonTest::printValue(const KsonValue& val, const std::string& format, bool fromObject) {

	switch (val.m_type) {
	case KsonType::OBJECT:
		if (fromObject) print("{\n");
		else print(format + "{\n");
		printObject(val.m_object, F);
		print(format + "}");
		break;

	case KsonType::ARRAY:
		if (fromObject) print("[\n");
		else print(format + "[\n");
		printArray(val.m_array, F);
		print(format + "]");
		break;

	case KsonType::STRING: {
		std::string str;
		if (!fromObject) str += format;
		print(str + "\"" + val.m_str + "\"");
		break;
	}

	case KsonType::NUMBER: {
		auto num = val.m_num;
		std::string str;
		if (!fromObject) str += format;
		if (num.m_isInt) print(str + std::to_string(num.m_int));
		else print(str + std::to_string(num.m_double));
		break;
	}

	case KsonType::BOOL: {
		std::string str;
		if (!fromObject) str += format;
		print(str + (val.m_bool ? "true" : "false"));
		break;
	}

	case KsonType::NUL: {
		std::string str;
		if (!fromObject) str += format;
		print(str + "null");
		break;
	}

	default:
		throw 1;
	}
}

void KsonTest::print(const std::string& info, bool close) {
	static std::ofstream outFile(m_fileName);

	if (m_toFile) outFile << info;
	else std::cout << info;

	if (close) outFile.close();
}
