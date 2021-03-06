// HalideGenerated.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>
#include "Halide.h"

#include "HalideGenerated_zeroFill.h"

#include "HalideGenerated_demosaicRG.h"
#include "HalideGenerated_demosaicGR.h"
#include "HalideGenerated_demosaicBG.h"
#include "HalideGenerated_demosaicGB.h"
#include "HalideGenerated_demosaicMONO.h"

#include "HalideGenerated_demosaicRGwLUT.h"
#include "HalideGenerated_demosaicGRwLUT.h"
#include "HalideGenerated_demosaicGBwLUT.h"
#include "HalideGenerated_demosaicBGwLUT.h"

#include "HalideGenerated_sort.h"


#ifdef __cplusplus
#define DLLEXPORT extern "C" __declspec(dllexport)
#define DLLIMPORT extern "C"
#else
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT 
#endif



DLLEXPORT int Test()
{
	return 10;
}

DLLEXPORT void ZeroFill(int* src, int* dst, int left, int top, int width, int height)
{
	// Compile時にはoutputのサイズを規定できない為、
	// Trimを使用する場合は呼び出し側で工夫する必要がある

	Halide::Runtime::Buffer<int> output(dst, width, height);
	output.set_min(left, top);

	zeroFill(output);
}

DLLEXPORT void Sort24To32(int* src, int* dst, int width, int height)
{
	Halide::Runtime::Buffer<int> input(src, width * height * 3 / 4);
	Halide::Runtime::Buffer<int> output(dst, width, height);

	sort(input, output);
}

DLLEXPORT void Demosaic(int* src, int* dst, int width, int height, float* matrix, int type)
{
	Halide::Runtime::Buffer<int> input(src, width, height);
	Halide::Runtime::Buffer<int> output(dst, 3, width, height);

	switch (type)
	{
	case 1:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicRG(input, m, output);
	}
	break;
	case 2:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicGR(input, m, output);
	}
	break;
	case 3:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicGB(input, m, output);
	}
	break;
	case 4:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicBG(input, m, output);
	}
	break;
	default:
		demosaicMONO(input, output);
		break;
	}
}

DLLEXPORT void DemosaicLUT(int* src, int srcWidth, int srcHeight, uint8_t* dst, int left, int top, int width, int height, float* matrix, uint8_t* lut, int lutSize, int type)
{
	Halide::Runtime::Buffer<int> input(src, srcWidth, srcHeight);
	Halide::Runtime::Buffer<uint8_t> output(dst, 3, width, height);
	output.set_min(0, left, top);

	Halide::Runtime::Buffer<uint8_t> _lut(lut, lutSize);

	switch (type)
	{
	case 1:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicRGwLUT(input, m, _lut, output);
	}
	break;
	case 2:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicGRwLUT(input, m, _lut, output);
	}
	break;
	case 3:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicGBwLUT(input, m, _lut, output);
	}
	break;
	case 4:
	{
		Halide::Runtime::Buffer<float> m(matrix, 3, 3);
		demosaicBGwLUT(input, m, _lut, output);
	}
	break;
	default:
		//MONO(input, output);
		break;
	}
}

int main()
{
    std::cout << "Hello World!\n"; 
}

