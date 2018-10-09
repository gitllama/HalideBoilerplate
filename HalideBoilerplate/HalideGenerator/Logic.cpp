#include "pch.h"
#include "Logic.h"

#include "Halide.h"
using namespace Halide;

Logic::Logic()
{
}

Logic::Logic(Func func)
{
	input = func;
}

Logic::~Logic()
{
}

Logic Logic::Init()
{
	Func output;
	Var x, y, c;

	output(c, x, y) = 0;

	return Logic(output);
}

Logic Logic::Init(ImageParam src)
{
	Func output;
	Var x, y, c;

	output(c, x, y, c) = src(c, x, y);

	return Logic(output);
}

Logic Logic::Init(int* src, int channels, int width, int height)
{
	ImageParam input(type_of<int>(), 3);
	Buffer<int> _src(src, channels, width, height);
	return Init(input);
}




//JITで使用する際（リアルタイムでのコンパイル

void Logic::Realize(int * dst, int channels, int width, int height)
{
	Buffer<int> _dst(dst, channels, width, height);
	input.realize(_dst);
}


//AOTで使用する際（事前コンパイルでDLLを生成

void Logic::Compile(std::string path, std::vector<Argument> arg, std::string name)
{
	input.compile_to_static_library(path, arg, name);
	printf("Halide pipeline compiled, %s\n", path);

	/*
	Target target;
	target.os = Target::Windows; // The operating system
	target.arch = Target::X86;
	target.bits = 64;            // The bit-width of the architecture
	std::vector<Target::Feature> x86_features;
	x86_features.push_back(Target::AVX);
	x86_features.push_back(Target::SSE41);
	target.set_features(x86_features);

	brighter.compile_to_static_library(path, arg, name, target);
	*/

}

//各Logic

Logic Logic::Offset(Param<int> value)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = input(c, x, y) + value;
	//brighter.vectorize(x, 16).parallel(y);

	return Logic(output);
}

Logic Logic::Demosaic(ImageParam src, Param<int> type)
{
	Func mono, color, rg, gr, bg, gb;
	Func output;
	Var c("c"), x("x"), y("y");

	Expr isMono = type == 0;
	Expr isRG = type == 1;
	Expr isGR = type == 2;
	Expr isGB = type == 3;
	Expr isBG = type == 4;

	Func plane = BoundaryConditions::repeat_edge(src);

	rg = _DemosaicColor(src, 0, 0);
	gr = _DemosaicColor(src, 1, 0);
	gb = _DemosaicColor(src, 0, 1);
	bg = _DemosaicColor(src, 1, 1);

	mono(c, x, y) = plane(0, x, y);
	color(c, x, y) = select(isRG, rg(c, x, y), 
						select(isGR, gr(c, x, y), 
							select(isGB, gb(c, x, y), bg(c, x, y))));


	output(c, x, y) = select(isMono, mono(c, x, y), color(c, x, y));


	output.reorder(x, y, c).unroll(x, 2).unroll(y, 2).vectorize(c, 3).parallel(y);

	//mono 8k,4k
	// まとめちゃうとあかん
	// default : 65
	// output.reorder(x, y, c).vectorize(c, 3) : 39
	// output.reorder(x, y, c).vectorize(c, 3).parallel(y) : 31

	return Logic(output);
}

Logic Logic::DemosaicMono(ImageParam src)
{
	Func mono;
	Func output;
	Var c("c"), x("x"), y("y");

	mono(x, y) = src(0, x, y);
	output(c, x, y) = mono(x, y);

	return Logic(output);
}

Logic Logic::DemosaicMonoSchedule()
{
	Var c("c"), x("x"), y("y");

	//mono 8kx4k
	// colorとまとめてしまうと、bayer差の分岐が外に出ていないので
	// 逆に遅くなる -> 方法は？

	// ロジックと分離しているが、nameが効いてるのでc,x,yは認識する
	// 別の名前付けるとエラー吐く

	// default 
	//  -> 63
	// input.reorder(x, y, c).vectorize(c, 3);
	//  -> 41
	// input.reorder(x, y, c).vectorize(c, 3).parallel(y);
	//  -> 33
	// input.reorder(c, x, y).parallel(y);
	//  -> 33

	// input.reorder(c, x, y).unroll(c, 3);
	//  -> 72
	// input.reorder(x, y, c).unroll(c, 3);
	//  -> 111

	// input.reorder(c, x, y).unroll(c, 3).parallel(y);
	//  -> 33

	//ただのメモリコピーなのでなかなかはやくならない

	input.reorder(c, x, y).parallel(y);
	return Logic(input);
}

