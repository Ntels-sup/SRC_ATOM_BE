#include "CMergeThread.hpp"

CMergeThread::CMergeThread(CThreadQueue<CMergeSession*> *rcvQueue)
{
    m_cMergeProc = new CMerge(rcvQueue);
}

CMergeThread::~CMergeThread()
{
    delete m_cMergeProc;
}

void CMergeThread::Run()
{
    m_cMergeProc->Run();
}

