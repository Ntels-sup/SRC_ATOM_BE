
#include <algorithm>

#include "AlarmCFG.hpp"
#include "FetchMaria.hpp"
#include "CConfig.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

AlarmCFG::
AlarmCFG()
{
    // 그냥 기본값 설정

    DBInfo.ip_   = "127.0.0.1";
    DBInfo.port_ = 3306;
    DBInfo.user_ = "atom";
    DBInfo.pw_   = "atom";
    DBInfo.name_ = "ATOM";

    NM.ip_          = "127.0.0.1";
    NM.port_        = 8000;
    NM.retry_cnt_   = 3;
    NM.wsm_node_no_ = -1;    // 1 is EMS
    NM.wsm_proc_no_ = -1;

    NM.nm_node_no_  = -1;    // 1 is EMS
    NM.nm_proc_no_  = -1;


    DEF.buffer_size_    = 512;
    DEF.ccd_for_protocol_ = "030200";

    ENV.log_path_.assign(getenv("HOME"));
    ENV.log_path_.append("/LOG");

    ENV.file_name_              = "ALM";
    ENV.message_id_path_.assign(getenv("HOME"));
    ENV.message_id_path_.append("/DATA/ALM");

    SVC.b_avoide_duplicate_     = true;
    SVC.tca_over_code_          = ALRM_TCA_OVER;
    SVC.tca_under_code_         = ALRM_TCA_UNDER;
    SVC.trap_type_              = 1;
}


AlarmCFG::
~AlarmCFG()
{
    // Empty
}

bool AlarmCFG::
Init()
{
    profile_.m_strNodeName = "ATOM";
    profile_.m_strProcName = ENV.file_name_;

    CConfig * conf = new (std::nothrow) CConfig();

    if(conf == NULL)
    {
        if(gAlmLog != NULL)
            gAlmLog->WARNING("%-24s| Init - new operator fail to config",
                "AlarmCFG");

        return false;
    }

    if(conf->Initialize() < 0)
    {
        if(gAlmLog != NULL)
            gAlmLog->WARNING("%-24s| Init - config init fail",
                "AlarmCFG");

        delete conf;
        conf = NULL;

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

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_HOST");
    if(p != NULL)
        DBInfo.ip_  = p;

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_PORT");
    if(p != NULL && atoi(p) > 0)
        DBInfo.port_ = atoi(p);

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_USER");
    if(p != NULL)
        DBInfo.user_ = p;

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_PASS");
    if(p != NULL)
        DBInfo.pw_  = p;

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"DB_DATABASE");
    if(p != NULL)
        DBInfo.name_ = p;

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"ATOM_SERVER_IP");
    if(p != NULL)
        NM.ip_ = p;

    p = conf->GetConfigValue((char *)"GLOBAL", (char *)"ATOM_SERVER_PORT");
    if(p != NULL && atoi(p) > 0)
        NM.port_ = atoi(p);

    p = conf->GetConfigValue((char *)"ALARM",  (char *)"ALM_MSG_ID_PATH");
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

    p = conf->GetConfigValue((char *)"ALARM",  (char *)"TRAP_TYPE");
    if(p != NULL)
        SVC.trap_type_ = atoi(p);

    delete conf;

    prnLog();

    return true;
}

bool AlarmCFG::
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

void AlarmCFG::
prnLog()
{
    if(gAlmLog != NULL)
    {
        gAlmLog->INFO("%-24s| DB IP              [%s]", "AlarmCFG", DBInfo.ip_.c_str());
        gAlmLog->INFO("%-24s| DB PORT            [%d]", "AlarmCFG", DBInfo.port_);
        gAlmLog->INFO("%-24s| DB USER            [%s]", "AlarmCFG", DBInfo.user_.c_str());
        // gAlmLog->INFO("%-24s| DB PW   [%s]", "AlarmCFG", DBInfo.pw_.c_str());
        gAlmLog->INFO("%-24s| DB NAME            [%s]", "AlarmCFG", DBInfo.name_.c_str());
        gAlmLog->INFO("%-24s| ATOM SERVER IP     [%s]", "AlarmCFG", NM.ip_.c_str());
        gAlmLog->INFO("%-24s| ATOM SERVER PORT   [%d]", "AlarmCFG", NM.port_);
        gAlmLog->INFO("%-24s| MSG ID PATH        [%s]", "AlarmCFG", ENV.message_id_path_.c_str());
        gAlmLog->INFO("%-24s| AVIODE DUPLICATION [%d]", "AlarmCFG", SVC.b_avoide_duplicate_);
        gAlmLog->INFO("%-24s| SNMP TRAP TYPE     [%d]", "AlarmCFG", SVC.trap_type_);
    }
}

bool AlarmCFG::
SetProfile(DB * _db)
{
    char    sql[256];

    // 나의 NODE 정보
    sprintf(sql,
        "SELECT PKG_NAME, NODE_TYPE, NODE_NO FROM TAT_NODE "
        " WHERE NODE_NAME='%s' AND USE_YN='Y'",
        profile_.m_strNodeName.c_str());

	int 		ret = 0;
    FetchMaria  f;

    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get node no [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char pkg_name[64];
    char node_type[64];
    char node_no[16];

    f.Clear();
    f.Set(pkg_name,     sizeof(pkg_name));
    f.Set(node_type,    sizeof(node_type));
    f.Set(node_no,      sizeof(node_no));

    if(f.Fetch() == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s]",
            "AlarmCFG",
            sql);
        return false;
    }

    profile_.m_strPkgName   = pkg_name;
    profile_.m_strNodeType  = node_type;
    profile_.m_nNodeNo      = atoi(node_no);

    // 나의 PROC 정보
    sprintf(sql,
        "SELECT PROC_NO FROM TAT_PROCESS "
        " WHERE PKG_NAME='%s' AND PROC_NAME='%s' AND NODE_TYPE='%s'",
        profile_.m_strPkgName.c_str(),
        profile_.m_strProcName.c_str(),
        profile_.m_strNodeType.c_str());

    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get proc no [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char proc_no[16];

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch() == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s]",
            "AlarmCFG",
            sql);
        return false;
    }

    profile_.m_nProcNo = atoi(proc_no);

    // NM 정보
    sprintf(sql,
        "SELECT PROC_NO FROM TAT_PROCESS "
        " WHERE PROC_NAME='NM'");

    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get nm [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch() == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s]",
            "AlarmCFG",
            sql);
        return false;
    }

    NM.nm_node_no_  = profile_.m_nNodeNo;
    NM.nm_proc_no_  = atoi(proc_no);

    // WSM 정보
    sprintf(sql,
        "SELECT PROC_NO FROM TAT_PROCESS "
        " WHERE PROC_NAME='WSM'");

    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get nm [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch() == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s]",
            "AlarmCFG",
            sql);
        return false;
    }

    NM.wsm_node_no_  = profile_.m_nNodeNo;
    NM.wsm_proc_no_  = atoi(proc_no);


    gAlmLog->INFO("%-24s| Init - ALM [%d:%d] NM [%d:%d] WSM [%d:%d]",
        "AlarmCFG",
        profile_.m_nNodeNo,
        profile_.m_nProcNo,
        NM.nm_node_no_,
        NM.nm_proc_no_,
        NM.wsm_node_no_,
        NM.wsm_proc_no_);

    return true;
}
