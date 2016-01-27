#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fstream>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int x = 0, y = 0;
long int location = 0;
int blockSize = 5;

void drawBlock(int offsetX, int offsetY) {
    int r = 255/*rand() % 256*/;
    int g = 0/*rand() % 256*/;
    int b = 0/*rand() % 256*/; 
    for (x = 0; x < blockSize; x++) {
        for (y = 0; y < blockSize; y++) {
            location = (offsetX +x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (offsetY +y + vinfo.yoffset) * finfo.line_length;
                *(fbp + location) = g;            // Some blue
                *(fbp + location + 1) = b;        // A little green
                *(fbp + location + 2) = r;        // A lot of red
                *(fbp + location + 3) = 0;          // No transparency 
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1) {
    float deltax = x1 - x0;
    float deltay = y1 - y0;
    float error = 0;
    float deltaerror = abs(deltay / deltax);
    int y = y0;
    for (int x = x0; x < x1; ++x) {
        drawBlock(x, y);
    }
    error = error + deltaerror;
    while (error >= 0.5) {
        drawBlock(x, y);
        y += (y1 - y0);
        error -= 1;
    }
}

int main() {
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

    draw_line(1,1,500,500);
    
    //draw_plane();
    //drawBlock(10, 10);

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}