#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <string>
#include <param.h>

#include "Shared.h"
#include "ParamHandler.h"

Params ParamHandler::m_params;
// int ParamHandler::param_app;
// int ParamHandler::param_fast_level;
// int ParamHandler::param_fast_suppression;
// int ParamHandler::param_res_id;
// int ParamHandler::param_fast;
// int ParamHandler::param_sobel;
// int ParamHandler::param_sobel_operation;
// int ParamHandler::param_threshold_level;
// 
pid_t ParamHandler::app_pid;

ParamHandler::ParamHandler()
{
    m_params.init();
}

Params ParamHandler::getParams()
{
    return m_params;
}
// int ParamHandler::getAppState()
// {
//   return param_app;
// }
// 
// int ParamHandler::getFastLevel()
// {
//   return param_fast_level;
// }
// 
// int ParamHandler::getFastSuppression()
// {
//   return param_fast_suppression;
// }
// 
// int ParamHandler::getFastResId()
// {
//   return param_res_id;
// }
// 
// int ParamHandler::getFastState()
// {
//   return param_fast;
// }
// 
// int ParamHandler::getSobelState()
// {
//   return param_sobel;
// }
// 
// int ParamHandler::getSobelOperation()
// {
//   return param_sobel_operation;
// }
// 
// int ParamHandler::getThresholdLevel()
// {
//   return param_threshold_level;
// }

void ParamHandler::init(char *app_name)
{
  char *value = NULL;

  app_pid = getpid();

  if (param_init(app_name) < 0) {
    syslog(LOG_CRIT, "Failed to initialize parameters!");
    exit(EXIT_FAILURE);
  }

  /***********************PARAM APP ENABLE************************/
  
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

  /***********************PARAM THRESHOLD FAST************************/
  
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
  
  /***********************PARAM SUPPRESSION FAST************************/
  
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
  
  /***********************PARAM RESOLUTION BOTH************************/
  
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
  
  /***********************PARAM FAST ENABLE************************/
  
  if (param_register_callback(PARAM_FAST, handleParam5) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_FAST);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_FAST, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_FAST);
    exit(EXIT_FAILURE);
  }
  handleParam5(PARAM_FAST, value);
  param_free(value);
  
  /***********************PARAM SOBEL ENABLE************************/
  
  if (param_register_callback(PARAM_SOBEL, handleParam6) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_SOBEL);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_SOBEL, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_SOBEL);
    exit(EXIT_FAILURE);
  }
  handleParam6(PARAM_SOBEL, value);
  param_free(value);
  
  /***********************PARAM THRESHOLD ENABLE************************/
  
  if (param_register_callback(PARAM_SOBEL_OPERATION, handleParam7) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_SOBEL_OPERATION);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_SOBEL_OPERATION, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_SOBEL_OPERATION);
    exit(EXIT_FAILURE);
  }
  handleParam7(PARAM_SOBEL_OPERATION, value);
  param_free(value);
  
    /***********************PARAM THRESHOLD LEVEL************************/
  
  if (param_register_callback(PARAM_THRESHOLD_LEVEL, handleParam8) < 0) {
    syslog(LOG_CRIT, "Failed to register callback for parameter: %s", PARAM_THRESHOLD_LEVEL);
    exit(EXIT_FAILURE);
  }
  if (param_get(PARAM_THRESHOLD_LEVEL, &value) < 0) {
    syslog(LOG_CRIT, "Failed to get parameter: %s", PARAM_THRESHOLD_LEVEL);
    exit(EXIT_FAILURE);
  }
  handleParam8(PARAM_THRESHOLD_LEVEL, value);
  param_free(value);
}

param_stat ParamHandler::handleParam1(const char *name, const char *value)
{
  if (string(value) == "yes") {
    m_params.param_app = ENABLED;
  } else {
    m_params.param_app = DISABLED;
  }

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  kill(app_pid, SIGUSR1);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam2(const char *name, const char *value)
{
  int tmp = atoi(value);

  m_params.param_fast_level = tmp < 10 ? 10 : tmp > 60? 60: tmp;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam3(const char *name, const char *value)
{
  if (string(value) == "yes")
      m_params.param_fast_suppression = ENABLED;
  else 
      m_params.param_fast_suppression = DISABLED;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam4(const char *name, const char *value)
{
  int tmp = atoi(value);
  m_params.param_res_id = tmp;
  
  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam5(const char *name, const char *value)
{
  if (string(value) == "yes")
      m_params.param_fast = ENABLED;
  else 
      m_params.param_fast = DISABLED;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam6(const char *name, const char *value)
{
  if (string(value) == "yes")
      m_params.param_sobel = ENABLED;
  else 
      m_params.param_sobel = DISABLED;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam7(const char *name, const char *value)
{
  int tmp = atoi(value);
  
  m_params.param_sobel_operation = tmp;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}

param_stat ParamHandler::handleParam8(const char *name, const char *value)
{
  int tmp = atoi(value);

  m_params.param_threshold_level = tmp > 255? 255: tmp;

  syslog(LOG_INFO, "Parameter updated: Name: %s, Value: %s\n", name, value);

  return PARAM_OK;
}