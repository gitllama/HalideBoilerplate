#pragma once

#include "Halide.h"
using namespace Halide;

class Logic
{
public:
	Func input;
	int _width;
	int _height;
	int _channels;

	Logic();
	Logic(Func input, int width, int height, int channels);
	~Logic();

	static Logic Init(int width, int height, int channels);

	static Logic Init(int * src, int width, int height, int channels);

	Logic Offset(int value);

	void Finalize();

	void Finalize(int * dst);

	void Finalize(uint8_t * dst);

	Logic DemosaicMono(int * src);

	Logic Demosaic(int * src, int a, int b);

	Logic Clamp(int * src, int left, int top);

	Logic Clamp();



	static Logic Init(ImageParam src);



	Logic DemosaicMono(int * src, int width, int height);

	Logic Offset(Param<int> value);

	Logic Demosaic(Param<int> type);

	Logic ToByte();

	void Compile(std::string path, std::vector<Argument> arg, std::string name);

	void Finalize(int * dst, int width, int height);

	void Finalize(unsigned char * dst, int width, int height);



};

