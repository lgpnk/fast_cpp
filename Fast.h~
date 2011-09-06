#ifndef FAST_H
#define FAST_H
#include "Shared.h"
#include "Pixel.h"
#include <string.h>

using namespace std;

typedef unsigned char uint8_t;

class Fast
{
protected:
  //int m_pix_count;
  int m_lower_t;
  int m_higher_t;
  int m_n;
  int frame_count;
  
  void make_offsets(int pixel[], int row_stride);

  bool zero_dark(const uint8_t* p, const int pixel[]);
  bool zero_bright(const uint8_t* p, const int pixel[]);
  bool eight_dark(const uint8_t* p, const int pixel[]);
  bool eight_bright(const uint8_t* p, const int pixel[]);
  
public:
    Fast(){frame_count = 0;}
    ~Fast(){}

     int fast_corner_score(const uint8_t* p, const int pixel[], int t_start, bool bright);
//     int fast9_corner_score(const uint8_t* p, const int pixel[], int bstart, bool bright);
//     int fast10_corner_score(const uint8_t* p, const int pixel[], int bstart);
//     int fast11_corner_score(const uint8_t* p, const int pixel[], int bstart);
//     int fast12_corner_score(const uint8_t* p, const int pixel[], int bstart);

     Pixel* fast_detect(const uint8_t* im, int xsize, int ysize, int stride, int b, int &ret_num_corners);
//     Pixel* fast9_detect(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);
//     Pixel* fast10_detect(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);
//     Pixel* fast11_detect(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);
//     Pixel* fast12_detect(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);

     void fast_detect_nosup(const uint8_t* im, int xsize, int ysize, int stride, int t, int &ret_num_corners);
     
     int *fast_score(const uint8_t* i, int stride, Pixel* corners, int num_corners, int b);
//     int* fast9_score(const uint8_t* i, int stride, Pixel* corners, int num_corners, int b);
//     int* fast10_score(const uint8_t* i, int stride, Pixel* corners, int num_corners, int b);
//     int* fast11_score(const uint8_t* i, int stride, Pixel* corners, int num_corners, int b);
//     int* fast12_score(const uint8_t* i, int stride, Pixel* corners, int num_corners, int b);

     void fast_detect_nonmax(const uint8_t* im, const int &xsize, const int &ysize, const int &stride, const int &b, int& ret_num_corners, int suppression);
//     Pixel* fast9_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int& ret_num_corners);
//     Pixel* fast10_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);
//     Pixel* fast11_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);
//     Pixel* fast12_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners);

     Pixel* nonmax_suppression(const Pixel* corners, const int* scores, int num_corners, int &ret_num_nonmax);
    
     bool full_seg_test_bright(const uint8_t* p, const int pixel[]);
     bool full_seg_test_dark(const uint8_t* p, const int pixel[]);
    };

#endif // FAST_H
