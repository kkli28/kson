// kson.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "kson.h"
#include <iostream>
#include <fstream>

int main()
{
	kson::KsonTest test;
	test.runAllTest();

	system("pause");
    return 0;
}

