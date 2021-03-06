// HalideGenerator.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include "pch.h"
#include <iostream>

#include "Logic.h"
#include "Halide.h"
using namespace Halide;


void generateDDL(std::string path) {

	ImageParam input(type_of<uint32_t>(), 1, "raw");
	ImageParam raw(type_of<int>(), 2, "input_raw");
	ImageParam matrix(type_of<float>(), 2, "matrix");
	ImageParam LUT(type_of<uint8_t>(), 1, "LUT");

	Logic::init(0, 2)
		.compileWithRuntime(path, "zeroFill", {});

	Logic::init(0, 2)
		.sort(input)
		.sortSchedule()
		.compile(path, "sort", { input });

	Logic::init(0, 2)
		.DemosaicColor(raw, 0, 0)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicRG", { raw, matrix });

	Logic::init(0, 2)
		.DemosaicColor(raw, 1, 0)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGR", { raw, matrix });

	Logic::init(0, 2)
		.DemosaicColor(raw, 0, 1)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGB", { raw, matrix });

	Logic::init(0, 2)
		.DemosaicColor(raw, 1, 1)
		.Transform(matrix)
		.toInt()
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicBG", { raw, matrix });

	Logic::init(0, 2)
		.DemosaicMono(raw)
		.Convert_XYC2CXY()
		.DemosaicMonoSchedule()
		.compile(path, "demosaicMONO", { raw });

	Logic::init(0, 2)
		.DemosaicColor(raw, 0, 0)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicRGwLUT", { raw, matrix, LUT });
	Logic::init(0, 2)
		.DemosaicColor(raw, 1, 0)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGRwLUT", { raw, matrix, LUT });
	Logic::init(0, 2)
		.DemosaicColor(raw, 0, 1)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicGBwLUT", { raw, matrix, LUT });
	Logic::init(0, 2)
		.DemosaicColor(raw, 1, 1)
		.Transform(matrix)
		.LUT2Byte(LUT)
		.Convert_XYC2CXY()
		.DemosaicColorSchedule()
		.compile(path, "demosaicBGwLUT", { raw, matrix, LUT });
}

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << "\n";
	}
	if (argc > 1) {
		if (std::string(argv[1]) == "-g") {

		generateDDL(std::string(argv[2]));
		std::cout << "generated\n";
		}
	}
	
    std::cout << "Hello World!\n";

}
