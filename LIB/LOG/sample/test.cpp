#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "CFileLog.hpp"

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
	p_clsLog->Initialize(NULL, "GTP", "DIA01", -1, LV_INFO);

	p_clsLog->INFO("Test Log nice to meet you");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "nice");
	p_clsLog->INFO("Test Log %s", "eeeee");

	p_clsLog->INFO("Cur Log Level %d", p_clsLog->GetLogLevel());
	p_clsLog->SetLogLevel(LV_DEBUG);
	p_clsLog->INFO("After Log Level %d", p_clsLog->GetLogLevel());

	p_clsLog->DEBUG("3333333333333");

	if(p_clsLog)
		delete p_clsLog;

	return 0;
}
