#include <stdio.h>
#include <algorithm>
#include <math.h>

// octants
//  \2|1/
//  3\|/0
// ---+---
//  4/|\7
//  /5|6\

void plot(int x, int y, int col) {
	printf("(%d,%d)\n", x, y);
}

void bresenham(float x0, float y0, float x1, float y1, int col) {
	const bool steep = fabs(y1 - y0) > fabs(x1 - x0); // octants 1,2,5,6
	
	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	const float dx = x1 - x0;
	const float dy = fabs(y1 - y0);
	float err = dx / 2.0f;
	const int yinc = (y1 > y0) ? 1 : -1;
	int y = (int)y0;
	
	for (int x = (int)x0; x < (int)x1; x++) {
		if (steep) {
			plot(y, x, col);
		} else {
			plot(x, y, col);
		}
		err -= dy;
		if (err < 0) {
			y += yinc;
			err += dx;
		}
	}
}

int main() {
	bresenham(0,0,8,8,0);
}