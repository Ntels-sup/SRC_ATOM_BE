#include "TRM_Define.hpp"
#include "CConfigTRM.hpp"
#include "CConfig.hpp"

//extern
extern CFileLog* g_pcLog;

CConfigTRM& CConfigTRM::Instance(void)
{
	if(_instance == NULL)
	{
		_instance = new CConfigTRM();
	}

	return *_instance;
}

bool CConfigTRM::Loading()
{
	return true;
}


CConfigTRM* CConfigTRM::_instance = NULL;

CConfigTRM::CConfigTRM()
{
	LOG.m_strPkgName = "ATOM";
	LOG.m_strProcName = "TRM";
    LOG.m_strNodeName = "ATOM";
	LOG.m_strNmName = "NM";
    LOG.m_strTrmName = "TRM";
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

//DB
		DB.m_strDbAddr = "127.0.0.1";
		DB.m_nDbPort = 3306;
		DB.m_strDbuserId = "atom";
		DB.m_strDbPasswd = "atom";
		DB.m_strDbName = "ATOM";

// TRM
		TRM.m_nAutoExpireTime = 180; 	// Trace Auto Expire Time 
		TRM.m_nTimeout = 60; 	// Trace Time Out 
		TRM.m_nTraceCnt = 5;	// max 5 user
		TRM.m_strTraceDataPath.assign(getenv("HOME"));
		TRM.m_strTraceDataPath.append("/DATA/TRACE");
	}

}

CConfigTRM::~CConfigTRM()
{
	LOG.m_strModulePath.clear();
	LOG.m_strLogPath.clear();
	LOG.m_strPkgName.clear();
    LOG.m_strProcName.clear();
    LOG.m_strNodeName.clear();
    LOG.m_strNmName.clear();
    LOG.m_strTrmName.clear();
    LOG.m_strWsmName.clear();

	NM.m_nServPort = 0;
	NM.m_strNMAddr.clear();
	NM.m_nNMPort = 0;
	
	DB.m_strDbAddr.clear();
	DB.m_nDbPort = 0;
	DB.m_strDbName.clear();
	DB.m_strDbuserId.clear();
	DB.m_strDbPasswd.clear();

	TRM.m_nTimeout = 0;
	TRM.m_nTraceCnt = 5;
	TRM.m_nAutoExpireTime = 0;
	TRM.m_strTraceDataPath.clear();
}

int CConfigTRM::Initial()
{
	CConfig * conf = new (std::nothrow) CConfig();

	if (conf == NULL)
	{
		if(g_pcLog != NULL)
		{
			g_pcLog->WARNING("CConfigTRM Init Failure");
		}
		return false;
	}
	else
	{
		conf->Initialize();

		std::string	value;
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

// TRM
        value.clear();
        value = conf->GetConfigValue((char *)"TRM", (char *)"TRACE_PATH");
        if(value.size() > 0)
            TRM.m_strTraceDataPath = value;

        value.clear();
        value = conf->GetConfigValue((char *)"TRM", (char *)"TRACE_TIMEOUT");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            TRM.m_nTimeout  = atoi(value.c_str());

        value.clear();
        value = conf->GetConfigValue((char *)"TRM", (char *)"TRACE_COUNT");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            TRM.m_nTraceCnt  = atoi(value.c_str());

        value.clear();
        value = conf->GetConfigValue((char *)"TRM", (char *)"TRACE_AUTOEXPIRE_TIME");
        if(value.size() > 0 && atoi(value.c_str()) > 0)
            TRM.m_nAutoExpireTime  = atoi(value.c_str());

	}

	return true;
}

