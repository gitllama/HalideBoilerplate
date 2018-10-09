#pragma once

#include "Logic.h"
#include "Halide.h"
using namespace Halide;

class Util
{
public:
	Util();
	~Util();
	static void speedTest(Logic code, int * _dst, int w, int h);
	static void printArray(int * _src, int * _dst, int w, int h);
};

