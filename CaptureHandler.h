#ifndef CAPTURE_HANDLER_H
#define CAPTURE_HANDLER_H

#include <capture.h>
#include "Fast.h"
#include "Sobel.h"
#include "Kmeans.h"
#include "ThreadHandler.h"
#include "ParamHandler.h"

using namespace std;
struct Profiler
{
    int m_time_grab;
    int m_time_corner;
    int m_time_supp;
    int m_time_kmeans;
    int m_time_edge;
    int m_time_proc;
    int num_corner;
    void reset()
    {
	m_time_grab = 0;
	m_time_corner = 0;
	m_time_supp = 0;
	m_time_edge = 0;
	m_time_kmeans = 0;
	m_time_proc = 0;
	num_corner = 0;
    }
};
class CaptureHandler
{
public:
  CaptureHandler(Fast & f, Sobel &s);
  void  handle(int exit_signal, Params params);
  void open(int res_id, int fps);
  void close();
  static char* get_strfast();
  static char* get_strsobel();
  static char* get_strkmeans();
  static Kmeans* get_kmeans();
  static Profiler get_profiler();
  ~CaptureHandler();

private:
  int parse_window(const char *window, int *x1, int *x2, int *y1, int *y2);

  char* m_fast_param;
  char* m_sobel_param;
  
  Fast fast;
  Sobel sobel;
  ThreadHandler *threadHandler;
  media_stream *stream;
  static Kmeans m_kmeans;
  static char* str_corners;
  static char* str_sobel;
  static char* str_kmeans;
  static Profiler profiler;

};

#endif // CAPTURE_HANDLER_H
