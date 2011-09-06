#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <string>
#include <param.h>

#include "Shared.h"
#include "ParamHandler.h"

int ParamHandler::param_fast;
int ParamHandler::param_fast_level;
int ParamHandler::param_fast_suppression;
int ParamHandler::param_res_id;
int ParamHandler::param_capture_fps;

pid_t ParamHandler::app_pid;

ParamHandler::ParamHandler()
{
  param_fast       = DISABLED;
  param_fast_level = 40;
  param_fast_suppression = DISABLED;
  param_res_id = 0;
  param_capture_fps = 30;
}

int ParamHandler::getFastLevel()
{
  return param_fast_level;
}

int ParamHandler::getFastState()
{
  return param_fast;
}

int ParamHandler::getFastSuppression()
{
  return param_fast_suppression;
}

int ParamHandler::getFastResId()
{
  return param_res_id;
}

int ParamHandler::getCaptureFps()
{
  return param_capture_fps;
}

void ParamHandler::init(char *app_name)
{
  char *value = NULL;

  app_pid = getpid();

  if (param_init(app_name) < 0) {
    syslog(LOG_CRIT, "Failed to initialize parameters!");
    exit(EXIT_FAILURE);
  }

  if (param_register_callback(PARAM_ENABLED, handleParam1) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_ENABLED);
    exit(EXIT_FAILURE);
  }

  if (param_get(PARAM_ENABLED, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_ENABLED);
    exit(EXIT_FAILURE);
  }
  handleParam1(PARAM_ENABLED, value);
  param_free(value);

  if (param_register_callback(PARAM_LEVEL, handleParam2) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_LEVEL);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_LEVEL, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_LEVEL);
    exit(EXIT_FAILURE);
  }
  handleParam2(PARAM_LEVEL, value);
  param_free(value);
  
  if (param_register_callback(PARAM_SUPPRESSION, handleParam3) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_SUPPRESSION);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_SUPPRESSION, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_SUPPRESSION);
    exit(EXIT_FAILURE);
  }
  handleParam3(PARAM_SUPPRESSION, value);
  param_free(value);
  
  if (param_register_callback(PARAM_RES, handleParam4) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_RES);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_RES, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_RES);
    exit(EXIT_FAILURE);
  }
  handleParam4(PARAM_RES, value);
  param_free(value);
  
  if (param_register_callback(PARAM_CAPTURE_FPS, handleParam5) < 0) {
  syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_CAPTURE_FPS);
  exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_CAPTURE_FPS, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_CAPTURE_FPS);
    exit(EXIT_FAILURE);
  }
  handleParam5(PARAM_CAPTURE_FPS, value);
  param_free(value);
}

param_stat ParamHandler::handleParam1(const char *name, const char *value)
{
  if (string(value) == "yes") {
    param_fast = ENABLED;
  } else {
    param_fast = DISABLED;
  }

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  kill(app_pid, SIGUSR1);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam2(const char *name, const char *value)
{
  int tmp = atoi(value);

  param_fast_level = tmp < 10 ? 10 : tmp > 60? 60: tmp;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam3(const char *name, const char *value)
{
  if (string(value) == "yes")
      param_fast_suppression = ENABLED;
  else 
      param_fast_suppression = DISABLED;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam4(const char *name, const char *value)
{
  int tmp = atoi(value);
  param_res_id = tmp;
  
  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam5(const char *name, const char *value)
{
  int tmp = atoi(value);

  param_capture_fps = tmp < 4 ? 4 : tmp > 30? 30: tmp;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}