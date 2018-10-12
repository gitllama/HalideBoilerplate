#include "pch.h"
#include "Halide.h"
#include <stdio.h>

using namespace Halide;

/*
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
*/

/*
class Brighten : public Halide::Generator<Brighten> {
public:
	Input<Buffer<uint8_t>> input{ "input", 3 };

	// We will compile this generator in several ways to accept
	// several different memory layouts for the input and output. This
	// is a good use of a GeneratorParam (see lesson 15).
	enum class Layout { Planar, Interleaved, Either, Specialized };
	GeneratorParam<Layout> layout{ "layout",
		// default value
		Layout::Planar,
		// map from names to values
			{{ "planar",        Layout::Planar },
			 { "interleaved",   Layout::Interleaved },
			 { "either",        Layout::Either },
			 { "specialized",   Layout::Specialized }} };

	// We also declare a scalar input to control the amount of
	// brightening.
	Input<uint8_t> offset{ "offset" };

	// Declare our outputs
	Output<Buffer<uint8_t>> brighter{ "brighter", 3 };

	// Declare our Vars
	Var x, y, c;

	void generate() {
		// Define the Func.
		brighter(x, y, c) = input(x, y, c) + offset;
		brighter.vectorize(x, 16);

		if (layout == Layout::Planar) {

		}
		else if (layout == Layout::Interleaved) {

			input
				.dim(0).set_stride(3) // stride in dimension 0 (x) is three
				.dim(2).set_stride(1); // stride in dimension 2 (c) is one

			brighter
				.dim(0).set_stride(3)
				.dim(2).set_stride(1);

			input.dim(2).set_bounds(0, 3); // Dimension 2 (c) starts at 0 and has extent 3.
			brighter.dim(2).set_bounds(0, 3);
			brighter.reorder(c, x, y).unroll(c);
		}
		else if (layout == Layout::Either) {
			input.dim(0).set_stride(Expr()); // Use a default-constructed
											 // undefined Expr to mean
											 // there is no constraint.

			brighter.dim(0).set_stride(Expr());

		}
		else if (layout == Layout::Specialized) {

			input.dim(0).set_stride(Expr()); // Use an undefined Expr to
											 // mean there is no
											 // constraint.

			brighter.dim(0).set_stride(Expr());

			Expr input_is_planar =
				(input.dim(0).stride() == 1);
			Expr input_is_interleaved =
				(input.dim(0).stride() == 3 &&
					input.dim(2).stride() == 1 &&
					input.dim(2).extent() == 3);

			Expr output_is_planar =
				(brighter.dim(0).stride() == 1);
			Expr output_is_interleaved =
				(brighter.dim(0).stride() == 3 &&
					brighter.dim(2).stride() == 1 &&
					brighter.dim(2).extent() == 3);
			brighter.specialize(input_is_planar && output_is_planar);
			brighter.specialize(input_is_interleaved && output_is_interleaved)
				.reorder(c, x, y).unroll(c);
		}
	}
};
HALIDE_REGISTER_GENERATOR(Brighten, brighten)
*/