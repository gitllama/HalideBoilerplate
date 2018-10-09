#include "pch.h"
#include "SubLogic.h"

#include "Halide.h"
using namespace Halide;

SubLogic::SubLogic()
{
}

SubLogic::SubLogic(Func func, int width, int height, int channels)
{
	input = func;
	_width = width;
	_height = height;
	_channels = channels;
}

SubLogic::~SubLogic()
{
}

// JITで使用する際（リアルタイムでのコンパイル
// JITとAOTを同時に行う場合、ImageParam, Param<T>と使用してロジックを
// 共通化したほうが良い。

SubLogic SubLogic::Init(int width, int height, int channels)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = 0;

	return SubLogic(output, width, height, channels);
	/*

	メソッドチェーンは
	ClassName& Name(){ return *this; }
	で記載できるが、Funcを確保する為毎度インスタンス生成

	*/
}

SubLogic SubLogic::Init(int* src, int width, int height, int channels)
{
	Func output;
	Var x, y, c;

	Buffer<int> _src(src, channels, width, height);
	output(c, x, y) = _src(c, x, y);

	return SubLogic(output, width, height, channels);
}

SubLogic SubLogic::Offset(int value)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = input(c, x, y) + value;

	return SubLogic(output, _width, _height, _channels);
}

void SubLogic::Finalize()
{
	input.realize(_channels, _width, _height);
}

void SubLogic::Finalize(int * dst)
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

void SubLogic::Finalize(uint8_t * dst)
{
	Buffer<uint8_t> _dst(dst, _channels, _width, _height);
	input.realize(_dst);
}