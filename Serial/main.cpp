#include <iostream>
#include <bits/stdc++.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <chrono>

//  ./ImageFilters.out input.bmp

#define BLACK 0
#define WHITE 255
#define RED 6
#define BLUE 7
#define GREEN 8


using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using namespace std;

#pragma pack(1)
#pragma once

typedef unsigned char UCHAR;
typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;


class Pixel{
  public:
    UCHAR red, green, blue;
    Pixel(int r, int g, int b){
      red = UCHAR(r);
      blue = UCHAR(b);
      green = UCHAR(g);
    }
    Pixel(){
      red = UCHAR(0);
      green = UCHAR(0);
      blue = UCHAR(0);
    }
};
typedef vector<vector<Pixel>> PixelTable;

int rows;
int cols;

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize, PixelTable &pixelt){
  ifstream file(fileName);

  if (file){
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else{
    cout << "File" << fileName << " doesn't exist!" << endl;
    return 0;
  }
}

void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer, PixelTable &pixelt){
  PixelTable vec(rows, vector<Pixel> (cols));
  pixelt = vec;
  int count = 1;
  int extra = cols % 4;
  for (int i = rows - 1 ; i >= 0; i--){
    count += extra;
    for (int j = cols - 1; j >= 0; j--){
      for (int k = 0; k < 3; k++){
        switch (k){
        case 0:
          // fileReadBuffer[end - count] is the red value
          pixelt[i][j].red = fileReadBuffer[end - count];
          count++;
          break;
        case 1:
          // fileReadBuffer[end - count] is the green value
          pixelt[i][j].green = fileReadBuffer[end - count];
          count++;
          break;
        case 2:
          // fileReadBuffer[end - count] is the blue value
          pixelt[i][j].blue = fileReadBuffer[end - count];
          count++;
          break;
          // go to the next position in the buffer
        }
      }
    }
  }
}


void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize, PixelTable pixelt){
  std :: ofstream write(nameOfFileToCreate);
  if (!write) {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = rows - 1 ; i >= 0; i--){
    count += extra;
    for (int j = cols - 1; j >= 0; j--){
      for (int k = 0; k < 3; k++){
        switch (k)
        {
        case 0:
          fileBuffer[bufferSize - count] = pixelt[i][j].red;
          count++;
          break;
        case 1:
          fileBuffer[bufferSize - count] = pixelt[i][j].green;
          count++;
          break;
        case 2:
          fileBuffer[bufferSize - count] = pixelt[i][j].blue;
          count++;
          break;
        }
      }
    }
  }
  write.write(fileBuffer, bufferSize);
}


PixelTable VerticalMirror(PixelTable pixelt) {
  auto start = chrono::high_resolution_clock::now();

  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows / 2; i++) {
      swap(pixelt[i][j], pixelt[rows - i - 1][j]);
    }
  }

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
  cout << "Flip: " << duration.count() / double(1e3) << " ms" << endl;

  return pixelt;
}

/*int ConvolutionHandler(int i, int j, PixelTable &pixelt, int color)
{
  vector<vector<int>> matrix{{1, 2, 1}, {2, 4, 2}, {1, 2, 1}}; // gausian Blur 
  int result = 0;

  for (int k = -1; k <= 1; k++)
  {
    for (int l = -1; l <= 1; l++)
    {
      int newI = i + k;
      int newJ = j + l;

      if (newI >= 0 && newI < rows && newJ >= 0 && newJ < cols)
      {
        switch (color)
        {
        case BLUE:
          result += pixelt[newI][newJ].blue * matrix[k + 1][l + 1];
          break;
        case GREEN:
          result += pixelt[newI][newJ].green * matrix[k + 1][l + 1];
          break;
        case RED:
          result += pixelt[newI][newJ].red * matrix[k + 1][l + 1];
          break;
        }
      }
    }
  }

  // normalize 
  result /= 16;

  if (result > 255)
  {
    return 255;
  }
  if (result < 0)
  {
    return 0;
  }

  return result;
}

PixelTable BlurFilter(PixelTable pixelt) {
  auto start = chrono::high_resolution_clock::now();

  PixelTable temp = pixelt;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      pixelt[i][j].red = ConvolutionHandler(i, j, temp, RED);
      pixelt[i][j].blue = ConvolutionHandler(i, j, temp, BLUE);
      pixelt[i][j].green = ConvolutionHandler(i, j, temp, GREEN);
    }
  }

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
  cout << "Blur: " << duration.count() / double(1e3) << " ms" << endl;

  return pixelt;
}*/


