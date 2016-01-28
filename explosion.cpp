void explode_image(std::vector< std::vector<char> > &matrix, int x, int y) {
    int seed = 0, rnd, ctr = 0;

    draw_image_explode(matrix, x, y, 1);

    for (int l = 1; l < 100; l++) {
        for (i = 0; i < matrix.size(); i++) {
            for (j = 0; j < matrix[i].size(); j++) {
                k = i * 2;
                rnd = rnd() % (2 * l);
                rnd = l - rnd;
                if (matrix[i][j] == '1') {
                    if (rnd() % 2 == 1) || (ctr > matrix[i].size() / l / 100) {
                        draw_pixel_explode(j + x + l, y + k + l, 1);
                        draw_pixel_explode(j + x + l, y + k + 1 + l, 1);
                    } else {
                        draw_pixel_explode(j + x + l, y + k + l, 0);
                        draw_pixel_explode(j + x + l, y + k + 1 + l, 0);
                        ctr++;
                    } 
                }
                
            }
            ctr = 0;
        }
    }

    //draw_image_explode(matrix, x, y, 0);
}

void draw_image_explode(std::vector< std::vector<char> > &matrix, int x, int y, int col) {
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); j++) {
            int k = i * 2;
            if (matrix[i][j] == '1') {
                draw_pixel_explode(j + x, y + k, col);
                draw_pixel_explode(j + x, y + k + 1, col);
            }  
        }
    }
}

void draw_pixel_explode(int offsetX, int offsetY, int color) {
    int seed = 30;
    int r = 255 - rnd() % seed;
    int g = 255 - rnd() % seed;
    int b = 0; 
    if (offsetX < 0 || offsetX > vinfo.xres)
        return;
    if (offsetY < 0 || offsetY > (vinfo.yres - 50))
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