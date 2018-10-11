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

	Logic Transform(ImageParam matrix);

	Logic ToInt();

	Logic LUT2Byte(ImageParam lut);

	Func AAA(Func input, int type);

	Func AAA(std::string type);

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

//template<typename T>
//void Logic::Realize(T * dst, int n)
//{
//	Buffer<T> _dst(dst, n);
//	input.realize(_dst);
//}

//template<typename T>
//void Logic::Realize(T * dst, int width, int height)
//{
//	Buffer<T> _dst(dst, width, height);
//	input.realize(_dst);
//}

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