#include "pch.h"
#include "Util.h"

#include "Logic.h"
#include "Halide.h"
using namespace Halide;

Util::Util()
{
}

Util::~Util()
{
}

void Util::speedTest(Logic code, int* _dst, int w, int h) {

	std::chrono::system_clock::time_point  start, end;
	for (int i = 0; i < 10; i++)
	{
		std::cout << i << ":";
		start = std::chrono::system_clock::now();

		code.Realize(_dst, 3, w, h);

		end = std::chrono::system_clock::now();
		double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		printf("%f ms\r\n", elapsed);
	}
}

void Util::printArray(int* _src, int* _dst, int w, int h) {
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
