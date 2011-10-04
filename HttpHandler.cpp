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

#define HTTP_PATH_MOD		"/local/fastcpp/mod.cgi"
#define HTTP_PATH_SOBEL		"/local/fastcpp/sobel.cgi"
#define HTTP_PATH_UPDATE	"/local/fastcpp/update.cgi"

#define HTTP_TIMEOUT    30
#define HTTP_WAIT       -1
#define HTTP_NO_WAIT    0

#define LOG(fmt, args...)    { syslog(LOG_INFO, fmt, ## args); printf(fmt, ## args); }

static bool resolution_changed = false;
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
    if (net_http_add_path(HTTP_PATH_MOD, this->handle_algorithm_mod) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_MOD);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_MOD);
    
//     if (net_http_add_path(HTTP_PATH_SOBEL, this->handle_sobel) < 0) 
//     {
// 	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_SOBEL);
// 	net_http_cleanup();
// 	return -1;
//     }
//     syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_SOBEL);
    
    if (net_http_add_path(HTTP_PATH_UPDATE, this->handle_update) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_UPDATE);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_UPDATE);
    
  return 0;
}


void HttpHandler::handle_algorithm_mod(const char *method, const char *path, const http_options *options, int fd)
{
    ssize_t      ret_value  = 1;
    char * value;
    const char* mod;
    pthread_t thread;
    Arg *arg = new Arg();
    
    syslog(LOG_INFO, "Received HTTP Request: %s", path);

    if (!(mod = net_http_option(options, "mod"))) 
    {
        syslog(LOG_WARNING, "Bad Request!");
	close(fd);
	return;
    }
    
    if(string(mod) == "corner")
      arg->mod = 0;
    else if (string(mod) == "sobel")
      arg->mod = 1;
    else if (string(mod) == "both")
      arg->mod = 2;
    
    arg->threadHandler = threadHandler;
    arg->fd = fd;

    param_get(PARAM_ENABLED, &value);
    
        ret_value = net_http_send_headers(fd,
				  HTTP_TIMEOUT,
				  txt_HTTP_HEADER_200,
				  txt_CONTENT_TYPE_MULTIPART,
				  txt_CRLF,
				  NULL);

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
    const char* fast;
    
    ThreadHandler *threadHandler = arg->threadHandler;
    
    syslog(LOG_CRIT, "Start HTTP Thread");
    switch(arg->mod)
    {
      case 0:
      {
	  char my_string_corner[SIZEOF_APP_CORNER_COORD];
	  if (param_set(PARAM_FAST, "yes", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_FAST, "yes");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  while(ret_value > 0) 
	  {
	      pthread_mutex_lock(threadHandler->get_mutex_b());
	      snprintf(my_string_corner,
		  sizeof(my_string_corner),
		  "%s",
		  CaptureHandler::get_strfast());
	      
	      int len = strlen(my_string_corner);
	      
	      while (len < SIZEOF_PKG)
		len += sprintf(my_string_corner + len, " ");
	      len+= sprintf(my_string_corner + len, "EOF");
		      ret_value = net_http_send_multipart_content(fd,
							  HTTP_TIMEOUT,
							  txt_text_html_utf8,
							  my_string_corner,
							  strlen(my_string_corner));
      // 	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);	
      //  	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, "EOF");	
      // 	pthread_mutex_unlock(threadHandler->get_mutex_a());
	      if ((SignalHandler::getExitSignal()) || threadHandler->getStopRequest()) 
		  ret_value = -1; /* end loop */
		  
      // 	net_http_waitfor_and_handle_req(0);
	  }
	  if (param_set(PARAM_FAST, "no", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_FAST, "no");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  close(fd);
	  threadHandler->remove_thread();
	  syslog(LOG_CRIT, "Close HTTP Thread");
	  pthread_exit(NULL);
      }
      break;
      case 1:
      {
	  char my_string_edge[SIZEOF_APP_SOBEL];
	  if (param_set(PARAM_SOBEL, "yes", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SOBEL, "yes");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  while(ret_value > 0) 
	  {
	      pthread_mutex_lock(threadHandler->get_mutex_b());
	      snprintf(my_string_edge,
		  sizeof(my_string_edge),
		  "%s",
		  CaptureHandler::get_strsobel());
	      
	      int len = strlen(my_string_edge);
	      
// 	      while (len < SIZEOF_PKG)
// 		len += sprintf(my_string_edge + len, " ");
	      len+= sprintf(my_string_edge + len, "EOH");
		      ret_value = net_http_send_multipart_content(fd,
							  HTTP_TIMEOUT,
							  txt_text_html_utf8,
							  my_string_edge,
							  strlen(my_string_edge));
      // 	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);	
      //  	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, "EOF");	
      // 	pthread_mutex_unlock(threadHandler->get_mutex_a());
	      if ((SignalHandler::getExitSignal()) || threadHandler->getStopRequest()) 
		  ret_value = -1; /* end loop */
		  
      // 	net_http_waitfor_and_handle_req(0);
	  }
	  if (param_set(PARAM_SOBEL, "no", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SOBEL, "no");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  close(fd);
	  threadHandler->remove_thread();
	  syslog(LOG_CRIT, "Close HTTP Thread");
	  pthread_exit(NULL);

      }
      break;
      case 2:
      {
	  char my_string_ec[SIZEOF_APP_SOBEL + SIZEOF_APP_CORNER_COORD];
	  if (param_set(PARAM_FAST, "yes", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_FAST, "yes");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }	  
	  if (param_set(PARAM_SOBEL, "yes", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SOBEL, "yes");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  while(ret_value > 0) 
	  {
	      pthread_mutex_lock(threadHandler->get_mutex_b());
	      snprintf(my_string_ec,
		  sizeof(my_string_ec),
		  "%sEOS%s",
		  CaptureHandler::get_strsobel(),
		  CaptureHandler::get_strfast());
      
	      int len = strlen(my_string_ec);
	      
	      while (len < SIZEOF_PKG)
		len += sprintf(my_string_ec + len, " ");
	      len += sprintf(my_string_ec + len, "EOF");
		      ret_value = net_http_send_multipart_content(fd,
							  HTTP_TIMEOUT,
							  txt_text_html_utf8,
							  my_string_ec,
							  strlen(my_string_ec));
      // 	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);	
      //  	ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, "EOF");	
      // 	pthread_mutex_unlock(threadHandler->get_mutex_a());
	      if ((SignalHandler::getExitSignal()) || threadHandler->getStopRequest()) 
		  ret_value = -1; /* end loop */
		  
      // 	net_http_waitfor_and_handle_req(0);
	  }
	  if (param_set(PARAM_FAST, "no", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_FAST, "no");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  if (param_set(PARAM_SOBEL, "no", 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SOBEL, "no");
	      close(fd);
	      net_http_cleanup();
	      exit(EXIT_FAILURE);
	  }
	  close(fd);
	  threadHandler->remove_thread();
	  syslog(LOG_CRIT, "Close HTTP Thread");
	  pthread_exit(NULL);
      }
      break;
    }
}

void HttpHandler::handle_update(const char* method, const char* path, const http_options* options, int fd)
{
  const char *action       = NULL;
  const char *fast_level   = NULL;
  const char *fast_enabled = NULL;
  const char *suppression  = NULL;
  const char *http_thread  = NULL;
  const char *resolution   = NULL;
  const char *param 	   = NULL;
  const char *threshold_level   = NULL;
  const char *sobel_operation = NULL;
  char	     *param_value;

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
      else if ((resolution = net_http_option(options, "res"))) 
      {
	  if (param_set(PARAM_RES, resolution, 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_RES, resolution);
	    goto server_error;
	  }
	  resolution_changed = true;
      }
      else if ((threshold_level = net_http_option(options, "tlevel"))) 
      {
	  if (param_set(PARAM_THRESHOLD_LEVEL, threshold_level, 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_THRESHOLD_LEVEL, threshold_level);
	    goto server_error;
	  }
      }
      else if ((sobel_operation = net_http_option(options, "operation"))) 
      {
	  if (param_set(PARAM_SOBEL_OPERATION, sobel_operation, 1) < 0) 
	  {
	      syslog(LOG_CRIT, "Failed to set parameter: %s to: %s", PARAM_SOBEL_OPERATION, sobel_operation);
	    goto server_error;
	  }
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
else if(string(action) == "get")
{
      if ((param = net_http_option(options, "param")))
      {
	  net_http_send_headers(fd,
				  HTTP_TIMEOUT,
				  txt_HTTP_HEADER_200,
				  txt_Content_Type_text_html_utf8,
				  txt_CRLF,
				  NULL);

	  if(string(param) == "res")
	  {
		param_get(PARAM_RES, &param_value);
		net_http_send_string_utf8(fd, HTTP_TIMEOUT, param_value);
		param_free(param_value);
	  }
	  else if(string(param) == "sup")
	  {
		param_get(PARAM_SUPPRESSION, &param_value);
		net_http_send_string_utf8(fd, HTTP_TIMEOUT, param_value);
		param_free(param_value);
	  }
	  else if(string(param) == "lev")
	  {
		param_get(PARAM_LEVEL, &param_value);
		net_http_send_string_utf8(fd, HTTP_TIMEOUT, param_value);
		param_free(param_value);
	  }
      }
    close(fd);
    return;
}
//   else if(string(action) == "get") 
//   {
// // 	  param_get(PARAM_RES, &res_get);
// // 	  net_http_send_headers(fd,
// // 				  HTTP_TIMEOUT,
// // 				  txt_HTTP_HEADER_200,
// // 				  txt_Content_Type_text_html_utf8,
// // 				  txt_CRLF,
// // 				  NULL);
// // 	  net_http_send_string_utf8(fd, HTTP_TIMEOUT, res_get);
// // 	  syslog(LOG_INFO, "RES = %s", res_get);
// // 	  param_free(res_get);
//     if (net_http_send_headers(fd,
//                               HTTP_TIMEOUT,
//                               txt_HTTP_HEADER_200,
//                               txt_Content_Type_text_html_utf8,
//                               txt_CRLF,
//                               NULL) < 0) {
//       goto response_failure;
//     }
//   }
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
    ssize_t      ret_value  = 1;
    char * value;
    pthread_t thread;
    Arg *arg = new Arg();
    
    arg->threadHandler = threadHandler;
    arg->fd = fd;

    param_get(PARAM_ENABLED, &value);
    
        ret_value = net_http_send_headers(fd,
				  HTTP_TIMEOUT,
				  txt_HTTP_HEADER_200,
				  txt_CONTENT_TYPE_MULTIPART,
				  txt_CRLF,
				  NULL);

    if(ret_value > 0 && string(value) == "yes")
    {
	threadHandler->add_thread();
	pthread_create(&thread, NULL, start_thread, (void*)arg);
    }
    else
      close(fd);
    
    param_free(value);
}
int HttpHandler::waitForFastStart(int param_app)
{
  if (param_app == DISABLED) {
    syslog(LOG_INFO, "Waiting...");
    net_http_waitfor_and_handle_req(HTTP_WAIT);
  }

  if (param_app == ENABLED) {
    syslog(LOG_INFO, "Start fast!");
    return FAST_START;
  }

  return -1;
}

int HttpHandler::checkForFastStopOrReconf(int param_app)
{
  net_http_waitfor_and_handle_req(HTTP_NO_WAIT);
  if (param_app == DISABLED) {
    syslog(LOG_INFO, "Stop fast!");
    return FAST_STOP;
  } else if (resolution_changed) {
    resolution_changed = false;
    syslog(LOG_INFO, "New window parameters!");
    return FAST_RECONF;
  }

  return -1;
}

// void HttpHandler::handle_sobel(const char *method, const char *path, const http_options *options, int fd)
// {
//   media_stream * stream;
//   ssize_t        ret_value = 1;
//   const char *   my_tmp_option;
//   char           my_media_props[100];
//   int            skipframes = 0;
//   int            sleeptime  = 0;
// 
//   /* handle test stuff */
//   my_tmp_option = net_http_option(options, "skipframes");
//   if (my_tmp_option) {
//     skipframes = atoi(my_tmp_option);
//     LOG("%s/%s() skipframes=%d\n",
//            __FILE__, __FUNCTION__, skipframes);
//   }
//   
//   my_tmp_option = net_http_option(options, "sleeptime");
//   if (my_tmp_option) {
//     sleeptime = atoi(my_tmp_option);
//     LOG("%s/%s() sleeptime=%d\n",
//            __FILE__, __FUNCTION__, sleeptime);
//   }
// 
//   /* handle fps */
//   my_tmp_option = net_http_option(options, "fps");
//   if (!my_tmp_option) {
//     my_tmp_option = "10";
//   }
//   
//   snprintf(my_media_props,
//            sizeof(my_media_props),
//            "fps=%s",
//            my_tmp_option);
// 
//   /* Handle resolution */
//   my_tmp_option = net_http_option(options, "resolution");
//   if (my_tmp_option) {
//     strncat(my_media_props,
//             "&resolution=",
//             sizeof(my_media_props) - 1);
//     strncat(my_media_props,
//             my_tmp_option,
//             sizeof(my_media_props) - 1);
//   } else {
//     strncat(my_media_props,
//             "&resolution=352x288",
//             sizeof(my_media_props) - 1);
//   }
//   /* put in the format */
//   strncat(my_media_props,
//           "&sdk_format=Y800",
//           sizeof(my_media_props) - 1);
//   /* media_props completed */
//   LOG("%s media_props=\"%s\"\n",
//          __FUNCTION__,
//          my_media_props);
// 
//   stream = capture_open_stream(IMAGE_UNCOMPRESSED, my_media_props);
// 
//   LOG("%s opening stream=%p\n",
//          __FUNCTION__,
//          stream);
// 
//   if (stream) {
//     media_frame *frame = NULL;
// 
//     ret_value = net_http_send_headers(fd,
//                                       HTTP_TIMEOUT,
//                                       txt_HTTP_HEADER_200,
//                                       "Content-Type: image/x-portable-graymap\r\n",
//                                       txt_CRLF,
//                                       NULL);
//     if (ret_value < 0) {
//       goto closefd;
//     }
//     /* Read a new buffer */
//     frame = capture_get_frame(stream);
// 
// //     /* Skip frames, just to stress the system */
// //     while (skipframes && frame) {
// //       capture_frame_free(frame);
// //       LOG("%s skipframe\n",
// //              __FUNCTION__);
// // 
// //       sleep(sleeptime);
// //       frame = capture_get_frame(stream);
// //       skipframes--;
// //     }
// 
//     /* If buf == NULL: nothing could be read */
// 
//     /* This could happen if we are using non-blocking read,
//      * or in case of an error. As we are not using non-blocking,
//      * treat it as an error */
//     if (!frame) {
//       ret_value = -1;
//       LOG("%s/%s frame = NULL\n",
//           __FILE__, __FUNCTION__);
//       goto closefd;
//     }
//     {
//       /* set up an image header first */
//       int            image_height        = capture_frame_height(frame);
//       int            image_width         = capture_frame_width(frame);
//       int            image_stride        = capture_frame_stride(frame);
//       uint8_t 	     *data               = (uint8_t*)capture_frame_data(frame);
//       uint8_t	     *sobel_data	 = NULL;
// 
//       const size_t   image_header_length = 100;
//       char *         my_image_header     = NULL;
//       size_t         content_length      = image_stride * image_height * 4;
//       
//       my_image_header = (char *) malloc(image_header_length + content_length);
//       sobel_data = (uint8_t*)malloc(image_stride * image_height);
//       int pos = 0;
//       if (!my_image_header) {
//         goto closefd;
//       }
// //       ret_value = snprintf(my_image_header, image_header_length,
// //                            "P5\n"
// //                            "# CREATOR: Axis Communications AB\n"
// //                            "%d %d\n"
// //                            "%d\n"
// // 			   "EOH",
// //                            image_width, image_height, 255);
// 
//     /*  if (ret_value > 0) */{
//         unsigned char *image = (unsigned char *)(my_image_header);
//         int            j;
//         int            i;
// 	
//  	sobel(data, image_width, image_height, image_stride, sobel_data);
// 	
//         for (i = 0; i < image_height; i++) {
//           for (j = 0; j < image_width; j++) {
// 	    pos += sprintf(my_image_header + pos, "%d;", sobel_data[i * image_stride+ j]);
//           }
//         }
//         content_length += ret_value;
//         (void)net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_image_header);
//       }
//       
//       free(my_image_header);
//       free(sobel_data);
//       capture_frame_free(frame);
//     }
//     LOG("%s closing stream=%p",
//         __FUNCTION__,
//         stream);
// 
//     capture_close_stream(stream);
//   } else {
//     net_http_send_headers(fd,
//                           HTTP_TIMEOUT,
//                           txt_HTTP_RESPONSE_500,
//                           txt_CRLF,
//                           NULL);
//   }
//   
// closefd:
//   close(fd);
// }