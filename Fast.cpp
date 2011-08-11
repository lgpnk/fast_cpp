#include <stdlib.h>
#include "Fast.h"
#include "syslog.h"
void Fast::fast_detect_nonmax(const uint8_t* im, const int& xsize, const int& ysize, const int &stride, const int &b, int& ret_num_corners, int suppression)
{
    	Pixel* corners;    	
	int num_corners;
	
	corners = fast_detect(im, xsize, ysize, stride, b, ret_num_corners);
	if(suppression)
	{
	    int* scores;
	    Pixel* nonmax;
	    num_corners = ret_num_corners;
	    scores = fast_score(im, stride, corners, num_corners, b);
	    nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);
	    free(scores);
	    free(nonmax);
	}
 	free(corners);

// 	ret_num_corners =  num_corners;
}

bool Fast::full_seg_test_bright(const uint8_t* p, const int pixel[])
{
    int pix_count = 1;
    int ptr = 1;
    if(p[pixel[0]] > m_higher_t)
    {
	while(p[pixel[ptr]] > m_higher_t)
	{	
	    if(++pix_count >= 9)
	      return true;
	
	    ptr ++;
	}
    
	ptr = 15;
	//CounterClockWise
	while(p[pixel[ptr]] > m_higher_t)
	{
	    if(++pix_count >= 9)
	      return true;
	
	    ptr--;
	}
    }
    if(p[pixel[8]] > m_higher_t)
    {
	pix_count = 1;
	ptr = 9;
	//ClockWise
	while(p[pixel[ptr]] > m_higher_t)
	{	
	    if(++pix_count >= 9)
	      return true;
	    
	    ptr = (ptr + 1) % 16;
	}
	
	ptr = 7;
	//CounterClockWise
	while(p[pixel[ptr]] > m_higher_t)
	{
	    if(++pix_count >= 9)
	      return true;
	    
	    ptr > 0 ? ptr -- : ptr = 15;
	}
    }  
    return false;
}
  
bool Fast::full_seg_test_dark(const uint8_t* p, const int pixel[])
{
    int pix_count = 1;
    int ptr = 1;
    if(p[pixel[0]] < m_lower_t)
    {
	while(p[pixel[ptr]] < m_lower_t)
	{	
	    if(++pix_count >= 9)
	      return true;
	
	    ptr ++;
	}
    
	ptr = 15;
	//CounterClockWise
	while(p[pixel[ptr]] < m_lower_t)
	{
	    if(++pix_count >= 9)
	      return true;
	
	    ptr--;
	}
    }
    if(p[pixel[8]] < m_lower_t)
    {
	pix_count = 1;
	ptr = 9;
	//ClockWise
	while(p[pixel[ptr]] < m_lower_t)
	{	
	    if(++pix_count >= 9)
	      return true;
	    
	    ptr = (ptr + 1) % 16;
	}
	
	ptr = 7;
	//CounterClockWise
	while(p[pixel[ptr]] < m_lower_t)
	{
	    if(++pix_count >= 9)
	      return true;
	    
	    ptr > 0 ? ptr -- : ptr = 15;
	}
    }  
    return false;
}

// Pixel* Fast::fast10_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
// {
// 	Pixel* corners;
// 	int num_corners;
// 	int* scores;
// 	Pixel* nonmax;
// 
// 	corners = fast10_detect(im, xsize, ysize, stride, b, &num_corners);
// 	scores = fast10_score(im, stride, corners, num_corners, b);
// 	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);
// 
// 	free(corners);
// 	free(scores);
// 
// 	return nonmax;
// }
// 
// Pixel* Fast::fast11_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
// {
// 	Pixel* corners;
// 	int num_corners;
// 	int* scores;
// 	Pixel* nonmax;
// 
// 	corners = fast11_detect(im, xsize, ysize, stride, b, &num_corners);
// 	scores = fast11_score(im, stride, corners, num_corners, b);
// 	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);
// 
// 	free(corners);
// 	free(scores);
// 
// 	return nonmax;
// }
// 
// Pixel* Fast::fast12_detect_nonmax(const uint8_t* im, int xsize, int ysize, int stride, int b, int* ret_num_corners)
// {
// 	Pixel* corners;
// 	int num_corners;
// 	int* scores;
// 	Pixel* nonmax;
// 
// 	corners = fast12_detect(im, xsize, ysize, stride, b, &num_corners);
// 	scores = fast12_score(im, stride, corners, num_corners, b);
// 	nonmax = nonmax_suppression(corners, scores, num_corners, ret_num_corners);
// 
// 	free(corners);
// 	free(scores);
// 
// 	return nonmax;
// }