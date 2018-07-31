#include "stdafx.h"
#include "ktest.h"

using namespace kson;

int main()
{
	KsonTest test;
	//test.runAllTest(KsonTestType::ONLY_RESULT);
	test.runAllTest(KsonTestType::PRINT_VISUALIZE);

	system("pause");
    return 0;
}
