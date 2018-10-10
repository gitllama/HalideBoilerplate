#include "pch.h"
#include "Logic.h"

#include "Halide.h"
using namespace Halide;


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


#pragma region Util

template<typename T, typename... Ts>
void Logic::speedTest(T * dst, Ts... args)
{
	std::chrono::system_clock::time_point  start, end;
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << ":";
		start = std::chrono::system_clock::now();

		//if (input.dimensions() == 3)
		//{
		//	//int n = sizeof...(args)
		//	//(*this).Realize(dst, 3, width, height);
		//}
		//else if (input.dimensions() == 1)
		//{
		//	//(*this).Realize(dst, width);
		//}
		//else 
		//{
			//(*this).Realize(dst, width, height);
		//}
		(*this).Realize(dst, args);

		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		printf("%f ms\r\n", elapsed);
	}
}

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



#pragma region Init

Logic Logic::Init()
{
	return Init(0);
}

Logic Logic::Init(int dim)
{
	Func output("Init");
	Var x("x"), y("y"), c("c"), i("i");

	switch (dim)
	{
		case 1:
			output(i) = 0;
			break;
		case 3:
			output(c, x, y) = 0;
			break;
		default:
			output(x, y) = 0;
			break;
	}

	return Logic(output);
}

Logic Logic::Init(ImageParam src)
{
	Func output;
	Var x("x"), y("y"), c("c"), i("i");

	switch (src.dimensions())
	{
		case 1:
			output(x, y) = src(x, y);
			break;
		case 3:
			output(c, x, y) = src(c, x, y);
			break;
		default:
			output(x, y) = src(x, y);
			break;
	}

	return Logic(output);
}

Logic Logic::Init(int* src, int width, int height)
{
	ImageParam input(type_of<int>(), 2);
	Buffer<int> _src(src, width, height);
	return Init(input);
}

Logic Logic::Init(int* src, int channels, int width, int height)
{
	ImageParam input(type_of<int>(), 3);
	Buffer<int> _src(src, channels, width, height);
	return Init(input);
}

#pragma endregion


#pragma region Realize / Compile

//JITで使用する際（リアルタイムでのコンパイル

//template<typename T>
//void Logic::Realize(T * dst, int n)
//{
//	Buffer<T> _dst(dst, n);
//	input.realize(_dst);
//}

//template<typename T>
//void Logic::Realize(T * dst, int width, int height)
//{
//	Buffer<T> _dst(dst, width, height);
//	input.realize(_dst);
//}

template<typename T, typename... Ts>
void Logic::Realize(T * dst, Ts... args)
{
	Buffer<T> _dst(dst, args);
	input.realize(_dst);
}

//AOTで使用する際（事前コンパイルでDLLを生成

void Logic::CompileWithRuntime(std::string path, std::vector<Argument> arg, std::string name)
{
	input.compile_to_static_library(path, arg, name);
	printf("Halide pipeline compiled, %s\n", path);
}

void Logic::Compile(std::string path, std::vector<Argument> arg, std::string name)
{
	Target target = get_target_from_environment();

	target.set_feature(Target::NoRuntime, true);

	input.compile_to_static_library(path, arg, name, target);
	//input.compile_to_file(path, arg, name);
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

#pragma endregion


//各Logic

#pragma region PreProcess

// データの帯域圧縮のため 24bitx4 -> 32bitx3 
Logic Logic::Sort(ImageParam src)
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

Logic Logic::SortSchedule()
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

	mono(x, y) = src(x, y);
	output(c, x, y) = mono(x, y);

	return Logic(output);
}

Logic Logic::DemosaicBitshift(Param<int> value)
{
	Func output;
	Var c("c"), x("x"), y("y");

	output(c, x, y) = input(c, x, y) >> value;

	return Logic(output);
}

Logic Logic::DemosaicLUT(ImageParam lut)
{
	Func output;
	Var c("c"), x("x"), y("y");

	Expr clamped = clamp(input(c, x, y), 0, INT32_MAX - 1);
	output(c, x, y) = lut(clamped);

	return Logic(output);
}

Logic Logic::DemosaicHistogram(ImageParam src)
{
	Func output("histogram");
	Var c("c"), x("x"), y("y"), i("i");

	RDom r(0, src.width(), 0, src.height());

	//output(x) = 0;
	output(src(r.x, r.y)) += 1;
	//Buffer<int> halide_result = histogram.realize(256);

	return Logic(input);
}
//Func histogram("histogram");


Logic Logic::DemosaicMonoSchedule()
{
	Var c("c"), x("x"), y("y");

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


#pragma endregion






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

//Buffer<int> shifted(5, 7); // In the constructor we tell it the size.
//shifted.set_min(100, 50);

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
