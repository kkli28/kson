#include "stdafx.h"
#include "ktest.h"
#include <fstream>
#include <functional>

using namespace kson;

//============================================================
//  ksonTest: Kson解析器的测试类
//============================================================

// runAllTest
void KsonTest::runAllTest(KsonTestType type) {

	// 只显示测试结果
	if (type == KsonTestType::ONLY_RESULT) {
		try {
			//testObject();
			//testArray();
			//testString();
			//testNumber();
			//testBool();
			//testNull();

			//testAll1();
			//testAll2();
			//testAll3();
			//testAll4();
			//testAll5();
			//testAll6();

			testSpace();
			testComment();
		}
		catch (int) {
			print("[ FAIL! ]\n");
		}
	}

	// 显示从测试文件中读出的内容
	else {
		std::vector<std::string> files = {

			// 测试: 所有数据类型的组合
			"test_case/test_all1.kson",
			"test_case/test_all2.kson"
			"test_case/test_all3.kson",
			
			// 测试: 空格 / 注释 / 不支持字符
			"test_case/test_space.kson",
			"test_case/test_comment.kson",
			//"test_case/test_unsurpport.kson",   // 需要单独通过程序进行测试，不知道怎么将不支持字符写入文件。。。
		};

		for (auto file : files) {
			print(std::string("======== ") + file + " ========\n");
			Kson ks(file);
			auto obj = ks.parse();
			if (!obj.first) {
				print("\n######## ERROR ! ########\n\n");
				print(ks.getErrorInfo());
				print("\n");
			}
			printVisualize(obj.second);
			print("\n");
		}
	}
}

// printVisualize
void KsonTest::printVisualize(const KsonObject& val) {
	print("[RESULT]\n{\n");
	printObject(val, "    ");
	print("}\n");
}

// printObject
void KsonTest::printObject(const KsonObject& obj, const std::string& format) {

	int size = obj.size();
	int index = 0;

	for (auto p : obj) {
		print(format + p.first + ": ");
		auto val = p.second;

		printValue(val, format, true);

		if (index < size - 1) print(",");
		print("\n");
		++index;
	}
}

// printArray
void KsonTest::printArray(const KsonArray& arr, const std::string& format) {

	int size = arr.size();
	int index = 0;

	for (auto val : arr) {

		printValue(val, format, false);

		if (index < size - 1) print(",");
		print("\n");
		++index;
	}
}

// printValue
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

// print
void KsonTest::print(const std::string& info) {
	std::cout << info;
}

// testAll1: test_case/test_all1.kson
void KsonTest::testAll1() {
	print("\n==== test: all1 ====\n");

	Kson kson("test_case/test_all1.kson");
	auto ret = kson.parse();
	expectEQ(ret.first, true, "");

	auto obj = std::move(ret.second);
	expectEQ(obj["a"].m_type, KsonType::NUMBER, "");
	expectEQ(obj["b"].m_str, std::string("abcd"), "");
	expectEQ(obj["c"].m_type, KsonType::OBJECT, "");

	auto obj1 = std::move(obj["c"].m_object);
	expectEQ(obj1["d"].m_num.m_int, 1, "");
	//expectEQ(obj1)
	// TODO: 
}

/*
// testAll2: test_case/test_all2.kson
bool KsonTest::testAll2() {

}

// testAll3: test_case/test_all3.kson
bool KsonTest::testAll3() {

}

// testAll4: test_case/test_all4.kson
bool KsonTest::testAll4() {

}

// testAll5: test_case/test_all5.kson
bool KsonTest::testAll5() {

}

// testAll6: test_case/test_all6.kson
bool KsonTest::testAll6() {

}
*/

// testSpace: test_case/test_space.kson
void KsonTest::testSpace() {
	print("\n==== test: space ====\n");

	std::string ksonStr = "{a:1, b:1.1, c:[{d:1, e:1, f:[1, 2, true]}, 3, \"abcd\", true, null]}";
	std::string ksonFile = "test_case/test_space.kson";

	auto obj = testTwoKson(ksonStr, ksonFile);
	expectEQ(obj.at("a").m_num.m_int, 1, "");
	double db = obj.at("b").m_num.m_double;
	expectEQ(db > 1.0 && db < 1.11, true, "");

	KsonArray arr = obj["c"].m_array;
	expectEQ(arr.size(), size_t(5), "");
	expectEQ(arr[0].m_type, KsonType::OBJECT, "");
	
	expectEQ(arr[1].m_type, KsonType::NUMBER, "");
	expectEQ(arr[1].m_num.m_int, 3, "");

	expectEQ(arr[2].m_type, KsonType::STRING, "");
	expectEQ(arr[2].m_str, std::string("abcd"), "");

	expectEQ(arr[3].m_type, KsonType::BOOL, "");
	expectEQ(arr[3].m_bool, true, "");

	expectEQ(arr[4].m_type, KsonType::NUL, "");
	expectEQ(arr[4].m_null, (void*)nullptr, "");

	KsonObject obj1 = arr[0].m_object;
	expectEQ(obj1.size(), size_t(3), "");
	expectEQ(obj1["d"].m_num.m_int, 1, "");
	expectEQ(obj1["e"].m_num.m_int, 1, "");
	expectEQ(obj1["f"].m_array.size(), size_t(3), "");

	KsonArray arr1 = obj1["f"].m_array;
	expectEQ(arr1[0].m_num.m_int, 1, "");
	expectEQ(arr1[1].m_num.m_int, 2, "");
	expectEQ(arr1[2].m_bool, true, "");
	
	print("[ SUCCESS! ]\n");
}

// testComment: test_case/test_comment.kson
void KsonTest::testComment() {
	print("\n==== test: comment ====\n");

	std::string ksonStr = "{ a: 1, b: \"abcd\", c: {}, d: [] }";
	std::string ksonFile = "test_case/test_comment.kson";
	auto obj = testTwoKson(ksonStr, ksonFile);
	
	expectEQ(obj["a"].m_num.m_int, 1, "");
	expectEQ(obj["b"].m_str, std::string("abcd"), "");
	expectEQ(obj["c"].m_type, KsonType::OBJECT, "");
	expectEQ(obj["c"].m_object.size(), size_t(0), "");
	expectEQ(obj["d"].m_type, KsonType::ARRAY, "");
	expectEQ(obj["d"].m_array.size(), size_t(0), "");
	
	print("[ SUCCESS! ]\n");
}

KsonObject KsonTest::testTwoKson(const std::string& ksonStr, const std::string& ksonFile) {
	Kson kson1(ksonStr, false);
	Kson kson2(ksonFile, true);
	auto ret1 = kson1.parse();
	auto ret2 = kson2.parse();
	expectEQ(ret1.first, true, "");
	expectEQ(ret2.first, true, "");
	expectEQ(ret1.second, ret2.second, "");
	return std::move(ret2.second);
}
