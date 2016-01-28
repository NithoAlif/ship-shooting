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
#include <ncurses.h>
#include "readfile.cpp"

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
long int location = 0;
char *fbp = 0;


void draw_pixel(int offsetX, int offsetY, int color) {
    int r = 255;
    int g = 255;
    int b = 255; 
    if (offsetX < 0 || offsetX > vinfo.xres)
        return;
    if (offsetY < 0 || offsetY > vinfo.yres)
        return;
        
    location = (offsetX + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
               (offsetY + vinfo.yoffset) * finfo.line_length;

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

// octants
//  \2|1/
//  3\|/0
// ---+---
//  4/|\7
//  /5|6\

void draw_line(float x0, float y0, float x1, float y1, int col = 1) {
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

void draw_bullet(float x0, float y0, float x1, float y1, int col) {
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

	int xmin = (int)x0, xmax = (int)x0;
	int length = (int) dx;
	int first = rand() % length/12 + length/10;
	int second = rand() % length/10 + length/6;
	int third = rand() % length/8 + length/4;
	int rem = length - first - second - third;
	int rem2 = rem/2;
	rem -= rem2;

	xmax += first;
	for (int i = 0; i < 3; i++) {
		for (int x = xmin; x < xmax; x++) {
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
		if (i == 0) {
			xmin += first + rem;
			xmax = xmin + second;
		} else if (i == 1) {
			xmin += second + rem2;
			xmax = xmin + third;
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

// (x0, y0)-------(x1, y1)
//    |               |
//    |               |
//    |               |
// (x2, y2)-------(x3, y3)
void draw_rect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color = 1) {
    draw_line(x1,y1,x2,y2,color);
    draw_line(x2,y2,x4,y4,color);
    draw_line(x4,y4,x3,y3,color);
    draw_line(x3,y3,x1,y1,color); 
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

void draw_image(std::vector< std::vector<char> > &matrix, int x, int y) {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); j++) {
            int k = i * 2;
            if (matrix[i][j] == '1') {
                draw_pixel(j+x, y+k, 1);
                draw_pixel(j+x, y+k+1, 1);
            }/* else {
                draw_pixel(j+x, y+k, 0);
                draw_pixel(j+x, y+k+1, 0);
            } */  
        }
    }
}

void erase_image(std::vector< std::vector<char> > &matrix, int x, int y) {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); j++) {
            int k = i * 2;
            if (matrix[i][j] == '1') {
                draw_pixel(j+x, y+k, 0);
                draw_pixel(j+x, y+k+1, 0);
            }  
        }
    }
}

void rotate_point(float* x1, float* y1, float x2, float y2, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    *x1 -= x2;
    *y1 -= y2;

    // rotate point
    float xnew = (*x1) * c - (*y1) * s;
    float ynew = (*x1) * s + (*y1) * c;

    // translate point back:
    *x1 = xnew + x2;
    *y1 = ynew + y2;
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


    float x1 = 390;
    float y1 = 500;
    float x2 = 410;
    float y2 = 500;
    float x3 = 390;
    float y3 = 580;
    float x4 = 410;
    float y4 = 580;

    float x_origin = x3 + (x4 - x3)/2;
    float y_origin = y4;

    draw_rect(x1, y1, x2, y2, x3, y3, x4, y4);

    // draw_line(0, 0, 100, 700, 1);
    // draw_rect(100, 100, 200, 200);

    std::vector< std::vector<char> > matrix = load_image("ufo.txt");
    int xpos = 0;

    initscr();
    noecho();
    int ch;
    nodelay(stdscr, TRUE);
            float sx=0, sy=0, originx = 0, originy=0;

    while(true) {

    	if ((ch = getch()) == ERR) {
            // user hasn't responded
            erase_image(matrix, xpos-1, 0);
            draw_image(matrix, xpos, 0);
	        xpos++;
			if (xpos > vinfo.xres)
				xpos = 0;
			usleep(5000);
        //draw_line(originx, originy, sx, sy, 0);
		}
        else {
            float angle;
            float length;
            draw_rect(x1, y1, x2, y2, x3, y3, x4, y4, 0);
            //user has pressed a key ch
            switch(ch) {
            	case 65:		// key up
                    angle = atan2((y2 - y1), (x2 - x1));
                    
                    originx = x1 + (x2 - x1)/2;
                    originy = y1 + (y2 - y1)/2;
                    
                    sx = originx;
                    sy = originy + -1000;

                    rotate_point(&sx, &sy, originx, originy, angle);

                    draw_line(originx, originy, sx, sy, 1);
            		break;
	            case 67:		// key right
	                rotate_point(&x1, &y1, x_origin, y_origin, 0.1);
	                rotate_point(&x2, &y2, x_origin, y_origin, 0.1);
	                rotate_point(&x3, &y3, x_origin, y_origin, 0.1);
	                rotate_point(&x4, &y4, x_origin, y_origin, 0.1);
                    //clear(vinfo.xres, vinfo.yres);
	                break;
	            case 68:		// key left
	                rotate_point(&x1, &y1, x_origin, y_origin, -0.1);
	                rotate_point(&x2, &y2, x_origin, y_origin, -0.1);
	                rotate_point(&x3, &y3, x_origin, y_origin, -0.1);
	                rotate_point(&x4, &y4, x_origin, y_origin, -0.1);
                    //clear(vinfo.xres, vinfo.yres);
	                break;
	            default:
	                break;
        	}
        }
        draw_rect(x1, y1, x2, y2, x3, y3, x4, y4);
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
