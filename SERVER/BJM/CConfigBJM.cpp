#include "BJM_Define.hpp"
#include "CConfigBJM.hpp"
#include "CConfig.hpp"

//extern
extern CFileLog* g_pcLog;

CConfigBJM& CConfigBJM::Instance(void)
{
	if(_instance == NULL)
	{
		_instance = new CConfigBJM();
	}

	return *_instance;
}

bool CConfigBJM::Loading()
{
	return BJM_OK;
}


CConfigBJM* CConfigBJM::_instance = NULL;

CConfigBJM::CConfigBJM()
{
	LOG.m_strPkgName = "ATOM";
	LOG.m_strProcName = "BJM";
	LOG.m_strNodeName = "ATOM";
	LOG.m_strNmName = "NM";
	LOG.m_strBjmName = "BJM";
	LOG.m_strWsmName = "WSM";
	
	LOG.m_nDuplCnt = 24;
	LOG.m_nLevel = LV_INFO;

	if(Initial() == false)
	{
// LOG
		LOG.m_strModulePath.assign(getenv("HOME"));
		LOG.m_strModulePath.append("/BIN");
		LOG.m_strLogPath.assign(getenv("HOME"));
		LOG.m_strLogPath.append("/LOG");

//NM
		NM.m_strNMAddr = "127.0.0.1";
		NM.m_nNMPort = 8000;	// NM
//		NM.m_nNMPort = 10000;	// simul

//DB
		DB.m_strDbAddr = "127.0.0.1";
		DB.m_nDbPort = 3306;
		DB.m_strDbuserId = "atom";
		DB.m_strDbPasswd = "atom";
		DB.m_strDbName = "ATOM";
	
		BJM.m_nTimecount = 60;
	}
}

CConfigBJM::~CConfigBJM()
{
	LOG.m_strModulePath.clear();
	LOG.m_strLogPath.clear();
	LOG.m_strPkgName.clear();
	LOG.m_strProcName.clear();
	LOG.m_strNodeName.clear();
	LOG.m_strNmName.clear();
	LOG.m_strBjmName.clear();
	LOG.m_strWsmName.clear();

	NM.m_nServPort = 0;
	NM.m_strNMAddr.clear();
	NM.m_nNMPort = 0;
	
	DB.m_strDbAddr.clear();
	DB.m_nDbPort = 0;
	DB.m_strDbName.clear();
	DB.m_strDbuserId.clear();
	DB.m_strDbPasswd.clear();

	BJM.m_nTimecount = 0;
}

int CConfigBJM::Initial()
{
    CConfig * conf = new (std::nothrow) CConfig();
    if (conf == NULL)
    {
        if(g_pcLog != NULL)
		{
            g_pcLog->ERROR("CConfigTRM Init Failure");
		}
		return false;
    }
    else
    {
        std::string     value;

        conf->Initialize();
// LOG
        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"LOG_PATH");
        if(value.size() > 0 && value != LOG.m_strLogPath)
            LOG.m_strLogPath = value;
// DB
        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_HOST");
        if(value.size() > 0)
            DB.m_strDbAddr  = value;

        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_PORT");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            DB.m_nDbPort  = atoi(value.c_str());

        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_USER");
        if(value.size() > 0)
            DB.m_strDbuserId  = value;

        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_PASS");
        if(value.size() > 0)
            DB.m_strDbPasswd  = value;

        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_DATABASE");
        if(value.size() > 0)
            DB.m_strDbName  = value;

// NM
        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"ATOM_SERVER_IP");
        if(value.size() > 0)
            NM.m_strNMAddr  = value;

        value.clear();
        value = conf->GetConfigValue((char *)"GLOBAL", (char *)"ATOM_SERVER_PORT");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            NM.m_nNMPort  = atoi(value.c_str());

        value.clear();
        value = conf->GetConfigValue((char *)"BJM", (char *)"BATCH_TIMECOUNT");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            BJM.m_nTimecount  = atoi(value.c_str());
    }

    return true;
}

