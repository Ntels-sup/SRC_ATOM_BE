
#include <cstdio>
#include <string>

#include "AppForAlarm.hpp"
#include "CFileLog.hpp"
#include "CModule.hpp"
// #include "CModuleIPC.hpp"


CFileLog * gLog = NULL;

bool initLog()
{

	/*--
    std::string strLogPath = "/UDBS/ATOM/LOG";

    // Log init
    gLog = new CFileLog();
    if (gLog->Initialize((char*)strLogPath.c_str(),
                                        "ATOM", (char*)"SIM", -1, LV_DEBUG) != 0) {
        printf("Log initalize failed");
        return false;
    }

    gLog->INFO("AppSim module starting.");
	--*/

    return true;
}

bool init(const char * _path, const char * _fname)
{
	int 	ret;
	gLog = new (std::nothrow) CFileLog(&ret);

	if(ret < 0)
	{
		delete gLog;
		gLog = NULL;
		return false;
	}

	gLog->Initialize(_path,
                     "",
                     _fname,
                     -1,
                     LV_DEBUG);

	return true;
}

void * AppSim(void * pArg)
{
    CModule::ST_MODULE_OPTIONS stOption =
        *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);

	CConfig * conf = new (std::nothrow) CConfig();

	if(conf == NULL)
	   return (void *)NULL;	

	if(conf->Initialize(stOption.m_szCfgFile) < 0)
	{
		delete conf;
		return (void *)NULL;
	}

	const char * p = conf->GetConfigValue((char *)"GLOBAL", (char *)"LOG_PATH");

    if(init(p, "SIM") == false)
    {
        delete conf;
        conf = NULL;

        return false;
    }

	delete conf;

    AppForAlarm     afa;

    if(afa.Init() != true)
    {
        gLog->ERROR("can't Init for AppForAlarm");
        return (void *)NULL;
    }

    gLog->INFO("AppSim module starting.");

    if(afa.SetIF(stOption.m_pclsModIpc, stOption.m_pclsAddress) == false)
    {
        gLog->ERROR("can't Setup for interface");
        return (void *)NULL;
    }


    while(true)
    {
        if(afa.IsEmpty() == true)
            continue;

        gLog->DEBUG("Received MSG");
        afa.Proc(afa.GetBody());
    }
}

#include "CModule.hpp"

MODULE_OP = {
    "ATOM_NA_STA",      // process name
    true,               // thread running
    initLog,               // Init
    AppSim,             // Process
    NULL                // CleanUp
};