Func Logic::_DemosaicColor(ImageParam src, int row, int col)
{
	Func plane = BoundaryConditions::repeat_edge(src);

	Func TB, X, LR, FIVE, ONE, TBLR, PLUS;
	Func R_R, R_Gr, R_B, R_Gb;
	Func G_R, G_Gr, G_B, G_Gb;
	Func B_R, B_Gr, B_B, B_Gb;
	Func layerB, layerG, layerR;
	Func color;
	Var c("c"), x("x"), y("y");

	Expr evenRow = (y % 2 == row);
	Expr evenCol = (x % 2 == col);

	ONE(x, y) = plane(0, x, y);
	LR(x, y) = saturating_cast<int>((plane(0, x - 1, y) + plane(0, x + 1, y)) / 2.0f);
	TB(x, y) = saturating_cast<int>((plane(0, x, y - 1) + plane(0, x, y + 1)) / 2.0f);
	X(x, y) = saturating_cast<int>((plane(0, x - 1, y - 1) + plane(0, x + 1, y - 1) + plane(0, x - 1, y + 1) + plane(0, x + 1, y + 1)) / 4.0f);
	PLUS(x, y) = saturating_cast<int>((plane(0, x - 1, y) + plane(0, x + 1, y) + plane(0, x, y - 1) + plane(0, x, y + 1)) / 4.0f);
	FIVE(x, y) = saturating_cast<int>((2.0f * plane(0, x, y) + plane(0, x - 1, y - 1) + plane(0, x + 1, y - 1) + plane(0, x - 1, y + 1) + plane(0, x + 1, y + 1)) / 8.0f);

	R_R(x, y) = ONE(x, y);
	R_Gr(x, y) = LR(x, y);
	R_B(x, y) = X(x, y);
	R_Gb(x, y) = TB(x, y);

	B_R(x, y) = X(x, y);
	B_Gr(x, y) = TB(x, y);
	B_B(x, y) = ONE(x, y);
	B_Gb(x, y) = LR(x, y);

	G_R(x, y) = PLUS(x, y);
	G_Gr(x, y) = FIVE(x, y);
	G_B(x, y) = PLUS(x, y);
	G_Gb(x, y) = FIVE(x, y);

	layerR(x, y) = select(evenRow,
		select(evenCol, R_R(x, y), R_Gr(x, y)),
		select(evenCol, R_Gb(x, y), R_B(x, y))
	);
	layerG(x, y) = select(evenRow,
		select(evenCol, G_R(x, y), G_Gr(x, y)),
		select(evenCol, G_Gb(x, y), G_B(x, y))
	);
	layerB(x, y) = select(evenRow,
		select(evenCol, B_R(x, y), B_Gr(x, y)),
		select(evenCol, B_Gb(x, y), B_B(x, y))
	);

	color(c, x, y) = select(c == 0, layerB(x, y), select(c == 1, layerG(x, y), layerR(x, y)));

	return color;
}

Logic Logic::DemosaicColor(ImageParam src, int row, int col)
{
	Func output;
	Func color = _DemosaicColor(src, row, col);
	Var c("c"), x("x"), y("y");

	output(c, x, y) = color(c,x,y);

	return Logic(output);
}

Logic Logic::DemosaicColorSchedule()
{
	Var c("c"), x("x"), y("y");

	//color 8kx4k

	// default 
	//  -> 280
	// input.reorder(c, x, y).unroll(c, 3).parallel(y);
	//  -> 85
	// input.reorder(c, x, y).parallel(y);
	// -> 78
	// input.reorder(c, x, y).unroll(c, 3).vectorize(x,3).parallel(y);
	//  -> 37

	input.reorder(c, x, y).vectorize(x, 3).parallel(y);
	//  -> 31


	return Logic(input);
}







