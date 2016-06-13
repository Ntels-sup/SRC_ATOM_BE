
#include <cstdio>
#include <unistd.h>
#include <poll.h>

#include "AlarmManager.hpp"
#include "CFileLog.hpp"
#include "AlarmCFG.hpp"

CFileLog *  gAlmLog = NULL;
AlarmCFG    gCFG;

static void  initSignal(void)
{
    static struct sigaction     act;
    act.sa_handler  =   SIG_IGN;

    // sigaction(SIGINT ,  &act, NULL);
    sigaction(SIGALRM , &act, NULL);
    sigaction(SIGHUP,   &act, NULL);
    sigaction(SIGCHLD,  &act, NULL);
}

void initLog()
{

    std::string     path(getenv("HOME"));
    path += "/LOG";
    std::string     fname = "ALM";

    gCFG.InitLog(path, fname, true);
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
    initSignal();

    // initLog();

    AlarmManager * pM   = new AlarmManager();

    while(true)
    {
        if(pM->Init() == true)
            break;

        gAlmLog->ERROR("%-24s| alarm manager init fail",
            "main");

        poll(NULL, 0, 5 * 1000);
    }

    while(true)
        pM->Do();

    pM->Final();

    gAlmLog->WARNING("%-24s| ALM TERMINATED",
        "main");

    finalLog();

    return 0;
}
