#pragma once

#include "Halide.h"
using namespace Halide;

class SubLogic
{
public:
	Func input;
	int _width;
	int _height;
	int _channels;

	SubLogic();
	SubLogic(Func func, int width, int height, int channels);
	~SubLogic();
	static SubLogic Init(int width, int height, int channels);
	static SubLogic Init(int * src, int width, int height, int channels);
	SubLogic Offset(int value);
	void Finalize();
	void Finalize(int * dst);
	void Finalize(uint8_t * dst);
};

