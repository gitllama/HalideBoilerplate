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
	Logic(Func func);
	~Logic();

	static Logic Init(ImageParam src);
	static Logic Init();

	static Logic Init(int * src, int channels, int width, int height);

	void Realize(int * dst, int channels, int width, int height);

	void Compile(std::string path, std::vector<Argument> arg, std::string name);

	void Compile(Halide::Module module, std::vector<Argument> arg, std::string name);


	Logic Offset(Param<int> value);

	Logic Demosaic(ImageParam src, Param<int> type);

	Logic DemosaicMono(ImageParam src);

	Logic DemosaicMonoSchedule();

	Func _DemosaicColor(ImageParam src, int a, int b);

	Logic DemosaicColor(ImageParam src, int row, int col);

	Logic DemosaicColorSchedule();






	Logic Clamp(int * src, int left, int top);

	Logic Clamp();






	Logic Demosaic(Param<int> type);

	Logic ToByte();

};

