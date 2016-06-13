#ifndef __CMERGETHREAD_HPP__
#define __CMERGETHREAD_HPP__

#include "CThread.hpp"
#include "CThreadQueue.hpp"
#include "CMerge.hpp"

class CMergeThread : public CThread{
    private:
        CMerge *m_cMergeProc;

    public:
        CMergeThread(CThreadQueue<CMergeSession*> *rcvQueue);
        ~CMergeThread();
        void Run();
};

#endif
