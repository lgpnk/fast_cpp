#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <net_http.h>
#include <capture.h>
#include <rapp/rapp.h>

#include "HttpHandler.h"
#include "ParamHandler.h"
#include "CaptureHandler.h"
#include "Shared.h"
#include "SignalHandler.h"

#define HTTP_PATH_CORNERS	"/local/fastcpp/corners.cgi"
#define HTTP_PATH_SOBEL		"/local/fastcpp/sobel.pgm"
#define HTTP_PATH_UPDATE	"/local/fastcpp/update.cgi"

#define HTTP_TIMEOUT    30
#define HTTP_WAIT       -1
#define HTTP_NO_WAIT    0

#define LOG(fmt, args...)    { syslog(LOG_INFO, fmt, ## args); printf(fmt, ## args); }

static bool window_changed = false;
ThreadHandler *HttpHandler::threadHandler = new ThreadHandler();

void* start_thread(void* tfd);
void sobel(const uint8_t* buf, int width, int height, int stride, uint8_t* new_buf);

HttpHandler::HttpHandler()
{
}

HttpHandler::~HttpHandler()
{
    net_http_cleanup();
}

int HttpHandler::init(char *app_name)
{
    if (net_http_init(app_name) < 0) 
    {
	syslog(LOG_CRIT, "Failed to init HTTP!");
	return -1;
    }
    if (net_http_add_path(HTTP_PATH_CORNERS, this->handle_corners) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_CORNERS);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_CORNERS);
    
    if (net_http_add_path(HTTP_PATH_SOBEL, this->handle_sobel) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_SOBEL);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_SOBEL);
    
    if (net_http_add_path(HTTP_PATH_UPDATE, this->handle_update) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_UPDATE);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_UPDATE);
    
  return 0;
}


void HttpHandler::handle_corners(const char *method, const char *path, const http_options *options, int fd)
{
    ssize_t      ret_value  = 1;
    char * value;
    pthread_t thread;
    Arg *arg = new Arg();
    
    arg->threadHandler = threadHandler;
    arg->fd = fd;
    
    ret_value = net_http_send_headers(fd,
				  HTTP_TIMEOUT,
				  txt_HTTP_HEADER_200,
				  txt_Content_Type_text_html_utf8,
				  txt_CRLF,
				  NULL);

    param_get(PARAM_ENABLED, &value);

    if(ret_value > 0 && string(value) == "yes")
    {
	threadHandler->add_thread();
	pthread_create(&thread, NULL, start_thread, (void*)arg);
    }
    else
      close(fd);
    
    param_free(value);
}

void* start_thread(void* tmp_arg)
{
    ssize_t      ret_value  = 1;
    
    Arg *arg =(Arg*) tmp_arg;
    int fd = (int) arg->fd;
    ThreadHandler *threadHandler = arg->threadHandler;
    char         my_string[SIZEOF_APP_CORNER_COORD];
    syslog(LOG_CRIT, "Start HTTP Thread");
    while(ret_value > 0) 
    {
	pthread_mutex_lock(threadHandler->get_mutex_b());
	snprintf(my_string,
             sizeof(my_string),
             "%s",
             CaptureHandler::get_strfast());
	
	int len = strlen(my_string);
	
	while (len < SIZEOF_PKG)
	  len += sprintf(my_string + len, " ");
	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);	
 	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, "EOF");	
// 	pthread_mutex_unlock(threadHandler->get_mutex_a());
	if ((SignalHandler::getExitSignal()) || threadHandler->getStopRequest()) 
	    ret_value = -1; /* end loop */
	    
// 	net_http_waitfor_and_handle_req(0);
    }
    close(fd);
    threadHandler->remove_thread();
    syslog(LOG_CRIT, "Close HTTP Thread");
    pthread_exit(NULL);
}

