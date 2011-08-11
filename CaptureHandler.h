#ifndef CAPTURE_HANDLER_H
#define CAPTURE_HANDLER_H

#include <capture.h>
#include "Fast.h"

using namespace std;

class CaptureHandler
{
public:
  CaptureHandler(Fast & f);
  void  handle(int exit_signal, int fast_level, int suppression);
  void open(void);
  void close();
  static char* get_strfast();
  ~CaptureHandler();

private:
  int parse_window(const char *window, int *x1, int *x2, int *y1, int *y2);

  Fast fast;
  media_stream *stream;
  static char* str_corners;
};

#endif // CAPTURE_HANDLER_H
