#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int x = 0, y = 0;
long int location = 0;
int fontWidth = 1;
int fontHeight = 9;


void draw_pixel(int offsetX, int offsetY, int color) {
    int r = 255;
    int g = 255;
    int b = 255; 
    if (offsetY < 700 && offsetY > 68) {
        for (int x = 0; x < 1; x++) {
            for (int y = 0; y < 1; y++) {
                location = (offsetX +x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                           (offsetY +y + vinfo.yoffset) * finfo.line_length;

                if (color == 1) {
                    *(fbp + location) = g;
                    *(fbp + location + 1) = b;
                    *(fbp + location + 2) = r;
                    *(fbp + location + 3) = 0;
                } else {
                    *(fbp + location) = 0;
                    *(fbp + location + 1) = 0;
                    *(fbp + location + 2) = 0;
                    *(fbp + location + 3) = 0;
                }
                
            }
        }
    } 
}

// octants
//  \2|1/
//  3\|/0
// ---+---
//  4/|\7
//  /5|6\

void draw_line(float x0, float y0, float x1, float y1, int col) {
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
			draw_pixel(y, x, col);
		} else {
			draw_pixel(x, y, col);
		}
		err -= dy;
		if (err < 0) {
			y += yinc;
			err += dx;
		}
	}
}


// (x0, y0)------------
//    |               |
//    |               |
//    |               |
//    ------------(x1, y1)
void draw_rect(int x0, int y0, int x1, int y1, int color = 1) {
	draw_line(x0, y0, x1, y0, color);
	draw_line(x1, y0, x1, y1, color);
	draw_line(x1, y1, x0, y1, color);
	draw_line(x0, y1, x0, y0, color);
}

void clear(int xRes, int yRes) {
    for (int x = 0; x < xRes - 10; x++) {
        for (int y = 0; y < yRes - 10; y++) {
            location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y + vinfo.yoffset) * finfo.line_length;

            *(fbp + location) = 0;        // Some blue
            *(fbp + location + 1) = 0;      // A little green
            *(fbp + location + 2) = 0;      // A lot of red
            *(fbp + location + 3) = 0;      // No transparency
        }
    }
}


int main()
{
    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (*fbp - '0' == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    clear(vinfo.xres, vinfo.yres);
    draw_line(0, 0, 100, 700, 1);
    draw_rect(100, 100, 200, 200);
    
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
