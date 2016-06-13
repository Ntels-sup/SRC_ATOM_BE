#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <ctime>
#include <iostream>

#include "CAtomAPI.hpp"
#include "CProtocol.hpp"

CFileLog *g_pclsLog = NULL;

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
	fprintf(stderr, "STARTUP\n");

	int ret = 0;
    uint64_t nLoopCnt = 0;
    char *pData = NULL;
    int nCmd = 0;

    ATOM_API *p_clsAPI = new ATOM_API();

    ret = p_clsAPI->Init(argc, argv);
	if(ret < 0)
	{
		printf("API Init Error\n");
		return -1;
	}

    g_pclsLog = p_clsAPI->GetLog();

    g_pclsLog->SetLogLevel(LV_DEBUG);

    while(p_clsAPI->m_bRun)
    {
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

        g_pclsLog->INFO("Test Application is Running, %d", nLoopCnt++);
    }


	return 0;
}
