
#include <stdio.h>
#include <string>

#include "VnfmIF.hpp"
#include "CFileLog.hpp"
#include "VnaCFG.hpp"

CFileLog   *   gLog      = NULL;
VnfmIF *       gVnfmIF   = NULL;
VnaCFG         gVnaCFG;

void initVnaLog()
{
    std::string     path(getenv("HOME"));
    path += "/LOG";
    std::string     fname = "VNA";

    // gVnaCFG.InitLog(path, fname, false);
    // gVnaCFG.InitLog(path, fname, true);
}

void finalVnaLog()
{
    if(gLog != NULL)
    {
        delete  gLog;
        gLog    = NULL;
    }
}

void finalVna()
{
    if(gVnfmIF != NULL)
    {
        delete gVnfmIF;
        gVnfmIF = NULL;
    }
}

bool InitVna()
{
    // initVnaLog();

    finalVna();

    if((gVnfmIF = new (std::nothrow) VnfmIF()) == NULL)
        return false;

    return true;
}


void * DoVNA(void * pArg)
{
    while(true)
    {
        if(gVnfmIF->Init(pArg) == true)
            gVnfmIF->Do();

        poll(NULL, 0, 10);
    }

    return (void *)NULL;
}

void FinalVna()
{
    finalVna();

    finalVnaLog();
}

#include "CModule.hpp"

MODULE_OP = {
    "ATOM_NA_EXA",      // process name
    true,              // thread running
    InitVna,            // Init
    DoVNA,              // Process
    FinalVna            // CleanUp
};
