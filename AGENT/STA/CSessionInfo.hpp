#ifndef __CSESSIONINFO_HPP__
#define __CSESSIONINFO_HPP__
#include <time.h>

class CSessionInfo{
    private:
        time_t m_receiveTime;
        char *m_chReceiveData;
        int m_nReceiveDataLen;
    public:
        CSessionInfo();
        ~CSessionInfo();
        time_t GetReceiveTime() { return m_receiveTime; };
        int GetReceiveData(char **a_chReceiveData, int *a_nReceiveDataLen);
        void SetReceiveData(char *a_chReceiveData, int a_nReceiveDataLen);
        void SetReceiveTime(time_t a_receiveTime) { m_receiveTime = a_receiveTime; }
};

#endif
