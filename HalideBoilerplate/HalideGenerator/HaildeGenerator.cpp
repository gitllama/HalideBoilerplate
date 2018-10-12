// HaildeGenerator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>
//#include <Windows.h>

//#include "MyFirstGenerator.cpp"
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

	Logic test = Logic::init()
		.DemosaicColor(input, 0, 0)
		.DemosaicColorSchedule();

	//Util::speedTest(test, _dst, w, h);
	//test.input.compile_to_c("c_code.cpp", { input, type }, "user_func");

	//test.Realize(_dst, 3, w, h);
	//Util::printArray(_src, _dst, w, h);
}

void generateDDL(std::string path) {

	ImageParam input(type_of<uint32_t>(), 1, "raw");
	ImageParam raw(type_of<int>(), 2, "input_raw");
	ImageParam matrix(type_of<float>(), 2, "matrix");
	ImageParam LUT(type_of<uint8_t>(), 1, "LUT");

	Logic::init()
		.compileWithRuntime(path, "zeroFill", {});

	Logic::init()
		.sort(input)
		.sortSchedule()
		.compile(path, "sort", { input });

	Logic::init()
		.DemosaicColor(raw, 0, 0)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicRG", { raw, matrix });

	Logic::init()
		.DemosaicColor(raw, 1, 0)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGR", { raw, matrix });

	Logic::init()
		.DemosaicColor(raw, 0, 1)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGB", { raw, matrix });

	Logic::init()
		.DemosaicColor(raw, 1, 1)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicBG", { raw, matrix });

	Logic::init()
		.DemosaicMono(raw)
		.Convert_XYC2CXY()
		.DemosaicMonoSchedule()
		.compile(path, "demosaicMONO", { raw });

	Logic::init()
		.DemosaicColor(raw, 0, 0)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicRGwLUT", { raw, matrix, LUT });
	Logic::init()
		.DemosaicColor(raw, 1, 0)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGRwLUT", { raw, matrix, LUT });
	Logic::init()
		.DemosaicColor(raw, 0, 1)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGBwLUT", { raw, matrix, LUT });
	Logic::init()
		.DemosaicColor(raw, 1, 1)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicBGwLUT", { raw, matrix, LUT });
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

	Logic::init()
		.DemosaicMono(input)
		.DemosaicBitshift(value)
		.DemosaicLUT(input_lut)
		.DemosaicMonoSchedule()
		.speedTest(dst, 3, w, h);

	ImageParam input_hist(type_of<uint8_t>(), 3);
	Buffer<uint8_t> b_hist(dst, 3, w, h);
	input_hist.set(b_hist);

	printf("%d\r\n", b_hist.width()); //解決するタイミング・・・
	printf("%d\r\n", b_hist.height());

	int * dsthist = new int[w * h * 3];

	Logic::init(1)
		.DemosaicHistogram(input_hist)
		.speedTest(dsthist, 256);
}

int main()
{
	int w = 8;
	int h = 8;

	int * _src = new int[w * h];
	int * _dst = new int[w*h];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			_src[x + y * w] = x + y * 10;
		}
	}

	ImageParam input(type_of<int>(), 2);
	Buffer<int> _input(_src, w, h);
	input.set(_input);

	Buffer<int> dst(_dst, w, h);
	Param<int> value;
	value.set(30);

	RDom r(0, input.width(), 0, input.height());
	Expr ave = sum(input(r.x, r.y)) / (input.width() * input.height());

	//r.where(input(r.x, r.y) > value);

	//Expr count = sum(select(input(r.x, r.y) > value, 0, 1));
	//Expr add = sum(select(input(r.x, r.y) > value, 0, input(r.x, r.y))) / count;

	Expr count = sum(input(r.x, r.y));
	Expr add = sum(input(r.x, r.y));
	Expr e = sum(select(input(r.x, r.y) > 0, 0, 1)); //数値の比較になってない

	//value.set(ave);

	Func output;
	Var x, y;
	//output(x, y) = select(input(x, y) > value, input(x, y) - add, input(x, y));
	output(x, y) = e;
	//output(r.x, r.y) = input(r.x, r.y);
	output.realize(dst);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			printf("%d ", dst(x,y));
		}
		printf("\r\n");
	}
	//printf("%d", value.get());

	//logicTest();
	//generateDDL("../x64/lib/");

	std::cout << "Hello World!\n";
}