void HttpHandler::handle_update(const char* method, const char* path, const http_options* options, int fd)
{
  const char *action       = NULL;
  const char *fast_level   = NULL;
  const char *fast_enabled = NULL;
  const char *suppression  = NULL;
  const char *http_thread  = NULL;

  syslog(LOG_INFO, "Received HTTP Request: %s", path);

  if (!(action = net_http_option(options, "action"))) {
    goto bad_request;
  }
  syslog(LOG_INFO, "action=%s", action);

  if (string(action) == "set") 
  {
      if ((fast_level = net_http_option(options, "level"))) 
      {
	  if (param_set(PARAM_LEVEL, fast_level, 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_LEVEL, fast_level);
	    goto server_error;
	  }
      }
      else if ((suppression = net_http_option(options, "suppression"))) 
      {
	  if (param_set(PARAM_SUPPRESSION, suppression, 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SUPPRESSION, suppression);
	    goto server_error;
	  }
      }
      else if ((http_thread = net_http_option(options, "http_thread"))) 
      {
	   if(threadHandler->is_running())
	      threadHandler->stop_thread_request();
      }



    if (net_http_send_headers(fd,
                              HTTP_TIMEOUT,
                              txt_HTTP_HEADER_200,
                              txt_Content_Type_text_html_utf8,
                              txt_CRLF,
                              NULL) < 0) {
      goto response_failure;
    }

    syslog(LOG_INFO, "Response sent!");
    close(fd);
    return;
}
bad_request:
  syslog(LOG_WARNING, "Bad Request!");
  if (net_http_send_string_utf8(fd, HTTP_TIMEOUT,
                                txt_HTTP_RESPONSE_400) < 0) {
    goto response_failure;
  }
  close(fd);
  return;

response_failure:
  syslog(LOG_WARNING, "Failed to send HTTP response!");
  close(fd);
  return;

server_error:
  if (net_http_send_string_utf8(fd, HTTP_TIMEOUT,
                                txt_HTTP_RESPONSE_500) < 0) {
    syslog(LOG_WARNING, "Failed to send HTTP response!");
  }
  close(fd);
  net_http_cleanup();
  exit(EXIT_FAILURE);
}

void HttpHandler::handle_sobel(const char *method, const char *path, const http_options *options, int fd)
{
  media_stream * stream;
  ssize_t        ret_value = 1;
  const char *   my_tmp_option;
  char           my_media_props[100];
  int            skipframes = 0;
  int            sleeptime  = 0;

  /* handle test stuff */
  my_tmp_option = net_http_option(options, "skipframes");
  if (my_tmp_option) {
    skipframes = atoi(my_tmp_option);
    LOG("%s/%s() skipframes=%d\n",
           __FILE__, __FUNCTION__, skipframes);
  }
  
  my_tmp_option = net_http_option(options, "sleeptime");
  if (my_tmp_option) {
    sleeptime = atoi(my_tmp_option);
    LOG("%s/%s() sleeptime=%d\n",
           __FILE__, __FUNCTION__, sleeptime);
  }

  /* handle fps */
  my_tmp_option = net_http_option(options, "fps");
  if (!my_tmp_option) {
    my_tmp_option = "10";
  }
  
  snprintf(my_media_props,
           sizeof(my_media_props),
           "fps=%s",
           my_tmp_option);

  /* Handle resolution */
  my_tmp_option = net_http_option(options, "resolution");
  if (my_tmp_option) {
    strncat(my_media_props,
            "&resolution=",
            sizeof(my_media_props) - 1);
    strncat(my_media_props,
            my_tmp_option,
            sizeof(my_media_props) - 1);
  } else {
    strncat(my_media_props,
            "&resolution=352x288",
            sizeof(my_media_props) - 1);
  }
  /* put in the format */
  strncat(my_media_props,
          "&sdk_format=Y800",
          sizeof(my_media_props) - 1);
  /* media_props completed */
  LOG("%s media_props=\"%s\"\n",
         __FUNCTION__,
         my_media_props);

  stream = capture_open_stream(IMAGE_UNCOMPRESSED, my_media_props);

  LOG("%s opening stream=%p\n",
         __FUNCTION__,
         stream);

  if (stream) {
    media_frame *frame = NULL;

    ret_value = net_http_send_headers(fd,
                                      HTTP_TIMEOUT,
                                      txt_HTTP_HEADER_200,
                                      "Content-Type: image/x-portable-graymap\r\n",
                                      txt_CRLF,
                                      NULL);
    if (ret_value < 0) {
      goto closefd;
    }
    /* Read a new buffer */
    frame = capture_get_frame(stream);

//     /* Skip frames, just to stress the system */
//     while (skipframes && frame) {
//       capture_frame_free(frame);
//       LOG("%s skipframe\n",
//              __FUNCTION__);
// 
//       sleep(sleeptime);
//       frame = capture_get_frame(stream);
//       skipframes--;
//     }

    /* If buf == NULL: nothing could be read */

    /* This could happen if we are using non-blocking read,
     * or in case of an error. As we are not using non-blocking,
     * treat it as an error */
    if (!frame) {
      ret_value = -1;
      LOG("%s/%s frame = NULL\n",
          __FILE__, __FUNCTION__);
      goto closefd;
    }
    {
      /* set up an image header first */
      int            image_height        = capture_frame_height(frame);
      int            image_width         = capture_frame_width(frame);
      int            image_stride        = capture_frame_stride(frame);
      uint8_t 	     *data               = (uint8_t*)capture_frame_data(frame);
      uint8_t	     *sobel_data		 = NULL;

      const size_t   image_header_length = 100;
      char *         my_image_header     = NULL;
      size_t         content_length      = image_width * image_height;
      
      my_image_header = (char *) malloc(image_header_length + content_length);
      sobel_data = (uint8_t*)malloc(image_stride * image_height);
      
      if (!my_image_header) {
        goto closefd;
      }
      ret_value = snprintf(my_image_header, image_header_length,
                           "P5\n"
                           "# CREATOR: Axis Communications AB\n"
                           "%d %d\n"
                           "%d\n",
                           image_width, image_height, 255);

      if (ret_value > 0) {
        unsigned char *image = (unsigned char *)(my_image_header + ret_value);
        int            j;
        int            i;
	
 	sobel(data, image_width, image_height, image_stride, sobel_data);
	
        for (i = 0; i < image_height; i++) {
          for (j = 0; j < image_width; j++) {
            image[i * image_width + j] = sobel_data[i * image_stride + j];
          }
        }
        content_length += ret_value;
        (void)net_http_send(fd, HTTP_TIMEOUT, my_image_header, content_length);
      }
      
      free(my_image_header);
      free(sobel_data);
      capture_frame_free(frame);
    }
    LOG("%s closing stream=%p",
        __FUNCTION__,
        stream);

    capture_close_stream(stream);
  } else {
    net_http_send_headers(fd,
                          HTTP_TIMEOUT,
                          txt_HTTP_RESPONSE_500,
                          txt_CRLF,
                          NULL);
  }
  
closefd:
  close(fd);
}