//56
//何もなし

//70
//output.tile(x, y, xi, yi, 3, 3);


//output.parallel(c);
//output.unroll(c).parallel(c);
//output.reorder(c, x, y).unroll(c).parallel(c);

//68
//output.unroll(c, 3).vectorize(c, 3);;
//output.print_loop_nest();

//consumer.trace_stores();
//output.unroll(c).vectorize(c,3);


//15
//output.tile(x, y, xi, yi, 3, 3).unroll(yi).unroll(xi).parallel(y); //yiループを展開


/*
Logic Logic::Clamp(int * src, int left, int top) {
	Func output;
	Var x, y, c;
	
	Expr clamped_x = clamp(x, left, _width);
	Expr clamped_y = clamp(y, top, _height);
	Buffer<int> _src(src, _width, _height);

	output(c, x, y) = _src(_channels, clamped_x, clamped_y);

	return Logic(output, _width, _height, _channels);

	//Func input_16("input_16");
	//input_16(x, y, c) = cast<uint16_t>(clamped(x, y, c));
}

*/





//Logic Logic::Demosaic(Param<int> type)
//{
//	Func output;
//	Var x, y, c;
//
//	output(x, y, c) = min(input(x, y, c), 255);
//
//
//	return Logic(output);
//}

//Logic Logic::ToByte()
//{
//	Func clampmin, clampmax, output;
//	Var x, y, c;
//
//	clampmin(x, y, c) = min(input(x, y, c), 255);
//	clampmax(x, y, c) = max(clampmin(x, y, c), 0);
//	output(x, y, c) = cast<uint8_t>(clampmax(x, y, c));
//
//	return Logic(output);
//}





//HalideExtented HalideExtented::Stagger(int value)
//{
//	Func output;
//	Var x, y;
//	Expr Even = (y % 2) == 0;
//	
//	output(x, y) = select(Even, func(x, y), func(x + value, y));
//
//	HalideExtented dst;
//	dst._width = _width;
//	dst._height = _height;
//	dst.func = output;
//
//	return dst;
//}


//HalideExtented HalideExtented::HOB()
//{
//	Func output;
//	Var x, y;
//	RDom r_l(0, _width);
//	RDom r_r(0, _width);
//		
//	output(x, y) = func(x, y) - sum(func(r_l, y)) / _width;
//
//
//	HalideExtented dst;
//	dst._width = _width;
//	dst._height = _height;
//	dst.func = output;
//
//	return dst;
//}


//HalideExtented HalideExtented::DefectCorrection(int value)
//{
//	HalideExtented dst;
//	dst._width = _width;
//	dst._height = _height;
//	dst.func = func;
//
//	return dst;
//}


//HalideExtented HalideExtented::DemosaicMono()
//{
//	Func output;
//	Var x, y, c;
//
//	output(c, x, y) = func(x, y);
//
//	HalideExtented dst;
//	dst._width = _width;
//	dst._height = _height;
//	dst.func = output;
//
//	return dst;
//}


/*


Halide::Func HalideExtented::Conv3x3(Halide::Func src, int width, int height, int value)
{
	Halide::Func output;
	Halide::Var x, y;
	Halide::RDom r(-1,3,-1,3);

	Halide::Func clamped = Halide::BoundaryConditions::constant_exterior(src, 0);
	output(x, y) = Halide::sum(clamped(x+r.x, y+r.y));

	return output;
}
*/




//
//void halideInitTest5(Mat& src, Mat& dest)
//{
//	Var x, y, c;
//	Halide::Buffer<uint8_t> input(src.ptr<uchar>(0), src.cols, src.rows, src.channels());
//	Halide::Buffer<uint8_t> output(dest.ptr<uchar>(0), dest.cols, dest.rows, dest.channels());
//	Func adder;
//	adder(x, y, c) = cast<uint8_t>(min((cast<int16_t>(input(x, y, c)) + (short)50), 255));
//
//	adder.realize(output);
//}