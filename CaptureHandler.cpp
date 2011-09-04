#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <capture.h>
#include <string>
#include <param.h>

#include "CaptureHandler.h"
#include "Fast.h"
#include "Shared.h"

#define CAPTURE_PROPERTIES    "sdk_format=Y800&resolution=160x120&fps=30"
#define SIZEOF_PROPERTIES     1024

char* CaptureHandler::str_corners;
char* CaptureHandler::get_strfast()
{
    return str_corners;
  
}
CaptureHandler::CaptureHandler(Fast &f)
{
    fast = f;
    stream   = NULL;
    str_corners = (char*)malloc(SIZEOF_APP_CORNER_COORD);
}

CaptureHandler::~CaptureHandler()
{
    close();  
    free(str_corners);
}

void
CaptureHandler::close()
{
    if (stream != NULL) 
    {
	capture_close_stream(stream);
	stream = NULL;
    }
}

void
CaptureHandler::handle(int exit_signal, int fast_level, int suppression)
{
  media_frame *frame = NULL;
  uint8_t *    data  = NULL;
  int          width;
  int          height;
  int          stride;
  int          result = 0;
  int	       num_corner;
  
  frame = capture_get_frame(stream);
  if (!frame) 
  {
      if (exit_signal) 
      {
	  return;
      }
      else 
      {
	  syslog(LOG_CRIT, "Failed to capture frame!");
	  close();
	  exit(EXIT_FAILURE);
      }
  }

  strcpy(str_corners, "");
  data   = (uint8_t*)capture_frame_data(frame);
  width  = capture_frame_width(frame);
  height = capture_frame_height(frame);
  stride = capture_frame_stride(frame);
  
//   if(threadHandler->is_running())
//     pthread_mutex_lock(threadHandler->get_mutex_a());
  fast.fast_detect_nonmax(data, width, height, stride, fast_level, num_corner, suppression);
    if(threadHandler->is_running())
    pthread_mutex_unlock(threadHandler->get_mutex_b());
  capture_frame_free(frame);

}

/* Capture */

void CaptureHandler::open(int res_id)
{
    char     properties[SIZEOF_PROPERTIES];
    const char*   RES[17] = {"160x120","800x600", "640x480", "480x360", "320x240", "240x180", 	//4:3
		        "800x450", "640x360", "480x270", "320x180", "160x90", 			//16:9
		        "800x500", "640x400", "480x300", "320x200", "160x100",			//16:10
		        "176x144" };								//??
		        
//     string capture_properies = "sdk_format=Y800&resolution"+RES+"&fps=30";
    snprintf(properties,
           sizeof(properties),
           "sdk_format=Y800");

    strncat(properties,
            "&resolution=",
            sizeof(properties) - 1);
    strncat(properties,
            RES[res_id],
            sizeof(properties) - 1);
    strncat(properties,
            "&fps=30",
            sizeof(properties) - 1);
    
//     snprintf(properties, SIZEOF_PROPERTIES, CAPTURE_PROPERTIES);

    stream = capture_open_stream(IMAGE_UNCOMPRESSED, properties);
    if (!stream)
    {
	syslog(LOG_CRIT, "Failed to open capture stream!");
	exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Stream Opened!");
}
