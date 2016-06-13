
#include <algorithm>

#include "VnaCFG.hpp"
#include "CConfig.hpp"
#include "CFileLog.hpp"
#include "FetchMaria.hpp"
#include "CommonCode.hpp"

extern  CFileLog *  gLog;

VnaCFG::
VnaCFG()
{

    ENV.log_path_.assign(getenv("HOME"));
    ENV.log_path_.append("/LOG");

    ENV.file_name_              = "VNA";

    SVC.gathering_period_ = 5;
    SVC.tps_period_       = 5;
    SVC.cpu_period_       = 5;
    SVC.mem_period_       = 5;
    SVC.disk_period_      = 60;
    SVC.net_period_       = 5;
}

VnaCFG::
~VnaCFG()
{



}

bool VnaCFG::
Init(char * _cfg_fname,
     DB *   _db,
     std::string  & _pkg_name,
     std::string  & _node_type)
{

    my_pkg_name_    = _pkg_name;
    my_node_type_   = _node_type;

    CConfig * conf = new (std::nothrow) CConfig();

    if(conf == NULL)
    {
        if(gLog != NULL)
            gLog->WARNING("%-24s| Init - new operator fail to config",
                "AlarmCFG");
        return true;
    }

    if(conf->Initialize(_cfg_fname) < 0)
    {
        if(gLog != NULL)
            gLog->WARNING("%-24s| Init - config init fail [%s]",
                "VnaCFG",
                _cfg_fname);

        delete conf;
        conf = NULL;

        return true;
    }

    bool                bDebug = false;
    const char *        p = NULL;


    p = conf->GetConfigValue((char *)"VNA", (char *)"DEBUG");
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

    p = conf->GetConfigValue((char *)"VNA", (char *)"TPS_PERIOD");
    if(p != NULL && atoi(p) > 0)
        SVC.tps_period_  = atoi(p);

    p = conf->GetConfigValue((char *)"VNA", (char *)"CPU_PERIOD");
    if(p != NULL && atoi(p) > 0)
        SVC.cpu_period_  = atoi(p);

    p = conf->GetConfigValue((char *)"VNA", (char *)"MEM_PERIOD");
    if(p != NULL && atoi(p) > 0)
        SVC.mem_period_  = atoi(p);

    p = conf->GetConfigValue((char *)"VNA", (char *)"DISK_PERIOD");
    if(p != NULL && atoi(p) > 0)
        SVC.disk_period_  = atoi(p);

    p = conf->GetConfigValue((char *)"VNA", (char *)"NET_PERIOD");
    if(p != NULL && atoi(p) > 0)
        SVC.net_period_  = atoi(p);

    delete conf;
    conf = NULL;

    // DB Config
    getDBConfig(SVC.tps_period_, _db, "VNA", "TPS_REPORT_PERIOD");
    getDBConfig(SVC.cpu_period_, _db, "VNA", "CPU_REPORT_PERIOD");
    getDBConfig(SVC.mem_period_, _db, "VNA", "MEM_REPORT_PERIOD");
    getDBConfig(SVC.disk_period_,_db, "VNA", "DISK_REPORT_PERIOD");
    getDBConfig(SVC.net_period_, _db, "VNA", "NET_REPORT_PERIOD");

    // Gathering Period 는 Reporting 주기 중 가장 작은 최소값으로 합니다.
    SVC.gathering_period_ = SVC.tps_period_;

    if(SVC.gathering_period_ > SVC.cpu_period_)
        SVC.gathering_period_   = SVC.cpu_period_;

    if(SVC.gathering_period_ > SVC.mem_period_)
        SVC.gathering_period_   = SVC.mem_period_;

    if(SVC.gathering_period_ > SVC.disk_period_)
        SVC.gathering_period_   = SVC.disk_period_;

    if(SVC.gathering_period_ > SVC.net_period_)
        SVC.gathering_period_   = SVC.net_period_;

    prnLog();

    return true;
}

bool VnaCFG::
InitLog(std::string & _log_path, std::string & _log_file, bool _is_debug)
{
    if(gLog != NULL)
    {
        delete gLog;
        gLog = NULL;
    }

    int     ret = 0;
    gLog = new (std::nothrow) CFileLog(&ret);

    if(ret < 0)
    {
        delete gLog;
        printf("can't new operator for CFileLog\n");
        return false;
    }

    if(_is_debug)
    {
        gLog->Initialize(_log_path.c_str(),
                         "",
                         _log_file.c_str(),
                         -1,
                         LV_DEBUG);
    }
    else
    {
        gLog->Initialize(_log_path.c_str(),
                         "",
                         _log_file.c_str(),
                         -1,
                         LV_INFO);
    }

    return true;
}

void VnaCFG::
getDBConfig(int        & _out,
            DB         * _db,
            const char * _section,
            const char * _tag)
{
    char    sql[256];

    sprintf(sql,
        "SELECT PKG_NAME, NODE_TYPE, VALUE FROM TAT_CONFIG "
        "WHERE SECTION='%s' AND TAG='%s' ORDER BY NODE_TYPE, PKG_NAME",
        _section,
        _tag);

    FetchMaria      f;
    f.Clear();

    if(_db->Query(&f, sql, strlen(sql)) < 0)
    {
        return ;
    }

    char pkg_name[DB_PKG_NAME_SIZE+1];
    char node_type[DB_NODE_TYPE_SIZE+1];
    char value[DB_VALUE_SIZE+1];

    f.Set(pkg_name,  sizeof(pkg_name));
    f.Set(node_type, sizeof(node_type));
    f.Set(value,     sizeof(value));

    while(f.Fetch())
    {
        if(strcmp(pkg_name, "*") != 0 &&
           my_pkg_name_.compare(pkg_name) != 0)
            continue;

        if(strcmp(node_type, "*") != 0 &&
           my_node_type_.compare(node_type) != 0)
            continue;

        _out = atoi(value);
    }
}

void VnaCFG::
prnLog()
{
    if(gLog != NULL)
    {
        gLog->INFO("%-24s| TPS  PERIOD [%d]", "VnaCFG", SVC.tps_period_);
        gLog->INFO("%-24s| CPU  PERIOD [%d]", "VnaCFG", SVC.cpu_period_);
        gLog->INFO("%-24s| MEM  PERIOD [%d]", "VnaCFG", SVC.mem_period_);
        gLog->INFO("%-24s| DISK PERIOD [%d]", "VnaCFG", SVC.disk_period_);
        gLog->INFO("%-24s| NET  PERIOD [%d]", "VnaCFG", SVC.net_period_);
    }
}
