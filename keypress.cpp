#include <ncurses.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
using namespace std;

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int x = 0, y = 0;
long int location = 0;
int blockSize = 10;
int fontWidth = 5;
int fontHeight = 9;

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

void drawBlock(int offsetX, int offsetY, int color) {
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256; 
    if (offsetY < 600 && offsetY > 0) {
        for (x = 0; x < blockSize; x++) {
            for (y = 0; y < blockSize; y++) {
                location = (offsetX +x + vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                           (offsetY +y + vinfo.yoffset) * finfo.line_length;

                if (color == 1) {
                    *(fbp + location) = 255;            // Some blue
                    *(fbp + location + 1) = 255;        // A little green
                    *(fbp + location + 2) = 255;        // A lot of red
                    *(fbp + location + 3) = 0;          // No transparency 
                } else {
                    *(fbp + location) = 0;              // Some blue
                    *(fbp + location + 1) = 0;          // A little green
                    *(fbp + location + 2) = 0;          // A lot of red
                    *(fbp + location + 3) = 0;          // No transparency 
                }
                
            }
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

    initscr();
    noecho();
    timeout(-1);
    int c;

    int posX = 400;
    int posY = 300;

    while(1) {
        c = getch();   
        drawBlock(posX, posY, 0);
        switch(c) {
            case 65:       // key up
                posY -= 10;   
                drawBlock(posX, posY, 1);
                break;
            case 66:       // key down
                posY += 10;
                drawBlock(posX, posY, 1);     
                break;
            case 67:       // key right
                posX += 10;   
                drawBlock(posX, posY, 1);
                break;
            case 68:       // key left
                posX -= 10;
                drawBlock(posX, posY, 1);     
                break;
            default:
                drawBlock(posX, posY, 1);
                break;
        }
    }

    munmap(fbp, screensize);
    close(fbfd);

    return 0;
}