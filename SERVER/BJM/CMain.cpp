#include <iostream>
#include <string>
#include <cstdio>
#include "CMain.hpp"
#include "CConfigBJM.hpp"
#include "CConfig.hpp"
#include "CMesgExchSocket.hpp"
#include "MariaDB.hpp"
#include "FetchData.hpp"

using namespace std;

CConfigBJM g_pcCFG;
CNMSession g_pcNM;
CFileLog *g_pcLog = NULL;

CMain::CMain()
{
	printf("Init\n");
	m_cscheduler = NULL;
	m_cnmsession = NULL;

	if(init() != true) 
	{
		printf("BJM Initialize ERROR \n");
	}
}

CMain::~CMain()
{
	delete m_cscheduler;
	m_cscheduler = NULL;

	delete m_cnmsession;
	m_cnmsession = NULL;

	delete g_pcLog;
	g_pcLog = NULL;
}

int CMain::init()
{
	int  		ret = 0;

	// initialize Config 
	CConfigBJM& cfg = CConfigBJM::Instance();

    // Log init
    g_pcLog = new CFileLog(&ret);
	if(ret < 0)
	{
        printf("Failed to Create Log Class\n");

        if(g_pcLog)
            delete g_pcLog;

        return BJM_NOK;		
	}

	CConfig *p_clsConfig = NULL;
	p_clsConfig = new CConfig();

    if (g_pcLog->Initialize((char*)cfg.LOG.m_strLogPath.c_str(), NULL, (char*)cfg.LOG.m_strProcName.c_str(), cfg.LOG.m_nDuplCnt, LV_INFO) != 0)
	{
        g_pcLog->INFO("Log initalize failed");
        return false;
    }
    g_pcLog->INFO("Log Initaialization OK");

	return true;
}

int CMain::Final()
{
	return BJM_OK;
}

int CMain::Run()
{
	m_cscheduler = new CScheduler();

	if(m_cscheduler->Run() == BJM_NOK)
		return false;

	// Communication with NM
	m_cnmsession = new CNMSession();
	if(m_cnmsession->Run() == BJM_NOK)
		return false;

	return true;
}

int main(int argc, char *args[])
{
	CMain *cBjmMain = NULL;

	cBjmMain = new CMain;
	
	if(cBjmMain->Run() != true)
	{
		printf("ERROR \n");
		return BJM_NOK;
	}

	cBjmMain = NULL;
	delete cBjmMain;

	return BJM_OK;

}
