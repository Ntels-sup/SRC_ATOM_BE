#include <stdio.h>
#include <stdlib.h>

#include "CProtocol.hpp"
#include "CAtomAPI.hpp"

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

int main (int argc, char *args[])
{
	uint64_t nLoopCnt = 0;
	char *pData = NULL;
	int nRet = 0;
	int nCmd = 0;

	ATOM_API *p_clsAPI = new ATOM_API();

	nRet = p_clsAPI->Init(argc, args);

	g_pclsLog = p_clsAPI->GetLog();

	g_pclsLog->SetLogLevel(LV_DEBUG);

	printf("333333333333 %d %d %s\n", p_clsAPI->IsRun(), nRet, p_clsAPI->GetErrorMsg());

	while(p_clsAPI->IsRun())
	{
		nCmd = p_clsAPI->GetCommand();

		if(nCmd > 0)
			process_command(nCmd);
		else
			g_pclsLog->ERROR("%s", p_clsAPI->GetErrorMsg());

		pData = NULL;

		if(p_clsAPI->ReadWait(5) < 0)
			continue;

		p_clsAPI->GetData(&pData);
		if(pData)
		{
			process_data(pData);
			p_clsAPI->ReadComplete();

			//Data 처리가 완전히 완료 된 경우에만 호출
			p_clsAPI->FreeReadData();

			//읽은 Data 를 그대로 전달할 경우			
			//p_clsAPI->SendData();

			//Data 를 새롭게 쓸 경우
			//p_clsAPI->SendData(pData, nDataLen, nIdx);
			
		}


	
		g_pclsLog->INFO("Test Application Is Running, %d", nLoopCnt++);	
	}
	printf("4444444444\n");

	return 0;
}
