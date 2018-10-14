#include "pch.h"
#include "Logic.h"

#include "Halide.h"
using namespace Halide;


// Varはx, y ,cの順にしておいた方が良い
// width()とかと食い違いが発生する
// bitmapは出力入力時の次元の変換で対応

#pragma region Constructor / Destructor

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

#pragma endregion

Logic Logic::load(ImageParam src)
{
	Func output;
	Var x("x"), y("y"), c("c"), i("i");
	output = src;

	//switch (src.dimensions())
	//{
	//case 1:
	//	output(x, y) = src(x, y);
	//	break;
	//case 3:
	//	output(x, y, c) = src(x, y, c);
	//	break;
	//default:
	//	output(x, y) = src(x, y);
	//	break;
	//}

	return Logic(output);
}


#pragma region Util

void Logic::printArray(int* _src, int* _dst, int w, int h) {
	printf("input : \r\n");
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			printf("%d ", _src[(x + y * w)]);
		}
		printf("\r\n");
	}
	printf("\r\n");
	for (int ch = 0; ch < 3; ch++) {
		printf("Ch %d : \r\n", ch);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				printf("%d ", _dst[ch + (x + y * w) * 3]);
			}
			printf("\r\n");
		}
		printf("\r\n");
	}
}

#pragma endregion


#pragma region Realize / Compile

//JITで使用する際（リアルタイムでのコンパイル

//ヘッダ側参照

//AOTで使用する際（事前コンパイルでDLLを生成

void Logic::compileWithRuntime(std::string path, std::string name, std::vector<Argument> arg)
{
	Target target = get_target_from_environment();
	_complile(input, path, name, arg, target);
}

void Logic::compile(std::string path, std::string name, std::vector<Argument> arg)
{
	Target target = get_target_from_environment();
	/*
	target.os = Target::Windows; // The operating system
	target.arch = Target::X86;
	target.bits = 64;            // The bit-width of the architecture
	std::vector<Target::Feature> x86_features;
	x86_features.push_back(Target::AVX);
	x86_features.push_back(Target::SSE41);
	target.set_features(x86_features);
	*/
	target.set_feature(Target::NoRuntime, true);

	_complile(input, path, name, arg, target);
}

void Logic::_complile(Func input, std::string path, std::string name, std::vector<Argument> arg, Target target)
{
	std::string fullpath = path + "HalideGenerated_" + name;
	input.compile_to_static_library(fullpath, arg, name, target);
	//input.compile_to_file(path, arg, name);
	printf("Halide pipeline compiled, %s\n", name);
}

#pragma endregion


//各Logic

#pragma region PreProcess

// データの帯域圧縮のため 24bitx4 -> 32bitx3 
Logic Logic::sort(ImageParam src)
{
	// AA_AA_AA_BB BB_BB_CC_CC CC_DD_DD_DD
	int width = 1178;

	Func output("Sort");
	Var x("x"), y("y"), i("i");

	Expr row = y % 2 == 0;
	Expr col = x % 2 == 0;

	Expr index = (cast<int>(x / 2) + cast<int>(y / 2) * cast<int>(width / 2)) * 3;
	//Expr index = (cast<int>(x / 2) + cast<int>(y / 2) * cast<int>(width / 2)) * 3;

	Expr _data0 = cast<uint32_t>(src(index));
	Expr _data1 = cast<uint32_t>(src(index + 1));
	Expr _data2 = cast<uint32_t>(src(index + 2));

	Expr data0 = _data0 & Expr((uint32_t)0xFFFFFF00);
	Expr data1 = ((_data0 & Expr((uint32_t)0x000000FF)) << 24) | ((_data1 & Expr((uint32_t)0xFFFF0000)) >> 8);
	Expr data2 = ((_data1 & Expr((uint32_t)0x0000FFFF)) << 16) | ((_data2 & Expr((uint32_t)0xFF000000)) >> 16);
	Expr data3 = _data2 << 8;

	data0 = cast<int32_t>(data0 >> 8);
	data1 = cast<int32_t>(data1 >> 8);
	data2 = cast<int32_t>(data2 >> 8);
	data3 = cast<int32_t>(data3 >> 8);

	output(x, y) = select(row,
		select(col, data0, data1),
		select(col, data2, data3));

	return Logic(output);
}

