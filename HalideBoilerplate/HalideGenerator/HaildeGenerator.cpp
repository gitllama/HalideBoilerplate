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

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
