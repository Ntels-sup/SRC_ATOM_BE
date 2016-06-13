#include "CThread.hpp" 

CThread::CThread () 
{
    this->threadRunFlag = false;
}
CThread::~CThread ()
{
    if(this->IsRun())
    {
        this->Stop();
    }
}
 
bool CThread::IsRun()
{
    return this->threadRunFlag;
}
 
void* CThread::Run_ (void* aThis) 
{
    CThread *pThis = (CThread *)aThis;
    while( pThis->IsRun())
    {
        pThis->Run();
    }
    return 0;
}

void CThread::Start ()
{ 
    this->threadRunFlag = true;
    pthread_create (&mThread, NULL,&CThread::Run_, (void*)this);
}
 
void CThread::Stop ()
{
    this->threadRunFlag = false;
    pthread_join (mThread, NULL);
}