Logic Logic::HNR()
{
	//	RDom r_l(0, _width);
//	RDom r_r(0, _width);
//		
//	output(x, y) = func(x, y) - sum(func(r_l, y)) / _width;
	return Logic(input);
}

Logic Logic::PreProcessSchedule()
{
	return Logic(input);
}

Logic Logic::sortSchedule()
{
	Var x("x"), y("y"), xi("xi"), yi("yi"), fused("fused");

	/*
	int w = 22560;
	int h = 11780;

	uint32_t * _src = new uint32_t[w * h * 3 / 4];
	ImageParam input(type_of<uint32_t>(), 1);
	Buffer<uint32_t> src(_src, w * h * 3 / 4);
	input.set(src);

	int * _dst = new int[w * h];

	Logic test = Logic::Init()
		.Sort(input)
		.SortSchedule();

	Util::speedTest(test, _dst, w, h);
	*/
	
	// [ms] : schedule
	
	// 670  : default 
	// 452  : input.tile(x,y,xi,yi,2,2).unroll(xi).unroll(yi);
	// 437  : input.tile(x,y,xi,yi,2,2).fuse(xi,yi,fused).unroll(fused);
	// 700  : input.tile(x,y,xi,yi,2,2).reorder(xi,yi,x,y).fuse(xi,yi,fused).unroll(fused).vectorize(fused,4);
	// 118  : input.tile(x,y,xi,yi,2,2).fuse(xi,yi,fused).unroll(fused).parallel(y);
	
	// 210  : input.tile(x,y,xi,yi,4,2).unroll(xi).vectorize(xi, 4).unroll(yi);
	// 77   : input.tile(x, y, xi, yi, 4, 2).unroll(xi).vectorize(xi, 4).unroll(yi).parallel(y);

	// ※ C# -> 150, parallel -> 91


	input.tile(x, y, xi, yi, 4, 2).unroll(xi).vectorize(xi, 4).unroll(yi).parallel(y);
	//input.print_loop_nest();

	return Logic(input);
}

#pragma endregion


#pragma region BeforeDemosaic

Logic Logic::Stagger(Param<int> value)
{
	Func output("Stagger");
	Var x, y;

	Expr row = y % 2 == 0;
	Expr col = x < 2 || 13 < x;

	output(x, y) = select(row, input(x, y),
		col, input(x, y), input(x + value, y));

	return Logic(output);

	//Buffer<int> shifted(5, 7);
	//shifted.set_min(100, 50);
	
	//output(x, y) = select(Even, func(x, y), func(x + value, y));
}

Logic Logic::Dark(ImageParam value)
{
	Func output("Dark");
	Var x, y;
	output(x, y) = input(x, y); // - value(x, y);
	return Logic(output);
}

Logic Logic::Offset(Param<int> value)
{
	Func output("Offset");
	Var x, y;

	output(x, y) = input(x, y) + value;

	return Logic(output);
}

Logic Logic::Bitshift(Param<int> value)
{
	Func output("Bitshift");
	Var x, y;

	output(x, y) = input(x, y) >> value;

	return Logic(output);
}

Logic Logic::BeforeDemosaicProcessSchedule()
{
	return Logic(input);
}

#pragma endregion


#pragma region Demosaic



Logic Logic::DemosaicMono(ImageParam src)
{
	Func mono;
	Func output;
	Var x("x"), y("y"), c("c");

	mono(x, y) = src(x, y);
	output(x, y, c) = mono(x, y);

	return Logic(output);
}

