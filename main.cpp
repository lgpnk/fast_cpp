/****************** INCLUDE FILES SECTION ***********************************/

#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <libgen.h>
#include <rapp/rapp.h>

#include "Shared.h"
#include "CaptureHandler.h"
#include "HttpHandler.h"
#include "SignalHandler.h"
#include "ParamHandler.h"
#include "Fast.h"


/****************** CONSTANT AND MACRO SECTION ******************************/

#define MAJOR_VERSION    1
#define MINOR_VERSION    1

/****************** LOCAL FUNCTION DECLARATION SECTION **********************/

/* Syslog */
static void open_syslog(const char *app_name);
static void close_syslog(void);

/* Daemon */
static void daemonize(void);

/****************** LOCAL VARIABLE DECLARATION SECTION **********************/

/****************** FUNCTION DEFINITION SECTION *****************************/

int
main(int argc, char *argv[])
{
  int fast_status      = FAST_IDLE;
  char *         app_name = basename(argv[0]);
  int            result   = 0;
  
  /* Parameters */
  ParamHandler   paramHandler;
  /* HTTP */
  HttpHandler    httpHandler;
  /* Signals */
  SignalHandler  signalHandler;
  /* Fast */
  Fast       fast;
  /* Sobel */
  Sobel sobel;
  /* Capture */
  CaptureHandler captureHandler(fast, sobel);

  open_syslog(app_name);

  daemonize();

  rapp_initialize();
  
  /* initialization */
  signalHandler.init();

  paramHandler.init(app_name);

  if (httpHandler.init(app_name) < 0) {
    exit(EXIT_FAILURE);
  }

  /* main loop */
  while (!signalHandler.getExitSignal())
  {
      switch (fast_status) 
      {
      case FAST_IDLE:
	result =
	  httpHandler.waitForFastStart(paramHandler.getParams().param_app);
	if (result != -1) 
	{
	    fast_status = result;
	}
	break;

      case FAST_START:
	captureHandler.open(paramHandler.getParams().param_res_id);
	syslog(LOG_INFO, "fastres");
	fast_status = FAST_RUNNING;
	break;

      case FAST_RUNNING:
	captureHandler.handle(signalHandler.getExitSignal(), paramHandler.getParams());
	result = httpHandler.checkForFastStopOrReconf(paramHandler.getParams().param_app);
	if (result != -1) 
	{
	    fast_status = result;
	}
	break;

      case FAST_RECONF:
	captureHandler.close();
	captureHandler.open(paramHandler.getParams().param_res_id);
	fast_status = FAST_RUNNING;
	break;

      case FAST_STOP:
	captureHandler.close();
	fast_status = FAST_IDLE;
	break;
      }
  }

  /* clean up */
  close_syslog();

  rapp_terminate();
  
  return EXIT_SUCCESS;
}

/****************** LOCAL FUNCTION DEFINITION SECTION ***********************/

/* Syslog */
static void
open_syslog(const char *app_name)
{
  openlog(app_name, LOG_PID, LOG_LOCAL4);
  syslog(LOG_INFO, "Starting!");
}

static void
close_syslog(void)
{
  syslog(LOG_INFO, "Exiting!");
}

/* Daemon */
static void
daemonize(void)
{
  if (daemon(0, 0) < 0) {
    syslog(LOG_CRIT, "Failed to daemonize!");
    exit(EXIT_FAILURE);
  }
}

/*********************** END OF FILE ****************************************/
