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
kson 支持以下 5 种数据类型，以及空类型 null

======== 对象 ========
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
	        a : 1,
			b : {
			        //...
			    }
		}
	d : [1, "abc"]
}

======== 数组 ========

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

======== 数值 ========

// ---- 示例 ----

// 整数
0101    // 二进制
0123    // 八进制
1234    // 十进制
0xab    // 十六进制
12e3    // 科学计数法

+0xab
-12e3

// 浮点数
1.1
2.2e3

======= 字符串 =======

// ---- 示例 ----
"123"
"abc\nd"
"!@#%$^"

======= 布尔值 =======

// ---- 示例 ----
true
false

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

	// 为 number 定义一个类方便 parse
	struct KsonNum {
		KsonNum(bool b, int i, double d) : m_isInt(b), m_int(i), m_double(d) {}

		bool m_isInt;
		int m_int;
		double m_double;
	};

	class KsonValue {
	public:

		// friend
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
		
	private:
		KsonObject      m_object;   // object
		KsonArray       m_array;    // array
		KsonStr         m_str;      // string
		KsonNum         m_num;      // number
		KsonBool        m_bool;     // bool
		KsonNull        m_null;     // null

		// 用于标识上方哪一个变量存储有真正的值
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
		Kson(const std::string& fileName);

		std::pair<bool, KsonObject> parse();
		std::string getErrorInfo() { return m_error; }

		void testPrint() { std::cout << m_str << std::endl; }
		
		//TEST !!!!!!! 为了测试所以改为public
	//private:
	public:
		std::pair<bool, KsonObject>   parseObject(const std::string& format);
		std::pair<bool, KsonArray>    parseArray(const std::string& format);
		std::pair<bool, KsonStr>      parseStr(const std::string& format);
		std::pair<bool, KsonNum>      parseNum(const std::string& format);
		std::pair<bool, KsonBool>     parseBool(const std::string& format);
		std::pair<bool, KsonNull>     parseNull(const std::string& format);

		std::pair<bool, std::string>  parseKey(const std::string& format);
		std::pair<bool, KsonValue>    parseValue(const std::string& format);

		bool isValidStrChar(char c);
		void skipWS();
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
		inline bool next(char c) { skipWS(); return m_str[m_idx] == c; }
		
	private:
		std::string m_str;      // json文本
		std::string m_error;    // 错误信息
		int m_idx = 0;          // 当前解析的位置
		int m_line = 0;         // 当前行号
	};
}



//============================================================
//  ksonTest: Kson解析器的测试类
//============================================================

namespace kson {
	
	class KsonTest {
	public:
		KsonTest(const std::string& fileName = "") : m_fileName(fileName), m_toFile(!fileName.empty()) {}

		void runAllTest(const std::string& fileName = "");
		
		// 以可视化的方式（竖向文件树）输出 ksonValue
		void printVisualize(const KsonObject& obj);

	private:
		void printObject(const KsonObject& obj, const std::string& format);
		void printArray(const KsonArray& arr, const std::string& format);
		void printValue(const KsonValue& val, const std::string& format, bool fromObject);

		void print(const std::string& info, bool close = false);

	private:
		std::string m_fileName;
		bool m_toFile = false;
	};
}

#endif
