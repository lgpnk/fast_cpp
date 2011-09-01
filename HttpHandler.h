#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <net_http.h>
#include "ThreadHandler.h"
#include <complex>

using namespace std;

class HttpHandler
{
public:
  HttpHandler();
  int  init(char *app_name);
  void closeHttp();
  int  waitForFastStart(int param_fast);
  int  checkForFastStopOrReconf(int param_fast);

  ~HttpHandler();
    void get_strfast();
protected: 
    void InternalThreadEntry();
private:
  static ThreadHandler *threadHandler;
  static void handle_corners(const char *method, const char *path, const http_options *options, int fd);
  static void handle_update(const char *method, const char *path, const http_options *options, int fd);
  static void handle_sobel(const char *method, const char *path, const http_options *options, int fd);
  
};
struct Arg
{
    ThreadHandler *threadHandler;
    int fd;    
};
#endif // HTTP_HANDLER_H
