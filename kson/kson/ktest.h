#ifndef __K_TEST_H__
#define __K_TEST_H__

#include "kson.h"

#define F (format + "    ")
#define KSON_TEST_DEBUG_MODE false
#define KSON_TEST_DEBUG(arg) { if (KSON_TEST_DEBUG_MODE) { std::cout<<format<<arg; }}

//============================================================
//  ksonTest: Kson解析器的测试类
//============================================================

namespace kson {

	enum class KsonTestType {
		ONLY_RESULT,
		PRINT_VISUALIZE
	};

	class KsonTest {
	public:

		void runAllTest(KsonTestType type = KsonTestType::ONLY_RESULT);

		// 以可视化的方式输出 ksonValue
		void printVisualize(const KsonObject& obj);

	private:
		
		// 测试各种文件，比较kson解析出的内容是否与预期相同
		void testObject();
		void testArray();
		void testString();
		void testNumber();
		void testBool();
		void testNull();
		void testAll1();
		void testAll2();
		void testAll3();
		void testAll4();
		void testAll5();
		void testAll6();
		void testSpace();
		void testComment();
		KsonObject testTwoKson(const std::string& ksonStr, const std::string& ksonFile);

		void printObject(const KsonObject& obj, const std::string& format);
		void printArray(const KsonArray& arr, const std::string& format);
		void printValue(const KsonValue& val, const std::string& format, bool fromObject);
		void print(const std::string& info);

		// 工具函数
	private:

		// expectEQ <T>
		template<typename T>
		void expectEQ(const T& val1, const T& val2, const std::string& format) const {
			KSON_TEST_DEBUG(std::string("expectEQ <") + typeid(T).name() + ">\n");
			if (val1 != val2) {
				std::cout << val1 << " != " << val2 << std::endl;
				throw 1;
			}
		}

		// expect <bool>
		template<>
		void expectEQ<bool>(const bool& val1, const bool& val2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <bool>\n");
			if (val1 != val2) {
				std::cout << (val1 ? "true" : "false") << " != " << (val2 ? "true" : "false") << std::endl;
				throw 1;
			}
		}
		
		// expectEQ <KsonType>
		template<>
		void expectEQ<KsonType>(const KsonType& val1, const KsonType& val2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <KsonType>\n");
			auto getKsonTypeName = [=](const KsonType& v) -> std::string {
				switch (v) {
				case KsonType::OBJECT: return "OBJECT";
				case KsonType::ARRAY: return "ARRAY";
				case KsonType::STRING: return "STRING";
				case KsonType::NUMBER: return "NUMBER";
				case KsonType::BOOL: return "BOOL";
				case KsonType::NUL: return "NUL";
				}
			};
			if (val1 != val2) {
				std::cout << getKsonTypeName(val1) << " != " << getKsonTypeName(val2) << std::endl;
			}
		}
		
		// expectEQ <KsonValue>
		template<>
		void expectEQ<KsonValue>(const KsonValue& val1, const KsonValue& val2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <KsonValue>\n");
			expectEQ(val1.m_type, val2.m_type, F);
			switch (val1.m_type) {
			case KsonType::OBJECT: expectEQ(val1.m_object, val2.m_object, F); break;
			case KsonType::ARRAY:  expectEQ(val1.m_array, val2.m_array, F); break;
			case KsonType::STRING: expectEQ(val1.m_str, val2.m_str, F); break;
			case KsonType::NUMBER: expectEQ(val1.m_num, val2.m_num, F); break;
			case KsonType::BOOL:   expectEQ(val1.m_bool, val2.m_bool, F); break;
			case KsonType::NUL:    expectEQ(val1.m_null, val2.m_null, F); break;
			default: throw 1;
			}
		}

		// expectEQ <KsonObject>
		template<>
		void expectEQ<KsonObject>(const KsonObject& obj1, const KsonObject& obj2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <KsonObject>\n");
			expectEQ(obj1.size(), obj2.size(), F);
			auto iter1 = obj1.begin();
			auto iter2 = obj2.begin();
			while (iter1 != obj1.end() && iter2 != obj2.end()) {
				expectEQ(iter1->first, iter2->first, F);
				expectEQ(iter1->second.m_type, iter2->second.m_type, F);
				expectEQ(iter1->second, iter2->second, F);
				++iter1;
				++iter2;
			}
		}

		// expectEQ <KsonArray>
		template<>
		void expectEQ<KsonArray>(const KsonArray& arr1, const KsonArray& arr2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <KsonArray>\n");
			size_t size = arr1.size();
			expectEQ(size, arr2.size(), F);
			for (size_t i = 0; i < size; ++i) {
				expectEQ(arr1[i], arr2[i], F);
			}
		}
		
		// expectEQ <KsonNum>
		template<>
		void expectEQ<KsonNum>(const KsonNum& num1, const KsonNum& num2, const std::string& format) const {
			KSON_TEST_DEBUG("expectEQ <KsonNum>\n");
			expectEQ(num1.m_isInt, num2.m_isInt, F);
			if (num1.m_isInt) {
				expectEQ(num1.m_int, num2.m_int, F);
			}
			else {
				expectEQ(num1.m_double, num2.m_double, F);
			}
		}
	};
}

#endif