int HttpHandler::waitForFastStart(int param_fast)
{
  if (param_fast == DISABLED) {
    syslog(LOG_INFO, "Waiting...");
    net_http_waitfor_and_handle_req(HTTP_WAIT);
  }

  if (param_fast == ENABLED) {
    syslog(LOG_INFO, "Start fast!");
    return FAST_START;
  }

  return -1;
}

int HttpHandler::checkForFastStopOrReconf(int param_fast)
{
  net_http_waitfor_and_handle_req(HTTP_NO_WAIT);
  if (param_fast == DISABLED) {
    syslog(LOG_INFO, "Stop fast!");
    return FAST_STOP;
  } else if (window_changed) {
    window_changed = false;
    syslog(LOG_INFO, "New window parameters!");
    return FAST_RECONF;
  }

  return -1;
}

void sobel(const uint8_t* buf, int width, int height, int stride, uint8_t* new_buf)
{
    int sumX = 0;
    int sumY = 0;
    int sum = 0;
    int GX[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
    int GY[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    for(int y = 0; y < height; y++)  
    {
	for(int x = 0; x < width; x++)  
	{
	     sumX = 0;
	     sumY = 0;

	     if(y == 0 || y == height - 1)
		sum = 0;
	     else if(x == 0 || x == width - 1)
		sum = 0;
	     else
	     {
	         for(int i = -1; i <= 1; i++) 
		 {
		    for(int j = -1; j <= 1; j++)  
		    {
			int piX = j + x;
			int piY = i + y;
	    
			int p = buf[piY * stride + piX];
			
			sumX += p * GX[(i + 1) * 3 + (j +1)];
			sumY += p * GY[(i + 1) * 3 + (j +1)];

	              }
	       }
                sum = abs(sumX) + abs(sumY);
          }
         if(sum > 255)
	    sum = 255;
         if(sum < 0) 
	    sum = 0;
	 
         u_int8_t newPixel = (255 - (sum));

         new_buf[y * stride + x] = newPixel ;
         }
         
   }
}