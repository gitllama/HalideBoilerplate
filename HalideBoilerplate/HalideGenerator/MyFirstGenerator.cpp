#include "pch.h"
#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class MyFirstGenerator : public Halide::Generator<MyFirstGenerator> {
public:
	Input<uint8_t> offset{ "offset" };
	Input<Buffer<uint8_t>> input{ "input", 2 };
	Output<Buffer<uint8_t>> brighter{ "brighter", 2 };

	Var x, y;
	void generate() {
		brighter(x, y) = input(x, y) + offset;
		brighter.vectorize(x, 16).parallel(y);
	}
};

HALIDE_REGISTER_GENERATOR(MyFirstGenerator, my_first_generator)