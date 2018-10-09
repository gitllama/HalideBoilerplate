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

// JIT�Ŏg�p����ہi���A���^�C���ł̃R���p�C��
// JIT��AOT�𓯎��ɍs���ꍇ�AImageParam, Param<T>�Ǝg�p���ă��W�b�N��
// ���ʉ������ق����ǂ��B

SubLogic SubLogic::Init(int width, int height, int channels)
{
	Func output;
	Var x, y, c;

	output(c, x, y) = 0;

	return SubLogic(output, width, height, channels);
	/*

	���\�b�h�`�F�[����
	ClassName& Name(){ return *this; }
	�ŋL�ڂł��邪�AFunc���m�ۂ���ז��x�C���X�^���X����

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

	���ڗp�ӂ��ꂽ�z��ɕ��荞�񂾕����������}�l�W�����g�I��
	�����ƕ֗��Ȃ̂ł��܂�p�r�͂Ȃ���

	Buffer<unsigned char> output = input.realize(width, height, channels);

	�Ƃ��邱�Ƃŉ��L�̂悤�Ɋe�v�f�ɃA�N�Z�X�\�B

	���T�C�Y : output.width();
	�c�T�C�Y : output.height();
	�`���l���� : output.channels();
	�v�f : output(x,y,c);
�@�@�擪�|�C���^ : output.begin(); / output2.data();
	�Ō���|�C���^ : output.end();

	*/
}

void SubLogic::Finalize(uint8_t * dst)
{
	Buffer<uint8_t> _dst(dst, _channels, _width, _height);
	input.realize(_dst);
}