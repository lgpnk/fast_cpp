#include "ThreadHandler.h"

pthread_mutex_t ThreadHandler::m_mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadHandler::m_mutex_b = PTHREAD_MUTEX_INITIALIZER;

bool ThreadHandler::m_is_running = false;
int ThreadHandler::m_nthreads = 0;

void ThreadHandler::add_thread()
{
    if(!m_is_running)
	m_is_running = true;
    m_nthreads++;
}

void ThreadHandler::remove_thread()
{
    if(m_nthreads == 1)
      m_is_running = false;
    m_nthreads--;
}
