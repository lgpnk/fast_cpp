#ifndef CAPTURE_HANDLER_H
#define CAPTURE_HANDLER_H

#include <capture.h>
#include "Fast.h"
#include "ThreadHandler.h"

using namespace std;

class CaptureHandler
{
public:
  CaptureHandler(Fast & f);
  void  handle(int exit_signal, int fast_level, int suppression);
  void open(int res_id);
  void close();
  static char* get_strfast();
  ~CaptureHandler();

private:
  int parse_window(const char *window, int *x1, int *x2, int *y1, int *y2);

  Fast fast;
  ThreadHandler *threadHandler;
  media_stream *stream;
  static char* str_corners;
};

#endif // CAPTURE_HANDLER_H
