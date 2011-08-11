#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <net_http.h>
#include <capture.h>

#include "HttpHandler.h"
#include "ParamHandler.h"
#include "CaptureHandler.h"
#include "Shared.h"
#include "SignalHandler.h"

#define HTTP_PATH_MAIN          "/local/fastcpp/main.cgi"
#define HTTP_PATH_STREAM	"/local/fastcpp/streamjpg.cgi"
#define HTTP_PATH_CORNERS	"/local/fastcpp/corners.cgi"

#define HTTP_TIMEOUT    30
#define HTTP_WAIT       -1
#define HTTP_NO_WAIT    0

#define LOG(fmt, args...)    { syslog(LOG_INFO, fmt, ## args); printf(fmt, ## args); }

static bool window_changed = false;

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

//     if (net_http_add_path(HTTP_PATH_MAIN, this->handle_main) < 0) 
//     {
// 	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_MAIN);
// 	net_http_cleanup();
// 	return -1;
//     }
//     
//     syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_MAIN);
    
//     if (net_http_add_path(HTTP_PATH_STREAM, this->handle_streaming_jpg) < 0) 
//     {
// 	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_STREAM);
// 	net_http_cleanup();
// 	return -1;
//     }
//     syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_STREAM);
//     
    if (net_http_add_path(HTTP_PATH_CORNERS, this->handle_corners) < 0) 
    {
	syslog(LOG_CRIT, "Failed to add HTTP path: %s", HTTP_PATH_CORNERS);
	net_http_cleanup();
	return -1;
    }
    syslog(LOG_INFO, "Added http path: %s", HTTP_PATH_CORNERS);

  return 0;
}

void HttpHandler::handle_main(const char *method, const char *path, const http_options *options, int fd)
{
    ssize_t     ret_value  = 1;
    const char *user_agent = NULL;
    
    char my_string[2048];
    const char *txt_local___http_stream_jpg = HTTP_PATH_STREAM;
    
    syslog(LOG_INFO, "Received HTTP Request: %s", path);
     
    /* retrieve the http user agent string */
    user_agent = net_http_option(options, "http_user_agent");
    
     /* send the http headers to the user */
    ret_value = net_http_send_headers(fd, HTTP_TIMEOUT, txt_HTTP_HEADER_200, txt_Content_Type_text_html_utf8, txt_CRLF, NULL);
    if (ret_value > 0)
    {
	/* send a html header and start the html body */
	snprintf(my_string,
		sizeof(my_string),
		"<head>%s</head>"
		"<title>Http Demo App Dynamic Main Page</title>"
		"<html>"
		"<body>"
		"<ul>"
		"your browser %s<br>"
		"</ul>"
		"</body>"
		"</html>",             
		txt_http_equiv_text_html_utf8,
		user_agent);
	net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);
	
	/* send a list of links for multipart jpeg streaming */
	snprintf(my_string, sizeof(my_string), 
	     "<li><a href=\"%s?fps=12&resolution=CIF\"> TEST  (%s)</a></li><br>", txt_local___http_stream_jpg);
	
	net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);
	
	/* send the html footer */
	snprintf(my_string,
		sizeof(my_string),
		"</ul>"
		"</body>"
		"</html>");
	net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);
    }
    close(fd);
}

void HttpHandler::handle_corners(const char *method, const char *path, const http_options *options, int fd)
{
    ssize_t      ret_value  = 1;
    char         my_string[SIZEOF_APP_CORNER_COORD];

    ret_value = net_http_send_headers(fd,
				      HTTP_TIMEOUT,
				      txt_HTTP_HEADER_200,
				      txt_Content_Type_text_html_utf8,
				      txt_CRLF,
				      NULL);


    while(ret_value > 0) 
    {
	snprintf(my_string,
             sizeof(my_string),
             "%s",
             CaptureHandler::get_strfast());
	
	  ret_value = net_http_send_string_utf8(fd, HTTP_TIMEOUT, my_string);	
	
	if ((SignalHandler::getExitSignal()) || (net_http_waitfor_req(0) > 0)) 
	    ret_value = -1; /* end loop */
	
	usleep(100000);
    }
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
