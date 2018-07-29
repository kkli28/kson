// kson.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ktest.h"

using namespace kson;

int main()
{
	KsonTest test;
	test.runAllTest(KsonTestType::ONLY_RESULT);

	system("pause");
    return 0;
}
