#pragma once
#include "Halide.h"

using namespace Halide;

// Halide tutorial lesson 15: Generators 

class DemosaicClass : public Halide::Generator<DemosaicClass>
{
public:
	Input<uint8_t> offset{ "offset" };
	Input<Buffer<uint8_t>> input{ "input", 2 };
	Output<Buffer<uint8_t>> brighter{ "brighter", 2 };
	Var x, y;

	void generate() {
		brighter(x, y) = input(x, y) + offset;
		brighter.vectorize(x, 16).parallel(y);
	}
	void schedule() {

	}
};

//ジェネレーターの登録
HALIDE_REGISTER_GENERATOR(DemosaicClass, demosaic_class)

//コンパイルオプションで