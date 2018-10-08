// HaildeGenerator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>

#include "Logic.h"
#include "Halide.h"
using namespace Halide;

int main()
{

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
	func_2(x, y) = plane(x-1, y) + plane(x, y) + plane(x+1, y);
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

/*
int output() {

#ifdef _DEBUG

	int w = 2267;
	int h = 1178;

	int * src = new int[w * h];
	int * buf = new int[w * h];
	int * dst2 = new int[w * h * 3];
	unsigned char * dst = new unsigned char[w * h * 3];

	//for (int y = 0; y < h; y++) {
	//	for (int x = 0; x < w; x++) {
	//		if (x % 2 == 0 && y % 2 == 0) {
	//			src[x + y * w] = 1;
	//		}
	//		else if (x % 2 == 1 && y % 2 == 1) {
	//			src[x + y * w] = 3;
	//		}
	//		else {
	//			src[x + y * w] = 2;
	//		}
	//	}
	//}

	Logic::Init(src, w, h, 1)
		//.Offset(100)
		.Finalize(buf);

	//for (int y = 0; y < h; y++) {
	//	for (int x = 0; x < w; x++) {
	//		printf("%d ", buf[x + y * w]);
	//	}
	//	printf("\r\n");
	//}

	//Logic::Init(w, h, 3)
	// 	.DemosaicMono(buf)
	//	.Finalize(dst2);

	//Logic::Init(w, h, 3)
	//	.Demosaic(buf, 0, 0)
	//	.Finalize(dst2);

	//for (int c = 0; c < 3; c++) {
	//	for (int y = 0; y < h; y++) {
	//		for (int x = 0; x < w; x++) {
	//			printf("%d ", dst2[c + (x + y*w)*3]);
	//		}
	//		printf("\r\n");
	//	}
	//	printf("\r\n");
	//}

	std::chrono::system_clock::time_point  start, end;
	Logic test = Logic::Init(w, h, 3).Demosaic(buf, 0, 0);
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << ":";
		start = std::chrono::system_clock::now();
		test.Finalize(dst2);
		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		printf("%f\r\n", elapsed);
	}

#else

	// 生成するロジックを記述します

	ImageParam input(type_of<int>(), 3);
	Param<int> offset;

	Logic::Init(input)
		.Offset(offset)
		.Compile("test", { input, offset }, "test");

	Logic::Init(input)
		.ToByte()
		.Compile("Demosaic", { input }, "Demosaic");

	std::cout << "Hello World!\n";

#endif

}

*/

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
