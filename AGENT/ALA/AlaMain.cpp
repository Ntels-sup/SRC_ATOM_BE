#include <stdio.h>
#include <string>

#include "AlarmAgent.hpp"
#include "CFileLog.hpp"
#include "AlaCFG.hpp"

CFileLog   *   gAlmLog      = NULL;
AlarmAgent *   gAlarmAgent  = NULL;
AlaCFG         gAlaCFG;

void initAlaLog()
{
    // 부르지 않습니다.
    //
    std::string     path(getenv("HOME"));
    path += "/LOG";
    std::string     fname = "ALA";

    gAlaCFG.InitLog(path, fname, false);
}

void finalAlaLog()
{
    if(gAlmLog != NULL)
    {
        delete  gAlmLog;
        gAlmLog    = NULL;
    }
}

void finalAlarmAgent()
{
    if(gAlarmAgent != NULL)
    {
        delete gAlarmAgent;
        gAlarmAgent = NULL;
    }
}

bool InitAla()
{
    // initAlaLog();

    finalAlarmAgent();

    if((gAlarmAgent = new (std::nothrow) AlarmAgent()) == NULL)
        return false;

    return true;
}


void * DoALA(void * pArg)
{
    if(gAlarmAgent->Init(pArg) == true)
        gAlarmAgent->Do();

    return (void *)NULL;
}

void FinalAla()
{
    finalAlarmAgent();

    finalAlaLog();
}

MODULE_OP = {
    "ATOM_NA_ALA",      // process name
    false,              // thread running
    InitAla,            // Init
    DoALA,              // Process
    FinalAla            // CleanUp
};
