#include "pch.h"
#include "Logic.h"

#include "Halide.h"
using namespace Halide;


Logic::Logic()
{
}

Logic::Logic(Func func, int width, int height, int channels)
{
	input = func;
	_width = width;
	_height = height;
	_channels = channels;
}

Logic::~Logic()
{
}


//JITで使用する際（リアルタイムでのコンパイル

Logic Logic::Init(int width, int height, int channels)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = 0;

	return Logic(output, width, height, channels);
	/*
	
	メソッドチェーンは
	ClassName& Name(){ return *this; }
	で記載できるが、Funcを確保する為毎度インスタンス生成

	*/
}

Logic Logic::Init(int* src, int width, int height, int channels)
{
	Func output;
	Var x, y, c;

	Buffer<int> _src(src, channels, width, height);
	output(c, x, y) = _src(c, x, y);

	return Logic(output, width, height, channels);
}


Logic Logic::Offset(Param<int> value)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = input(c, x, y) + value;

	return Logic(output, _width, _height, _channels);
}


void Logic::Finalize()
{
	input.realize(_channels, _width, _height);
}

void Logic::Finalize(int * dst)
{
	Buffer<int> _dst(dst, _channels, _width, _height);
	input.realize(_dst);

	/*
	
	直接用意された配列に放り込んだ方がメモリマネジメント的に
	何かと便利なのであまり用途はないが

	Buffer<unsigned char> output = input.realize(width, height, channels);

	とすることで下記のように各要素にアクセス可能。

	横サイズ : output.width();
	縦サイズ : output.height();
	チャネル数 : output.channels();
    要素 : output(x,y,c);
　　先頭ポインタ : output.begin(); / output2.data();
    最後尾ポインタ : output.end();
	
	*/
}

void Logic::Finalize(uint8_t * dst)
{
	Buffer<uint8_t> _dst(dst, _channels, _width, _height);
	input.realize(_dst);
}

Logic Logic::DemosaicMono(int * src)
{
	Func output;
	Var x, y, c;

	Buffer<int> _src(src, _width, _height);
	output(c, x, y) = _src(x, y);

	return Logic(output, _width, _height, _channels);
}

//0-0 RG
//1-0 GR
//0-1 GB
//1-1 BG
Logic Logic::Demosaic(int * src, int a,int b)
{
	Func output("demosaic");
	Var x("x"), y("y"), c("c");

	Expr evenRow = (y % 2 == a);
	//Expr oddRow = (y % 2 == 1);
	Expr evenCol = (x % 2 == b);
	//Expr oddCol = (x % 2 == 1);

	Buffer<int> _src(src, _width, _height);
	Func plane = BoundaryConditions::repeat_edge(_src);

	Func TB, X, LR, FIVE, ONE, TBLR, PLUS;
	
	ONE(x, y) = plane(x, y);
	LR(x, y) = (plane(x - 1, y) + plane(x + 1, y)) / 2;
	TB(x, y) = (plane(x, y - 1) + plane(x, y + 1)) / 2;
	X(x, y) = (plane(x - 1, y - 1) + plane(x + 1, y - 1) + plane(x - 1, y + 1) + plane(x + 1, y + 1)) / 4;
	PLUS(x, y) = (plane(x - 1, y) + plane(x + 1, y) + plane(x, y - 1) + plane(x, y + 1)) / 4;
	FIVE(x, y) = (plane(x, y) + (plane(x - 1, y - 1) + plane(x + 1, y - 1) + plane(x - 1, y + 1) + plane(x + 1, y + 1)) / 4) / 2;

	Func R_R, R_Gr, R_B, R_Gb;
	Func G_R, G_Gr, G_B, G_Gb;
	Func B_R, B_Gr, B_B, B_Gb;

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



	Func layerR, layerG, layerB;

	layerR(x, y) = select(evenRow,
		select(evenCol, R_R(x, y) , R_Gr(x, y)),
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

	output(c, x, y) = select(c == 0, layerB(x, y), select(c == 1, layerG(x, y), layerR(x, y)));
	
	Var xi, yi;
	
	//56
	//何もなし

	//70
	//output.tile(x, y, xi, yi, 3, 3);

	//3600
	//output.parallel(c);

	//3525
	//output.unroll(c).parallel(c);
	
	//3357
	//output.reorder(c, x, y).unroll(c).parallel(c);

	//68
	output.unroll(c,3).vectorize(c, 3);;
	output.print_loop_nest();

	//consumer.trace_stores();
	//output.unroll(c).vectorize(c,3);


	//15
	//output.tile(x, y, xi, yi, 3, 3).unroll(yi).unroll(xi).parallel(y); //yiループを展開



	return Logic(output, _width, _height, _channels);
}

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


//AOTで使用する際（事前コンパイルでDLLを生成

//Logic Logic::Init(ImageParam src)
//{
//	Func output;
//	Var x, y, c;
//
//	output(x, y, c) = src(x, y, c);
//
//	return Logic(output);
//}

//Logic Logic::Offset(Param<int> value)
//{
//	Func output;
//	Var x, y, c;
//
//	output(x, y, c) = input(x, y, c) + value;
//	//brighter.vectorize(x, 16).parallel(y);
//
//	return Logic(output);
//}

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

//void Logic::Compile(std::string path, std::vector<Argument> arg, std::string name)
//{
//	input.compile_to_static_library(path, arg, name);
//	printf("Halide pipeline compiled, %s\n", path);
//
//	/*
//	Target target;
//	target.os = Target::Windows; // The operating system
//	target.arch = Target::X86;
//	target.bits = 64;            // The bit-width of the architecture
//	std::vector<Target::Feature> x86_features;
//	x86_features.push_back(Target::AVX);
//	x86_features.push_back(Target::SSE41);
//	target.set_features(x86_features);
//
//	brighter.compile_to_static_library(path, arg, name, target);
//	*/
//
//}
//

//Logic Logic::Init(int * src, int width, int height)
//{
//	Buffer<int> buf(src, width, height);
//	Func output;
//	Var x, y;
//
//	Func a = BoundaryConditions::repeat_edge(buf);
//
//	output(x, y) = a(x, y);
//
//	HalideExtented dst;
//	dst._width = width;
//	dst._height = height;
//	dst.func = output;
//
//	return dst;
//}
//





//HalideExtented HalideExtented::Offset(int value)
//{
//	Func output;
//	Var x, y;
//
//	output(x, y) = func(x,y) + value;
//
//	HalideExtented dst;
//	dst._width = _width;
//	dst._height = _height;
//	dst.func = output;
//
//	return dst;
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