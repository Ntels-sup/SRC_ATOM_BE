#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>
#include <iostream>

#include "CProtocol.hpp"
#include "CAtomAPI.hpp"

#include "AlarmTest.hpp"

CFileLog *g_pclsLog = NULL;

ATOM_API * g_pSenderAPI = NULL;

int process_data(char *pData)
{
        g_pclsLog->INFO("Receive Data %s", pData);
            return 0;
}

int process_command(int a_nCmd)
{
        g_pclsLog->INFO("Receive Command %d", a_nCmd);
            return 0;
}



int main(int argc, char* argv[])
{
    uint64_t nLoopCnt = 0;
    char *pData = NULL;
    int nCmd = 0;

    ATOM_API *p_clsAPI = new ATOM_API();

    p_clsAPI->Init(argc, argv);

    // g_pclsLog = p_clsAPI->GetLog();

    // g_pclsLog->SetLogLevel(LV_DEBUG);

    // For Alarm Test
    init_sender_api(argc, argv);
    CSocketServer * server = NULL;

    while(p_clsAPI->m_bRun)
    {
        g_pclsLog = p_clsAPI->GetLog();

        g_pclsLog->SetLogLevel(LV_DEBUG);

        nCmd = p_clsAPI->GetCommand();

        if(nCmd > 0)
            process_command(nCmd);

        pData = NULL;
        p_clsAPI->GetData(&pData, 5);

        if(pData)
        {
            process_data(pData);
            p_clsAPI->ReadComplete();

            p_clsAPI->FreeReadData();

            //읽은 Data 를 그대로 전달할 경우
            //p_clsAPI->SendData();

            //Data 를 새롭게 쓸 경우
            //p_clsAPI->SendData(pData, nDataLen, nIdx);


        }

        // For Alarm Test
        proc_alarm_event(server);

        g_pclsLog->INFO("Test Application is Running, %d", nLoopCnt++);
    }


	return 0;
}
