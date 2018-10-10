// HaildeGenerator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>

#include "Logic.h"
#include "Halide.h"
using namespace Halide;

void hoge() {
	int w = 16;
	int h = 9;

	int* _input = new int[w * h * 3 / 4];

	int* _A = new int[w * h];
	int* _B = new int[w * h];
	int* _afterDemosic = new int[w * h * 3];
	uint8_t* _output = new uint8_t[w * h * 3];


	//センサ特有の処理

	ImageParam input(type_of<int>(), 1);
	Buffer<int> b_input(_input, w * h * 3 / 4);
	input.set(b_input);

	//Logic::Init()
	//	.Sort(input)
	//	.HNR()
	//	//.VNR()
	//	.PreProcessSchedule()
	//	.Realize(_A, w, h);

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			printf("%d ", _A[i + j * w]);
		}
		printf("\r\n");
	}

	//デモザイク前画処理

	ImageParam A(type_of<int>(), 2);
	Buffer<int> b_A(_A, w, h);
	A.set(b_A);

	Param<int> stagger;
	Param<int> offset;
	Param<int> bitshift;
	stagger.set(1);
	offset.set(100);
	bitshift.set(1);

	//Logic::Init(A)
	//	//.Dark()
	//	//.Stagger(stagger)
	//	.Offset(offset)
	//	////.NR()
	//	.Bitshift(bitshift)
	//	.BeforeDemosaicProcessSchedule()
	//	.Realize(_B, w, h);

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			printf("%d ", _B[i + j * w]);
		}
		printf("\r\n");
	}

	//デモザイク

	ImageParam B(type_of<int>(), 2);
	Buffer<int> b_B(_B, w, h);
	B.set(b_B);
}

void example_1() {

	int w = 4;
	int h = 4;
	int c = 1;
	int * _src = new int[w * h * c];
	int * _dst = new int[w * h * c];

	for (int i = 0; i < 4 * 4; i++) {
		_src[i] = i;
	}

	ImageParam input(type_of<int>(), 3);
	Param<int> offset;

	//AOT
	Logic::Init(input)
		.Offset(offset)
		.Compile("test", {input, offset}, "test");


	//JIT
	Buffer<int> src(_src, c, w, h);
	input.set(src);
	offset.set(10);

	//Logic::Init(input)
	//	.Offset(offset)
	//	.Realize(_dst, c, w, h);

	for (int i = 0; i < 4 * 4; i++) {
		printf("%d\r\n", _dst[i]);
	}
}

void example_3() {

	int w = 8000;
	int h = 4000;

	int * _src = new int[w * h];
	int * _dst = new int[w * h * 3];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			_src[x + y * w] = x + y * 10;
		}
	}

	ImageParam input(type_of<int>(), 3);
	Param<int> type;

	Buffer<int> src(_src, 1, w, h);
	input.set(src);
	type.set(1);

	Logic test = Logic::Init()
		.DemosaicColor(input, 0, 0)
		.DemosaicColorSchedule();

	//Util::speedTest(test, _dst, w, h);
	//test.input.compile_to_c("c_code.cpp", { input, type }, "user_func");

	//test.Realize(_dst, 3, w, h);
	//Util::printArray(_src, _dst, w, h);
}

void generateDDL() {

	ImageParam input(type_of<uint32_t>(), 1);

	Logic::Init()
		.Sort(input)
		.SortSchedule()
		.CompileWithRuntime("Sort", { input }, "Sort");


	ImageParam inputC(type_of<int>(), 3);
	Logic::Init()
		.DemosaicColor(inputC, 0, 0)
		.DemosaicColorSchedule()
		.Compile("RG", { inputC }, "_RG");

	Logic::Init()
		.DemosaicColor(inputC, 1, 0)
		.DemosaicColorSchedule()
		.Compile("GR", { inputC }, "_GR");

	Logic::Init()
		.DemosaicColor(inputC, 0, 1)
		.DemosaicColorSchedule()
		.Compile("GB", { inputC }, "_GB");

	Logic::Init()
		.DemosaicColor(inputC, 1, 1)
		.DemosaicColorSchedule()
		.Compile("BG", { inputC }, "_BG");

	Logic::Init()
		.DemosaicMono(inputC)
		.DemosaicMonoSchedule()
		.Compile("MONO", { inputC }, "_MONO");

}

void logicTest() 
{
	int w = 8000;
	int h = 4000;

	int * src = new int[w * h];
	uint8_t * dst = new uint8_t[w * h * 3];

	uint8_t * lut = new uint8_t[INT32_MAX];

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			src[x + y * w] = x + y * 10;
		}
	}

	ImageParam input(type_of<int>(), 2);
	Buffer<int> b_src(src, w, h);
	input.set(b_src);

	Param<int> value;
	value.set(1);

	ImageParam input_lut(type_of<uint8_t>(), 1);
	Buffer<uint8_t> b_lut(lut, INT32_MAX);
	input_lut.set(b_lut);

	Logic::Init()
		.DemosaicMono(input)
		.DemosaicBitshift(value)
		.DemosaicLUT(input_lut)
		.DemosaicMonoSchedule()
		.speedTest(dst, 3, w, h);

	ImageParam input_hist(type_of<uint8_t>(), 3);
	Buffer<uint8_t> b_hist(dst, 3, w, h);
	input_hist.set(b_hist);
	
	int * dsthist = new int[w * h * 3];

	//Logic::Init(1)
	//	.DemosaicHistogram(input_hist)
	//	.speedTest(dsthist, 256, 0);
}

int main()
{
	logicTest();
	
	std::cout << "Hello World!\n";
}


