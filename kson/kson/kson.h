#ifndef __KSON__H__
#define __KSON_H__

#include <map>
#include <vector>
#include <string>
#include <iostream>

//============================================================
//  kson格式介绍
//============================================================

/*
kson 支持 6 种数据类型:
object:    对象
array:     数组
string:    字符串
number:    数字
bool:      布尔值
null:      空

同时，kson支持两种注释：行注释和块注释，定义同 C 语言中的注释

======== object: 对象 ========
// key 不用双引号包括
// value 可以是任何 kson 支持的数据类型

// ---- 格式 ----
{
key : value
key : value
}

// ---- 示例 ----

{
a : 1,
b : "123",
c : {
a : true,
b : {
//...
}
}
d : [1, "abc"]
}

======== array: 数组 ========

// ---- 格式 ----

[
value, value, value  //...
]

// ---- 示例 ----

[
1,
"abc",
{
a : 1
},
[1, "abc", true]
}

======== string: 字符串 ========

// ---- 格式 ----
"abcd1234!@#$"    // 普通字符
"\t \n \" \\"     // 转义字符

======== number: 数值 ========

// ---- 示例 ----

// 整数
1234    // 十进制
0xab    // 十六进制
12e3    // 科学计数法

+0xab
-12e3

// 浮点数
1.1
2.2e3

======= 布尔值 =======

// ---- 示例 ----
true
false
TRUE
FALSE

======== null: 空 ========

// ---- 示例 ----
null
NULL

*/


//============================================================
//  kson数据结构
//============================================================

namespace kson {

	enum class KsonType {
		OBJECT,
		ARRAY,
		STRING,
		NUMBER,
		BOOL,
		NUL
	};

	class KsonValue;

	using KsonObject = std::map<std::string, KsonValue>;
	using KsonArray = std::vector<KsonValue>;
	using KsonStr = std::string;
	using KsonInt = int;
	using KsonDouble = double;
	using KsonBool = bool;
	using KsonNull = void*;

	struct KsonNum {
		KsonNum(bool b, int i, double d) : m_isInt(b), m_int(i), m_double(d) {}

		bool m_isInt;
		int m_int;
		double m_double;
	};

	class KsonValue {
	public:

		// friend: kson 解析器，kson 测试类
		friend class Kson;
		friend class KsonTest;

		// 获取 KsonType
		KsonType     getType()   const { return m_type; }

		// 获取值
		KsonObject   getObject() const { return m_object; }
		KsonArray    getArray()  const { return m_array; }
		KsonStr      getStr() const { return m_str; }
		KsonInt      getInt()    const { return m_num.m_int; }
		KsonDouble   getDouble() const { return m_num.m_double; }
		KsonNull     getNull()   const { return m_null; }

		// 从 m_object[key] 中获取 KsonObject
		// 从 m_array[index] 中获取 KsonObject
		KsonObject   getObject(const std::string& key) { return m_object.at(key).m_object; }
		KsonObject   getObject(int index) { return m_array.at(index).m_object; }

		// 从 m_object[key] 中获取 KsonArray
		// 从 m_array[index] 中获取 KsonArray
		KsonArray    getArray(const std::string& key) { return m_object.at(key).m_array; }
		KsonArray    getArray(int index) { return m_array.at(index).m_array; }

	public:
		KsonValue();
		KsonValue(
			KsonType type,
			KsonObject&& obj, KsonArray&& arr, KsonStr&& str,
			KsonNum&& num, KsonBool bol, KsonNull nul
		) : m_type(type), m_object(std::move(obj)), m_array(std::move(arr)), m_str(std::move(str)), 
			m_num(std::move(num)), m_bool(bol), m_null(nul) {}
		
	private:
		KsonObject      m_object;   // object
		KsonArray       m_array;    // array
		KsonStr         m_str;      // string
		KsonNum         m_num;      // number
		KsonBool        m_bool;     // bool
		KsonNull        m_null;     // null

		KsonType        m_type = KsonType::OBJECT;
	};
}


//============================================================
//  kson解析器
//============================================================

namespace kson {

	class Kson
	{
	public:
		// 通过传入文件名，或kson字符串来进行解析
		Kson(const std::string& str, bool isFile = true);
		
		// 解析函数
		std::pair<bool, KsonObject> parse();
		
		// 获取解析过程中的错误信息
		std::string getErrorInfo() { return m_error; }

		// 测试是否正确读入了文件
		void printFile() { std::cout << m_str << std::endl; }

	private:
		std::pair<bool, KsonObject>   parseObject(const std::string& format);
		std::pair<bool, KsonArray>    parseArray(const std::string& format);
		std::pair<bool, KsonStr>      parseStr(const std::string& format);
		std::pair<bool, KsonNum>      parseNum(const std::string& format);
		std::pair<bool, KsonBool>     parseBool(const std::string& format);
		std::pair<bool, KsonNull>     parseNull(const std::string& format);

		std::pair<bool, std::string>  parseKey(const std::string& format);
		std::pair<bool, KsonValue>    parseValue(const std::string& format);
		std::pair<bool, KsonNum>      parseHex(const std::string& format);

		// 合法的字符串字符
		bool isValidStrChar(char c);

		// 跳过空白、注释，并检查后续字符是否支持
		void skipWS();
		void skipComment();

		// 添加错误信息
		void addError(std::string errInfo);
		void addError(std::string errInfo, char appChar);

	private:

		// inlines
		inline bool isChar(int offset, char c) { return m_str[m_idx + offset] == c; }
		inline bool isChar(char c) { return isChar(0, c); }
		inline bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
		inline bool isNumber(char c) { return (c >= '0' && c < '9'); }
		inline bool isNum(int offset = 0) {
			return (m_str[m_idx + offset] >= '0' && m_str[m_idx + offset] <= '9');
		}

	private:
		std::string m_str;      // json文本
		std::string m_error;    // 错误信息
		int m_idx = 0;          // 当前解析的位置
		int m_line = 1;         // 当前行号（从第一行开始）

		const std::string VALID_CHARACTOR = "~!@#$%^&*()_+`1234567890-=qwertyuiopQWERTYUIOP{}|[]\\asdfghjklASDFGHJKL:;'zxcvbnmZXCVBNM<>?,./\"";  // 双引号在最后，便于字符串解析
		using KSON_UNEXPECTED_CHARACTOR = int;
	};
}

#endif
