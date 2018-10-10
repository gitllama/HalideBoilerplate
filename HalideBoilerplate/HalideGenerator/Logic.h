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



	void printArray(int * _src, int * _dst, int w, int h);

	static Logic Init();
	static Logic Init(int dim);
	static Logic Init(ImageParam src);

	static Logic Init(int * src, int width, int height);
	static Logic Init(int * src, int channels, int width, int height);

	template<typename T, typename... Ts>
	void speedTest(T * dst, Ts... args);

	//template<typename T>
	//void Realize(T * dst, int n);

	//template<typename T>
	//void Realize(T * dst, int width, int height);

	template<typename T, typename... Ts>
	void Realize(T * dst, Ts... args);

	void CompileWithRuntime(std::string path, std::vector<Argument> arg, std::string name);
	void Compile(std::string path, std::vector<Argument> arg, std::string name);


	Logic Sort(ImageParam src);

	Logic HNR();

	Logic PreProcessSchedule();

	Logic SortSchedule();

	Logic BeforeDemosaicProcessSchedule();

	Logic Stagger(Param<int> value);

	Logic Dark(ImageParam value);

	Logic Offset(Param<int> value);

	Logic Bitshift(Param<int> value);

	Logic Demosaic(ImageParam src, Param<int> type);

	Logic DemosaicMono(ImageParam src);

	Logic DemosaicBitshift(Param<int> value);

	Logic DemosaicLUT(ImageParam lut);

	Logic DemosaicHistogram(ImageParam src);

	Logic DemosaicMonoSchedule();

	Func _DemosaicColor(ImageParam src, int a, int b);

	Logic DemosaicColor(ImageParam src, int row, int col);

	Logic DemosaicColorSchedule();



};



