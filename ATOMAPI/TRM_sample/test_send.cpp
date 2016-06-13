#include <stdio.h>
#include <stdlib.h>

#include "CProtocol.hpp"
#include "CAtomAPI.hpp"

CFileLog *g_pclsLog = NULL;

int process_data(char *pData)
{
	g_pclsLog->INFO("Receive Data");
	return 0;
}

int process_command(int a_nCmd)
{
	g_pclsLog->INFO("Receive Command %d", a_nCmd);
	return 0;
}

int main (int argc, char *args[])
{
	int nRet = 0;
	int nCmd = 0;

	ATOM_API *p_clsAPI = new ATOM_API();

	p_clsAPI->Init(argc, args, LV_DEBUG, 0, DEF_CMD_TYPE_SEND);

	g_pclsLog = p_clsAPI->GetLog();

	nRet = p_clsAPI->SendData((char*)"Test Data", strlen("Test Data"));

	if(nRet < 0)
	{
		g_pclsLog->ERROR("%s", p_clsAPI->GetErrorMsg());
		return -1;
	}


	rabbit::object o_root;
	rabbit::object o_body = o_root["BODY"];
	o_body["action"] = "STOP";

	CProtocol clsProtocol;
	
	clsProtocol.Clear();
	clsProtocol.SetFlagRequest();
	clsProtocol.SetSequence(0);
	clsProtocol.SetCommand("0000099990");
	clsProtocol.SetSource(p_clsAPI->GetNodeNo(), p_clsAPI->GetProcNo());
	clsProtocol.SetDestination(p_clsAPI->GetNodeNo(), p_clsAPI->GetProcNo());
	clsProtocol.SetPayload(o_root.str().c_str());

//	if(p_clsAPI->SendCommand(&clsProtocol, (char*)"TEST02", 0) < 0)
//		g_pclsLog->ERROR("%s", p_clsAPI->GetErrorMsg());

	while(1)
	{
		nCmd = p_clsAPI->GetCommand();

		if(nCmd > 0)
		{
			process_command(nCmd);
			break;
		}
		sleep(1);
	}

//	g_pclsLog->INFO("Stop Sender");	
	g_pclsLog->MessageLog(LV_DEBUG, "Stop Sender");	

	return 0;
}