Logic Logic::DemosaicBitshift(Param<int> value)
{
	Func output;
	Var x("x"), y("y"), c("c");

	output(x, y, c) = input(x, y, c) >> value;

	return Logic(output);
}

Logic Logic::DemosaicLUT(ImageParam lut)
{
	Func output;
	Var x("x"), y("y"), c("c");

	Expr clamped = clamp(input(x, y, c), 0, INT32_MAX - 1);
	output(x, y, c) = lut(clamped);

	return Logic(output);
}

Logic Logic::DemosaicHistogram(ImageParam src)
{
	Func output("histogram");
	Var x("x"), y("y"), c("c"), i("i");

	RDom r(0, src.width(), 0, src.height());

	//output(x) = 0;
	output(src(r.x, r.y, 0)) += 1;
	//Buffer<int> halide_result = histogram.realize(256);

	return Logic(input);
}
//Func histogram("histogram");


Logic Logic::DemosaicMonoSchedule()
{
	Var x("x"), y("y"), c("c");

	//mono 8kx4k
	// colorとまとめてしまうと、bayer差の分岐が外に出ていないので
	// 逆に遅くなる -> 方法は？

	// ロジックと分離しているが、nameが効いてるのでc,x,yは認識する
	// 別の名前付けるとエラー吐く

	// 46  : default 
	// 41  : input.reorder(x, y, c).vectorize(c, 3);
	// 33  : input.reorder(x, y, c).vectorize(c, 3).parallel(y);
	// 33  : input.reorder(c, x, y).parallel(y);

	// 72  : input.reorder(c, x, y).unroll(c, 3);
	// 111 : input.reorder(x, y, c).unroll(c, 3);
	// 33  : input.reorder(c, x, y).unroll(c, 3).parallel(y);
	// 31  : input.tile(x, c, xi, ci, 4, 3).fuse(xi, ci, fused).unroll(fused).parallel(y);
	//ただのメモリコピーなのでなかなかはやくならない

	Var xi, xo, yi, ci,fused;
	input.reorder(c, x, y).unroll(c, 3).parallel(y);
	input.print_loop_nest();

	return Logic(input);
}


Logic Logic::DemosaicColor(ImageParam src, int row, int col)
{
	Func output;
	Var x("x"), y("y"), c("c");
	Expr evenRow = (y % 2 == row);
	Expr evenCol = (x % 2 == col);

	Func planeInt = BoundaryConditions::repeat_edge(src);
	Func plane;
	plane(x, y) = cast<float>(planeInt(x, y));

	Expr R_R, R_Gr, R_B, R_Gb;
	Expr G_R, G_Gr, G_B, G_Gb;
	Expr B_R, B_Gr, B_B, B_Gb;
	Func layerB, layerG, layerR;

	R_R = bayerInterpolation(plane, 0)(x, y);
	R_Gr = bayerInterpolation(plane, 1)(x, y);
	R_B = bayerInterpolation(plane, 3)(x, y);
	R_Gb = bayerInterpolation(plane, 2)(x, y);

	B_R = bayerInterpolation(plane, 3)(x, y);
	B_Gr = bayerInterpolation(plane, 2)(x, y);
	B_B = bayerInterpolation(plane, 0)(x, y);
	B_Gb = bayerInterpolation(plane, 1)(x, y);

	G_R = bayerInterpolation(plane, 4)(x, y);
	G_Gr = bayerInterpolation(plane, 0)(x, y);
	G_B = bayerInterpolation(plane, 4)(x, y);
	G_Gb = bayerInterpolation(plane, 0)(x, y);

	layerR(x, y) = select(evenRow,
		select(evenCol, R_R, R_Gr),
		select(evenCol, R_Gb, R_B)
	);
	layerG(x, y) = select(evenRow,
		select(evenCol, G_R, G_Gr),
		select(evenCol, G_Gb, G_B)
	);
	layerB(x, y) = select(evenRow,
		select(evenCol, B_R, B_Gr),
		select(evenCol, B_Gb, B_B)
	);

	output(x, y, c) = select(c == 0, layerB(x, y), c == 1, layerG(x, y), layerR(x, y));
	return Logic(output);
}

