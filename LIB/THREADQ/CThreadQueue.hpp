#ifndef __CTHREADQUEUE_HPP__
#define __CTHREADQUEUE_HPP__

#include <iostream>
#include <list>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define CTHRD_Q_ERR_STRING_SIZE 1024

template <typename T>
class CThreadQueue{
    private:
        pthread_mutex_t m_mutx;
        pthread_cond_t m_cond;
        std::list<T> m_lstQueue;
        char szErrString[CTHRD_Q_ERR_STRING_SIZE];

    public:
        enum eResult{ 
            CTHRD_Q_OK = 1,
            CTHRD_Q_EMPTY = 2,
            CTHRD_Q_TIMEOUT = 3,
            CTHRD_Q_INVAL = 4,
            CTHRD_Q_UNKNOWN = 5,
        };
        CThreadQueue(); 
        ~CThreadQueue();
        std::string GetErrorStr() {
            std::string strErrString;
            strErrString = szErrString;
            return strErrString;
        }
        void Push(T a_data);
        int Pop(T *a_data);
        int PopWait(T *a_data, struct timespec *a_wait);
};

template <typename T>
CThreadQueue<T>::CThreadQueue()
{
    pthread_cond_init(&m_cond, NULL);
    pthread_mutex_init(&m_mutx, NULL);
}

template <typename T>
CThreadQueue<T>::~CThreadQueue()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutx);
}

template <typename T>
void CThreadQueue<T>::Push(T a_data)
{
    pthread_mutex_lock(&m_mutx);

    m_lstQueue.push_back(a_data); 

    pthread_cond_signal(&m_cond);

    pthread_mutex_unlock(&m_mutx);
}

template <typename T>
int CThreadQueue<T>::Pop(T *a_data)
{
    T data;

    pthread_mutex_lock(&m_mutx);

    if(m_lstQueue.size() == 0){
        pthread_mutex_unlock(&m_mutx);
        return CTHRD_Q_EMPTY;
    }

    data = m_lstQueue.front(); 

    m_lstQueue.pop_front();

    pthread_mutex_unlock(&m_mutx);

    *a_data = data;

    return CTHRD_Q_OK;
}

template <typename T>
int CThreadQueue<T>::PopWait(T *a_data, struct timespec *a_wait = NULL)
{
    int nRet = 0;
    struct timespec delay;
    T data;

    pthread_mutex_lock(&m_mutx);


    if(m_lstQueue.size() == 0){
        if(a_wait == NULL){
            nRet = pthread_cond_wait(&m_cond, &m_mutx);
            if(nRet  != 0){
                snprintf(szErrString,CTHRD_Q_ERR_STRING_SIZE, "Cond wait error(err=%d(%s))\n",
                        nRet , strerror(nRet));
                pthread_mutex_unlock(&m_mutx);
                if(nRet == EINVAL){
                    return CTHRD_Q_INVAL;
                }
                else {
                    return CTHRD_Q_UNKNOWN;
                }
            }
        }
        else {
            clock_gettime(CLOCK_REALTIME, &delay);
            delay.tv_sec += a_wait->tv_sec;
            delay.tv_nsec += a_wait->tv_nsec;

            if(delay.tv_nsec >= 1000000000 /* 1 sec */){
                delay.tv_nsec -= 1000000000; /* 1sec */
                delay.tv_sec += 1;
            }

            nRet = pthread_cond_timedwait(&m_cond, &m_mutx, &delay);
            if(nRet != 0){
                pthread_mutex_unlock(&m_mutx);
                snprintf(szErrString,CTHRD_Q_ERR_STRING_SIZE, "Cond wait error(err=%d(%s))\n",
                        nRet , strerror(nRet));

                if(nRet == ETIMEDOUT){
                    return CTHRD_Q_TIMEOUT;
                }
                else if(nRet == EINVAL){
                    return CTHRD_Q_INVAL;
                }
                else {
                    return CTHRD_Q_UNKNOWN;
                }
            }
        }
    }

    data = m_lstQueue.front(); 

    m_lstQueue.pop_front();

    *a_data = data;

    pthread_mutex_unlock(&m_mutx);

    return CTHRD_Q_OK;

}

#endif

