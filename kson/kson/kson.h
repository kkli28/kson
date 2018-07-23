#ifndef __KSON__H__
#define __KSON_H__

#include <map>
#include <vector>
#include <string>
#include <iostream>

//============================================================
//  kson��ʽ����
//============================================================

/*
kson ֧������ 5 ���������ͣ��Լ������� null

======== ���� ========
// key ����˫���Ű���
// value �������κ� kson ֧�ֵ���������

// ---- ��ʽ ----
{
    key : value
	key : value
}

// ---- ʾ�� ----

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

======== ���� ========

// ---- ��ʽ ----

[
    value, value, value  //...
]

// ---- ʾ�� ----

[
    1,
	"abc",
	{
	    a : 1
	},
	[1, "abc", true]
}

======== ��ֵ ========

// ---- ʾ�� ----

// ����
0101    // ������
0123    // �˽���
1234    // ʮ����
0xab    // ʮ������
12e3    // ��ѧ������

+0xab
-12e3

// ������
1.1
2.2e3

======= �ַ��� =======

// ---- ʾ�� ----
"123"
"abc\nd"
"!@#%$^"

======= ����ֵ =======

// ---- ʾ�� ----
true
false

*/

//============================================================
//  kson���ݽṹ
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

	// Ϊ number ����һ���෽�� parse
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

		// ��ȡ KsonType
		KsonType     getType()   const { return m_type; }
		
		// ��ȡֵ
		KsonObject   getObject() const { return m_object; }
		KsonArray    getArray()  const { return m_array; }
		KsonStr      getStr() const { return m_str; }
		KsonInt      getInt()    const { return m_num.m_int; }
		KsonDouble   getDouble() const { return m_num.m_double; }
		KsonNull     getNull()   const { return m_null; }

		// �� m_object[key] �л�ȡ KsonObject
		// �� m_array[index] �л�ȡ KsonObject
		KsonObject   getObject(const std::string& key) { return m_object.at(key).m_object; }
		KsonObject   getObject(int index) { return m_array.at(index).m_object; }

		// �� m_object[key] �л�ȡ KsonArray
		// �� m_array[index] �л�ȡ KsonArray
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

		// ���ڱ�ʶ�Ϸ���һ�������洢��������ֵ
		KsonType        m_type = KsonType::OBJECT;
	};
}



//============================================================
//  kson������
//============================================================

namespace kson {
	
	class Kson
	{
	public:
		Kson(const std::string& fileName);

		std::pair<bool, KsonObject> parse();
		std::string getErrorInfo() { return m_error; }

		void testPrint() { std::cout << m_str << std::endl; }
		
		//TEST !!!!!!! Ϊ�˲������Ը�Ϊpublic
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
		std::string m_str;      // json�ı�
		std::string m_error;    // ������Ϣ
		int m_idx = 0;          // ��ǰ������λ��
		int m_line = 0;         // ��ǰ�к�
	};
}



//============================================================
//  ksonTest: Kson�������Ĳ�����
//============================================================

namespace kson {
	
	class KsonTest {
	public:
		KsonTest(const std::string& fileName = "") : m_fileName(fileName), m_toFile(!fileName.empty()) {}

		void runAllTest(const std::string& fileName = "");
		
		// �Կ��ӻ��ķ�ʽ�������ļ�������� ksonValue
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