Logic Logic::Transform(ImageParam matrix)
{
	Func output;
	Var x("x"), y("y"), c("c");

	Expr B = matrix(0, 0) * input(x, y, 0) + matrix(1, 0) * input(x, y, 1) + matrix(2, 0) * input(x, y, 2);
	Expr G = matrix(0, 1) * input(x, y, 0) + matrix(1, 1) * input(x, y, 1) + matrix(2, 1) * input(x, y, 2);
	Expr R = matrix(0, 2) * input(x, y, 0) + matrix(1, 2) * input(x, y, 1) + matrix(2, 2) * input(x, y, 2);

	output(x, y, c) = select(c == 0, B, c==1, G, R);

	return Logic(output);
}

Logic Logic::toInt()
{
	Func output;
	Var x("x"), y("y"), c("c");

	output(x, y, c) = saturating_cast<int>(input(x, y, c));

	return Logic(output);
}

Logic Logic::LUT2Byte(ImageParam lut)
{
	Func clamped, input_Y, output;
	Var x("x"), y("y"), c("c");

	clamped(x, y, c) = clamp(input(x, y, c), 0.0f, saturating_cast<float>(INT32_MAX));

	input_Y(x, y) = saturating_cast<int>(0.114f * clamped(x, y, 0) + 0.587f * clamped(x, y, 1) + 0.299f * clamped(x, y, 2));
	Expr e = lut(clamp(input_Y(x, y), 0, lut.width() - 1));

	output(x, y, c) = saturating_cast<uint8_t>(input(x, y, c) * e / input_Y(x, y));

	return Logic(output);
}

Logic Logic::LUT2Byte2(ImageParam lut)
{
	Func clamped, input_Y, input_Y2, output;
	Func a;
	Var x("x"), y("y"), c("c");
	RDom r(-2, 5, -2, 5);

	clamped(x, y, c) = clamp(input(x, y, c), 0.0f, saturating_cast<float>(INT32_MAX));
	input_Y(x, y) = (0.114f * clamped(x, y, 0) + 0.587f * clamped(x, y, 1) + 0.299f * clamped(x, y, 2));
	
	input_Y2(x, y) = saturating_cast<int>(sum(input_Y(x + r.x, y + r.y))/25.0f);

	Expr e = lut(clamp(input_Y2(x, y), 0, lut.width() - 1));

	output(x, y, c) = saturating_cast<uint8_t>(input(x, y, c) * e / input_Y2(x, y));

	return Logic(output);
}

Func Logic::bayerInterpolation(Func input, int type)
{
	Func output;
	Var x("x"), y("y"), c("c");
	switch (type) 
	{
	case 1: // L-R
		output(x, y) = (input(x - 1, y) + input(x + 1, y)) / 2.0f;
		break;
	case 2: // T-B
		output(x, y) = (input(x, y - 1) + input(x, y + 1)) / 2.0f;
		break;
	case 3: // LT-RT-LB-RB
		output(x, y) = ( input(x - 1, y - 1) + input(x + 1, y - 1) 
					   + input(x - 1, y + 1) + input(x + 1, y + 1) ) / 4.0f;
		break;
	case 4: // T-B-L-R
		output(x, y) = ( input(x - 1, y) + input(x + 1, y)
			           + input(x, y - 1) + input(x, y + 1) ) / 4.0f;
		break;
	case 5: // LT-RT-LB-RB-C
		output(x, y) = ( 4.0f * input(x , y) 
			           + input(x - 1, y - 1) + input(x + 1, y - 1)
			           + input(x - 1, y + 1) + input(x + 1, y + 1) ) / 8.0f;
		break;
	default: // C
		output(x, y) = input(x, y);
		break;

	}
	return output;
}