int ConvolutionFilter(int i, int j, PixelTable &pixelt, const vector<vector<int>> &kernel, int color) {
    int kernelSize = kernel.size();
    int halfKernelSize = kernelSize / 2;

    int result = 0;

    for (int ki = 0; ki < kernelSize; ++ki) {
        for (int kj = 0; kj < kernelSize; ++kj) {
            int ni = i + ki - halfKernelSize;
            int nj = j + kj - halfKernelSize;

            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                switch (color) {
                    case BLUE:
                        result += pixelt[ni][nj].blue * kernel[ki][kj];
                        break;
                    case GREEN:
                        result += pixelt[ni][nj].green * kernel[ki][kj];
                        break;
                    case RED:
                        result += pixelt[ni][nj].red * kernel[ki][kj];
                        break;
                }
            }
        }
    }

    // normalize
    result /= 16;

    result = max(0, min(255, result));

    return result;
}

PixelTable BlurFilter(PixelTable &pixelt) {
    auto start = chrono::high_resolution_clock::now();

    vector<vector<int>> blurKernel{{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    PixelTable temp = pixelt;

    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            int red = ConvolutionFilter(i, j, temp, blurKernel, RED);
            int blue = ConvolutionFilter(i, j, temp, blurKernel, BLUE);
            int green = ConvolutionFilter(i, j, temp, blurKernel, GREEN);

            pixelt[i][j].red = UCHAR(red);
            pixelt[i][j].blue = UCHAR(blue);
            pixelt[i][j].green = UCHAR(green);
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Blur: " << duration.count() / double(1e3) << " ms" << endl;

    return pixelt;
}

PixelTable PurpleFilter(PixelTable pixelt) {
  auto start = chrono::high_resolution_clock::now();

  for (int i = 0; i < rows; i ++){
    for (int j = 0; j < cols; j ++){
      int red = pixelt[i][j].red;
      int green = pixelt[i][j].green;
      int blue = pixelt[i][j].blue;
      int sepiaRed = int(0.5 * red + 0.3 * green + 0.5 * blue);
      int sepiaGreen = int(0.16 * red + 0.5 * green + 0.16 * blue);
      int sepiaBlue = int(0.6 * red + 0.2 * green + 0.8 * blue);
      pixelt[i][j].red = (sepiaRed > 255 ? 255 : sepiaRed);
      pixelt[i][j].green = (sepiaGreen > 255 ? 255 : sepiaGreen);
      pixelt[i][j].blue = (sepiaBlue > 255 ? 255 : sepiaBlue);
    }
  }

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
  cout << "Purple: " << duration.count() / double(1e3) << " ms" << endl;

  return pixelt;
}

PixelTable LinesFilter(PixelTable pixelt) {
   auto start = chrono::high_resolution_clock::now();

  float m = (float)rows / cols; 
  
  int lineWidth = 10;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
    
      // diameter line  
      if (abs(i - j*m) < lineWidth/5) {
        pixelt[i][j].red = WHITE; 
        pixelt[i][j].green = WHITE;
        pixelt[i][j].blue = WHITE;
      }

      // line above diameter
      if (abs(i - (j*m + 255)) < lineWidth/5) {
        pixelt[i][j].red = WHITE;
        pixelt[i][j].green = WHITE;
        pixelt[i][j].blue = WHITE;
      }

      // line below diameter  
      if (abs(i - (j*m - 255)) < lineWidth/5) {
        pixelt[i][j].red = WHITE;
        pixelt[i][j].green = WHITE;
        pixelt[i][j].blue = WHITE;
      }
    }
  }

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
  cout << "Lines: " << duration.count() / double(1e3) << " ms" << endl;

  return pixelt;
}

int main(int argc, char *argv[]) {
  auto totalStart = chrono::high_resolution_clock::now();

  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  PixelTable pixelt;

  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize, pixelt)) {
    cout << "Error in reading the file" << endl;
    return 1;
  }

  // read input
  auto readStart = chrono::high_resolution_clock::now();
  getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer, pixelt);
  auto readEnd = chrono::high_resolution_clock::now();
  auto readDuration = chrono::duration_cast<chrono::microseconds>(readEnd - readStart);
  cout << "Read: " << readDuration.count() / double(1e3) << " ms" << endl;

  //filters  
  pixelt = VerticalMirror(pixelt);
  pixelt = BlurFilter(pixelt);
  pixelt = PurpleFilter(pixelt);
  pixelt = LinesFilter(pixelt);

  // write output
  auto writeStart = chrono::high_resolution_clock::now();
  char *outname = new char[20];
  strcpy(outname, "output.bmp");
  writeOutBmp24(fileBuffer, outname, bufferSize, pixelt);
  auto writeEnd = chrono::high_resolution_clock::now();
  auto writeDuration = chrono::duration_cast<chrono::microseconds>(writeEnd - writeStart);
 // cout << "Write: " << writeDuration.count() / double(1e3) << " ms" << endl;

 //final execution time
  auto totalEnd = chrono::high_resolution_clock::now();
  auto totalDuration = chrono::duration_cast<chrono::microseconds>(totalEnd - totalStart);
  cout << "Execution: " << totalDuration.count() / double(1e3) << " ms" << endl;

  return 0;
}
