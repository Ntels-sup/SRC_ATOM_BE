#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "CFileLog.hpp"
#include "CConfig.hpp"

int main(int argc, char *args[])
{
	int ret = 0;

	CFileLog *p_clsLog = NULL;
	p_clsLog = new CFileLog(&ret);

	if(ret < 0)
	{
		printf("Failed to Create Log Class\n");

		if(p_clsLog)
			delete p_clsLog;

		return -1;
	}

	//Base Log Path, Service Name, Process Name, Log Dup Check Time, Log Level 
	//If Base Log Path == NULL, Log print out on the std
	//Log Path ==> BaseLogPath/ServiceName/ProcessName.YYYYMMDD
	p_clsLog->Initialize(NULL, NULL, "TEST_CONFIG", -1, LV_INFO);


	CConfig *p_clsConfig = NULL;
	p_clsConfig = new CConfig();
//	p_clsConfig->Initialize("./ATOM.cfg");
	p_clsConfig->Initialize();
	
	p_clsLog->INFO("TEST4 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST4"));
	p_clsLog->INFO("TEST5 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST5"));
	p_clsLog->INFO("TEST3 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST3"));
	p_clsLog->INFO("Global TEST1 : %s", p_clsConfig->GetGlobalConfigValue((char*)"TEST1"));

	p_clsConfig->DumpConfig();

#if 0
	ret = p_clsConfig->InsertConfigValue("TEST_GROUP", "TEST1", "1");
	ret = p_clsConfig->InsertConfigValue("TEST_GROUP", "TEST2", "2");
	ret = p_clsConfig->InsertConfigValue("TEST_GROUP", "TEST3", "3");
	ret = p_clsConfig->InsertConfigValue("TEST_GROUP", "TEST4", "4");
#endif
//	ret = p_clsConfig->UpdateConfigValue("TEST_GROUP", "TEST2", "5");
//	p_clsConfig->DumpConfig();
//	ret = p_clsConfig->ReWriteConfig();

	if(p_clsConfig)
		delete p_clsConfig;

#if 0
	p_clsConfig = new CConfig();
	p_clsConfig->Initialize("./ATOM.cfg");
	
	p_clsLog->INFO("TEST4 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST4"));
	p_clsLog->INFO("TEST5 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST5"));
	p_clsLog->INFO("TEST3 : %s", p_clsConfig->GetConfigValue((char*)"RSA", (char*)"TEST3"));
	p_clsLog->INFO("Global TEST1 : %s", p_clsConfig->GetGlobalConfigValue((char*)"TEST1"));

	if(p_clsConfig)
		delete p_clsConfig;

#endif
	if(p_clsLog)
		delete p_clsLog;

	return 0;
}
