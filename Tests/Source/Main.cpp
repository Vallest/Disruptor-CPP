// Disruptor-CPP
// Copyright (c) Vallest Systems LLC. All Rights Reserved.
// Released under the MIT license.

#if !defined(DISRUPTOR_TESTS_LIB)

#include "UnitTesting.h"

#include <fstream>
#include <iostream>
#include <string>

std::ostream& getOutStream()
{
	return std::cout;
}

int main()
{
	runTestCases();
}

#endif // DISRUPTOR_TESTS_LIB

