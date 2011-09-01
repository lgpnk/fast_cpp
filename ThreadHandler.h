#ifndef THREADHANDLER_H
#define THREADHANDLER_H
#include <pthread.h>
class ThreadHandler
{
private:
  static pthread_mutex_t m_mutex_a;
  static pthread_mutex_t m_mutex_b;

  static bool m_is_running;
  static int m_nthreads;
  static bool m_stop_request;
public:
  bool is_running(){return m_is_running;}
  bool stop_thread_request(){m_stop_request = true;}
  
  pthread_mutex_t* get_mutex_a(){return &m_mutex_a;}
  pthread_mutex_t* get_mutex_b(){return &m_mutex_b;}
  
  bool getStopRequest(){return m_stop_request;}
  
  static void add_thread();
  static void remove_thread();


};

#endif // THREADHANDLER_H
