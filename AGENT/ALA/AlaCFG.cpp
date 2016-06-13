
#include <algorithm>

#include "AlaCFG.hpp"
#include "CConfig.hpp"
#include "CFileLog.hpp"

extern CFileLog * gAlmLog;

AlaCFG::
AlaCFG()
{
    ENV.log_path_.assign(getenv("HOME"));
    ENV.log_path_.append("/LOG");

    ENV.file_name_              = "ALA";
    ENV.message_id_path_.assign(getenv("HOME"));
    ENV.message_id_path_.append("/DATA/ALA");

    SVC.b_avoide_duplicate_     = true;
}

AlaCFG::
~AlaCFG()
{
    // Empty
}

bool AlaCFG::
Init(char * _cfg_fname)
{
    CConfig * conf = new (std::nothrow) CConfig();

    if(conf == NULL)
    {
        if(gAlmLog != NULL)
            gAlmLog->WARNING("%-24s| Init - new operator fail to config",
                "AlaCFG");

        return false;
    }

    if(conf->Initialize(_cfg_fname) < 0) {
        if(gAlmLog != NULL)
            gAlmLog->WARNING("%-24s| Init - config Initialize fail [%s]",
                "AlaCFG",
                _cfg_fname);

        if(conf != NULL)
        {
            delete conf;
            conf = NULL;
        }

        return false;
    }

    bool                bDebug = false;
    const char *        p = NULL;

    p = conf->GetConfigValue((char *)"ALARM", (char *)"DEBUG");
    if(p != NULL)
    {
        if(strcasecmp(p, "true") == 0 || strcasecmp(p, "1") == 0)
            bDebug = true;
    }

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"LOG_PATH");
    if(p != NULL)
        ENV.log_path_   = p;

    if(InitLog(ENV.log_path_, ENV.file_name_, bDebug) == false)
    {
        delete conf;
        conf = NULL;

        return false;
    }

    p = conf->GetConfigValue((char *)"ALARM",  (char *)"ALA_MSG_ID_PATH");
    if(p != NULL)
        ENV.message_id_path_ = p;

    p = conf->GetConfigValue((char *)"ALARM", (char *)"AVOIDE_DUPLICATION");
    if(p != NULL)
    {
        if(strcasecmp(p, "true") == 0 || strcasecmp(p, "1") == 0)
            SVC.b_avoide_duplicate_ = true;
        else
            SVC.b_avoide_duplicate_ = false;
    }

    delete conf;

    prnLog();

    return true;
}

bool AlaCFG::
InitLog(std::string & _log_path, std::string & _log_file, bool _is_debug)
{
    if(gAlmLog != NULL)
    {
        delete gAlmLog;
        gAlmLog = NULL;
    }

    int     ret = 0;
    gAlmLog = new (std::nothrow) CFileLog(&ret);

    if(ret < 0)
    {
        delete gAlmLog;
        printf("can't new operator for CFileLog\n");
        return false;
    }

    if(_is_debug)
    {
        gAlmLog->Initialize(_log_path.c_str(),
                            NULL,
                            _log_file.c_str(),
                            -1,
                            LV_DEBUG);
    }
    else
    {
        gAlmLog->Initialize(_log_path.c_str(),
                            NULL,
                            _log_file.c_str(),
                            -1,
                            LV_INFO);
    }

    return true;
}

void AlaCFG::
prnLog()
{
    if(gAlmLog != NULL)
    {
        gAlmLog->INFO("%-24s| message id path [%s]",
            "AlaCFG",
            ENV.message_id_path_.c_str());
        gAlmLog->INFO("%-24s| avoid duplication [%s]",
            "AlaCFG",
            (SVC.b_avoide_duplicate_)?"true":"false");
    }
}

bool AlaCFG::
SetProfile(CModule::ST_MODULE_OPTIONS & _option)
{
    if(_option.m_nNodeNo <= 0)
        return false;

    profile_.m_strPkgName    = _option.m_szPkgName;

    profile_.m_strNodeName   = _option.m_szNodeName;
    profile_.m_strNodeType   = _option.m_szNodeType;
    profile_.m_strNodeVersion= _option.m_szNodeVersion;

    profile_.m_nNodeNo       = _option.m_nNodeNo;
    profile_.m_nProcNo       = _option.m_nProcNo;

    return true;
}