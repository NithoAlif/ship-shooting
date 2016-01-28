#include <fstream>
#include <vector>							// for vector
#include <string>  
#include <iostream>

using namespace std;

vector<vector<char> > load_image(string image_src) {
    string line;
    ifstream in;
    in.open(image_src.c_str());
    vector<vector<char> > matrix;

    if (in.is_open()) {
        while ( getline (in,line) ) {
            vector<char> row;

            for (int i = 0; i < line.length(); i++) {
                row.push_back(line[i]);
            }

            matrix.push_back(row);
        }
        in.close();                                           
    }
    else {
        cout << "Failed to open" << endl;
    }

    return matrix;
}