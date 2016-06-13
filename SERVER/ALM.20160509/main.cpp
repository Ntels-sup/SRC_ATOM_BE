
#include <cstdio>
#include <unistd.h>

#include "AlarmManager.hpp"
#include "CFileLog.hpp"
#include "AlarmCFG.hpp"

CFileLog *  gAlmLog = NULL;
AlarmCFG    gCFG;

void initLog()
{
    if(gAlmLog == NULL)
    {
        int ret = 0;
        gAlmLog    = new (std::nothrow) CFileLog(&ret);

        if(ret < 0)
        {
            delete gAlmLog;
            printf("can't new operator for CFileLog\n");
            exit(0);
        }
    }

    std::string     path(getenv("HOME"));
    path += "/LOG";
    gAlmLog->Initialize((char *)path.c_str(), "ATOM", "ALM", -1, LV_DEBUG);
}

void finalLog()
{
    if(gAlmLog != NULL)
    {
        delete  gAlmLog;
        gAlmLog    = NULL;
    }
}


int main()
{
    initLog();

    AlarmManager * pM   = new AlarmManager();

    if(pM->Init() == false)
    {
        gAlmLog->ERROR("%-24s| alarm manager init fail",
            "main");
        return 0;
    }

    while(true)
        pM->Do();

    pM->Final();

    gAlmLog->WARNING("%-24s| ALM TERMINATED",
        "main");

    finalLog();

    return 0;
}
