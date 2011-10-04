#ifndef PARAM_HANDLER_H
#define PARAM_HANDLER_H

#include <param.h>

using namespace std;

class ParamHandler
{
public:
  ParamHandler();
  int  getAppState();
  int  getFastLevel();
  int  getFastSuppression();
  int  getFastResId();
  int  getFastState();
  int  getSobelState();
  int  getSobelOperation();
  int  getThresholdLevel();
  
  void init(char *app_name);

private:
  static param_stat handleParam1(const char *name, const char *value);
  static param_stat handleParam2(const char *name, const char *value);
  static param_stat handleParam3(const char *name, const char *value);
  static param_stat handleParam4(const char *name, const char *value);
  static param_stat handleParam5(const char *name, const char *value);
  static param_stat handleParam6(const char *name, const char *value);
  static param_stat handleParam7(const char *name, const char *value);
  static param_stat handleParam8(const char *name, const char *value);

  static int 	param_app;
  static int    param_fast_level;
  static int    param_fast_suppression;
  static int 	param_res_id;
  static int    param_fast;
  static int    param_sobel;
  static int    param_sobel_operation;
  static int    param_threshold_level;
  
  static pid_t  app_pid;
};

#endif // PARAM_HANDLER_H
