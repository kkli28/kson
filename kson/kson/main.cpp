// kson.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "kson.h"
#include <iostream>
#include <fstream>

int main()
{
	kson::Kson kson("kson.txt");
	kson.testPrint();
	auto str = kson.parseStr("");
	std::cout << (str.first ? "true" : "false") << "  " << str.second << std::endl;

	system("pause");
    return 0;
}

