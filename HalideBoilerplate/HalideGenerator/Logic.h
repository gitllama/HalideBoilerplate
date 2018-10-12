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

	static Logic init();
	static Logic init(int dim);
	static Logic init(ImageParam src);

	static Logic init(int * src, int width, int height);
	static Logic init(int * src, int channels, int width, int height);

	Logic Trim(int left, int top, int width, int height);

	template<typename T, typename... Ts>
	void speedTest(T * dst, Ts... args);

	template<typename T, typename... Ts> 
	void Realize(T * dst, Ts... args);

	template<typename T>
	void RealizeTrim(T * dst, Param<int> left, Param<int> top, Param<int> width, Param<int> height);

	void compileWithRuntime(std::string path, std::string name, std::vector<Argument> arg);
	void compile(std::string path, std::string name, std::vector<Argument> arg);


	Logic sort(ImageParam src);

	Logic HNR();

	Logic PreProcessSchedule();

	Logic sortSchedule();

	Logic BeforeDemosaicProcessSchedule();

	Logic Stagger(Param<int> value);

	Logic Dark(ImageParam value);

	Logic Offset(Param<int> value);

	Logic Bitshift(Param<int> value);

	Logic Trim(Param<int> left, Param<int> top, Param<int> width, Param<int> height);

	Logic Demosaic(ImageParam src, Param<int> type);

	Logic DemosaicMono(ImageParam src);

	Logic DemosaicBitshift(Param<int> value);

	Logic DemosaicLUT(ImageParam lut);

	Logic DemosaicHistogram(ImageParam src);

	Logic DemosaicMonoSchedule();

	Func _DemosaicColor(ImageParam src, int a, int b);

	Logic DemosaicColor(ImageParam src, int row, int col);

	Logic Transform(ImageParam matrix);

	Logic toInt();

	Logic LUT2Byte(ImageParam lut);

	Logic LUT2Byte2(ImageParam lut);

	Func bayerInterpolation(Func input, int type);


	Logic DemosaicColorSchedule();

	Logic Convert_XYC2CXY();



};


//テンプレートはヘッダで実装しないと、コンパイル時に実体が生成されないので
//リンカで落ちる

template<typename T, typename... Ts>
void Logic::speedTest(T * dst, Ts... args)
{
	std::chrono::system_clock::time_point  start, end;
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << ":";
		start = std::chrono::system_clock::now();

		//if (input.dimensions() == 3)
		//{
		//	//int n = sizeof...(args)
		//	//(*this).Realize(dst, 3, width, height);
		//}
		//else if (input.dimensions() == 1)
		//{
		//	//(*this).Realize(dst, width);
		//}
		//else 
		//{
			//(*this).Realize(dst, width, height);
		//}
		(*this).Realize(dst, args...);

		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		printf("%f ms\r\n", elapsed);
	}
}



template<typename T, typename... Ts> 
void Logic::Realize(T * dst, Ts... args)
{
	//switch (sizeof...(args))
	//{
	//	case 1:
	//		Buffer<T> _dst(dst, args[0]);
	//		input.realize(_dst);
	//		break;
	//	case 2:
	//		Buffer<T> _dst(dst, args[0], args[1]);
	//		input.realize(_dst);
	//		break;
	//	case 3:

			//break;

	//}
	Buffer<T> _dst(dst, args...);
	input.realize(_dst);
}

template<typename T>
void Logic::RealizeTrim(T * dst, Param<int> left, Param<int> top, Param<int> width, Param<int> height)
{
	Func output, trim;
	Var x("x"), y("y"), c("c");

	Buffer<T> shifted(width, height);
	shifted.set_min(left, top);

	input.realize(shifted);

	/*
		for (int y = 0; y < input.height(); y++) {
			for (int x = 0; x < input.width(); x++) {
				int i = shifted(x, y); 
		で範囲外のアクセスは可能（ゴミが吐かれる
	
		dstは当然
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int i = dst[x + y * width];

		input(x, y) = src(x - 1, y) + src(x + 1, y);
		のような場合でもshiftedでrealizeした場合は境界条件設定を
		行わなくてもOK（範囲外にアクセスすれば当然エラーを吐く
	*/
}