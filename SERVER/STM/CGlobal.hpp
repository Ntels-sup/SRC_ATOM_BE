#ifndef __CGLOBAL_HPP__
#define __CGLOBAL_HPP__
#include <string>
#include "CProtocol.hpp"
#include "CFileLog.hpp"
#include "CMergeSession.hpp"
#include "CThreadQueue.hpp"
#include "CMesgExchSocket.hpp"
#include "CLowerInterface.hpp"

using namespace std;

class CGlobal{
    private:
        static CGlobal *m_cInstance;
        CFileLog *m_cLog;
        CLowerInterface *m_cLowerInterface;
        CThreadQueue<CMergeSession*> *m_cMergeSndQueue;
        int m_nLocalProcNo;
        int m_nLocalNodeNo;
        int m_nAlmNo;

        CGlobal();
        ~CGlobal();
    public:
        static CGlobal *GetInstance();
        static int GetCmdCode(std::string &a_strCmdCode){
            if(a_strCmdCode.size() < 10){
                return 0;
            }

            return atoi(&a_strCmdCode.c_str()[4]);
        };
        CFileLog *GetLogP();
        CLowerInterface *GetLowerInterfaceP() { return m_cLowerInterface; };
        void SetMergeSndQueue(CThreadQueue<CMergeSession*> *a_sndQueue) { m_cMergeSndQueue = a_sndQueue; };
        CThreadQueue<CMergeSession*>* GetMergeSndQueueP() { return m_cMergeSndQueue; };

        void SetLocalNodeNo(int a_nNodeNo) { m_nLocalNodeNo = a_nNodeNo; };
        void SetLocalProcNo(int a_nProcNo) { m_nLocalProcNo = a_nProcNo; };
        void SetAlmNo(int a_nAlmNo) { m_nAlmNo = a_nAlmNo; };
        int GetLocalNodeNo() { return m_nLocalNodeNo; };
        int GetLocalProcNo() { return m_nLocalProcNo; };
        int GetAlmNo() { return m_nAlmNo; };
};

#endif

