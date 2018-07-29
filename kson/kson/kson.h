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
kson ֧�� 6 ����������:
object:    ����
array:     ����
string:    �ַ���
number:    ����
bool:      ����ֵ
null:      ��

ͬʱ��kson֧������ע�ͣ���ע�ͺͿ�ע�ͣ�����ͬ C �����е�ע��

======== object: ���� ========
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
a : true,
b : {
//...
}
}
d : [1, "abc"]
}

======== array: ���� ========

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

======== string: �ַ��� ========

// ---- ��ʽ ----
"abcd1234!@#$"    // ��ͨ�ַ�
"\t \n \" \\"     // ת���ַ�

======== number: ��ֵ ========

// ---- ʾ�� ----

// ����
1234    // ʮ����
0xab    // ʮ������
12e3    // ��ѧ������

+0xab
-12e3

// ������
1.1
2.2e3

======= ����ֵ =======

// ---- ʾ�� ----
true
false
TRUE
FALSE

======== null: �� ========

// ---- ʾ�� ----
null
NULL

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

	struct KsonNum {
		KsonNum(bool b, int i, double d) : m_isInt(b), m_int(i), m_double(d) {}

		bool m_isInt;
		int m_int;
		double m_double;
	};

	class KsonValue {
	public:

		// friend: kson ��������kson ������
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
//  kson������
//============================================================

namespace kson {

	class Kson
	{
	public:
		// ͨ�������ļ�������kson�ַ��������н���
		Kson(const std::string& str, bool isFile = true);
		
		// ��������
		std::pair<bool, KsonObject> parse();
		
		// ��ȡ���������еĴ�����Ϣ
		std::string getErrorInfo() { return m_error; }

		// �����Ƿ���ȷ�������ļ�
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

		// �Ϸ����ַ����ַ�
		bool isValidStrChar(char c);

		// �����հס�ע�ͣ����������ַ��Ƿ�֧��
		void skipWS();
		void skipComment();

		// ��Ӵ�����Ϣ
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
		std::string m_str;      // json�ı�
		std::string m_error;    // ������Ϣ
		int m_idx = 0;          // ��ǰ������λ��
		int m_line = 1;         // ��ǰ�кţ��ӵ�һ�п�ʼ��

		const std::string VALID_CHARACTOR = "~!@#$%^&*()_+`1234567890-=qwertyuiopQWERTYUIOP{}|[]\\asdfghjklASDFGHJKL:;'zxcvbnmZXCVBNM<>?,./\"";  // ˫��������󣬱����ַ�������
		using KSON_UNEXPECTED_CHARACTOR = int;
	};
}

#endif
