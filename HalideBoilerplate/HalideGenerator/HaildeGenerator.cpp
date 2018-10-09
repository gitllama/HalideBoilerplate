// HaildeGenerator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>

#include "Util.h"
#include "Logic.h"
#include "Halide.h"
using namespace Halide;

int main()
{
	ImageParam input(type_of<int>(), 3);


	Logic::Init()
		.DemosaicColor(input, 0, 0)
		.DemosaicColorSchedule()
		.Compile("RG", { input }, "_RG");

	//Logic::Init()
	//	.DemosaicColor(input, 1, 0)
	//	.DemosaicColorSchedule()
	//	.Compile("GR", { input }, "_GR");

	//Logic::Init()
	//	.DemosaicColor(input, 0, 1)
	//	.DemosaicColorSchedule()
	//	.Compile("GB", { input }, "_GB");

	//Logic::Init()
	//	.DemosaicColor(input, 1, 1)
	//	.DemosaicColorSchedule()
	//	.Compile("BG", { input }, "_BG");

	//Logic::Init()
	//	.DemosaicMono(input)
	//	.DemosaicMonoSchedule()
	//	.Compile("MONO", { input }, "_MONO");


	std::cout << "Hello World!\n";
}


int example_1() {

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

	Logic::Init(input)
		.Offset(offset)
		.Realize(_dst, c, w, h);

	for (int i = 0; i < 4 * 4; i++) {
		printf("%d\r\n", _dst[i]);
	}
}

int example_2() {

	//ImageParam kernel{ Int(16), 2, "kernel" };
	//Param<int32_t> kernel_size{ "kernel_size", 3, 1, 5 };

	int w = 2267;
	int h = 1178;

	int * _src = new int[w * h];
	int * _dst = new int[w * h * 3];

	Buffer<int> src(_src, w, h);
	Buffer<int> dst(_dst, 3, w, h);

	Func plane = BoundaryConditions::repeat_edge(src);

	Var c("c"), x("x"), y("y");
	Func func_1("func_1"), func_2("func_2"), func_3("func_2");
	Func output("output");

	func_1(x, y) = plane(x, y);
	func_2(x, y) = plane(x - 1, y) + plane(x, y) + plane(x + 1, y);
	func_3(x, y) = plane(x, y);

	output(c, x, y) = select(c == 0, func_1(x, y), select(c == 1, func_2(x, y), func_3(x, y)));


	//19 : なにもなし

	//24
	// output.vectorize(c,3);

	//7
	//output.reorder(x, y, c);
	//output.vectorize(c,3);

	//5
	//output.reorder(x, y, c).vectorize(c, 3).parallel(y);

	//7
	//Var xi, yi;
	//output.tile(x, y, xi, yi, 4, 4).vectorize(c, 3).parallel(y);

	//Var fused("fuse");
	//output.fuse(x, y, fused);


	output.print_loop_nest();
	std::chrono::system_clock::time_point  start, end;
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << ":";
		start = std::chrono::system_clock::now();

		output.realize(dst);

		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		printf("%f\r\n", elapsed);
	}
}

int example_3() {

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

	Util::speedTest(test, _dst, w, h);
	test.input.compile_to_c("c_code.cpp", { input, type }, "user_func");

	//test.Realize(_dst, 3, w, h);
	//Util::printArray(_src, _dst, w, h);
}


// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