Logic Logic::DemosaicColorSchedule()
{
	Var x("x"), y("y"), c("c"), xi, yi,ci, fused;

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

	//input.tile(x,y,xi,yi,8,4).reorder(c, xi, yi, x, y).vectorize(xi).parallel(y, 4);
	return Logic(input);
}


Logic Logic::Convert_XYC2CXY()
{
	Func output;
	Var x("x"), y("y"), c("c");

	output(c, x, y) = input(x, y, c);

	return Logic(output);
}

#pragma endregion




Func Logic::_DemosaicColor(ImageParam src, int row, int col)
{
	//未修正
	Func plane = BoundaryConditions::repeat_edge(src);

	Func TB, X, LR, FIVE, ONE, TBLR, PLUS;
	Func R_R, R_Gr, R_B, R_Gb;
	Func G_R, G_Gr, G_B, G_Gb;
	Func B_R, B_Gr, B_B, B_Gb;
	Func layerB, layerG, layerR;
	Func color;
	Var x("x"), y("y"), c("c");

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

//ImageParam kernel{ Int(16), 2, "kernel" };
//Param<int32_t> kernel_size{ "kernel_size", 3, 1, 5 };

// 

/* memo
Expr clamped_x = clamp(x, left, _width);
Expr clamped_y = clamp(y, top, _height);

output(x, y, c) = min(input(x, y, c), 255);

clampmin(x, y, c) = min(input(x, y, c), 255);
clampmax(x, y, c) = max(clampmin(x, y, c), 0);
output(x, y, c) = cast<uint8_t>(clampmax(x, y, c));

Halide::Func HalideExtented::Conv3x3(Halide::Func src, int width, int height, int value)
{
	Halide::Func output;
	Halide::Var x, y;
	Halide::RDom r(-1,3,-1,3);

	Halide::Func clamped = Halide::BoundaryConditions::constant_exterior(src, 0);
	output(x, y) = Halide::sum(clamped(x+r.x, y+r.y));

	return output;
}

void halideInitTest5(Mat& src, Mat& dest)
{
	Var x, y, c;
	Halide::Buffer<uint8_t> input(src.ptr<uchar>(0), src.cols, src.rows, src.channels());
	Halide::Buffer<uint8_t> output(dest.ptr<uchar>(0), dest.cols, dest.rows, dest.channels());
	Func adder;
	adder(x, y, c) = cast<uint8_t>(min((cast<int16_t>(input(x, y, c)) + (short)50), 255));

	adder.realize(output);
}

color_image(x, y, c) = select(c == 0, 245, // Red value
	c == 1, 42,  // Green value
	132);        // Blue value

*/


//4つづつ
//Var x_outer, x_inner;
//gradient.split(x, x_outer, x_inner, 4);
//gradient.vectorize(x_inner);

//Var x_outer, x_inner;
//gradient.split(x, x_outer, x_inner, 2);
//gradient.unroll(x_inner);

//producer.compute_at(consumer, y);
//で3ライン保存のラインbyラインなしょりになりそう

//producer.store_root();
//producer.compute_at(consumer, y);
//で3ライン保存のラインbyラインなだけど、全面ストア

//producer.store_root().compute_at(consumer, x);
//四角形にすごく

//Var yo, yi;
//consumer.split(y, yo, yi, 16);
//consumer.parallel(yo);
//consumer.vectorize(x, 4);
//producer.store_at(consumer, yo);
//producer.compute_at(consumer, yi);
//producer.vectorize(x, 4);

/*


circle(x, y) = x + y;
RDom r(0, 7, 0, 7);
r.where((r.x - 3)*(r.x - 3) + (r.y - 3)*(r.y - 3) <= 10);

circle(r.x, r.y) *= 2;

*/