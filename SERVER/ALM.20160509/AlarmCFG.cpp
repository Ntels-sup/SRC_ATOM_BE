
#include "AlarmCFG.hpp"
#include "FetchMaria.hpp"

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
    profile_.m_strProcName = "ALM";

    return true;
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

    if(_db->Query(sql, strlen(sql)) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get node no [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    char pkg_name[64];
    char node_type[64];
    char node_no[16];

    FetchMaria  f;

    f.Clear();
    f.Set(pkg_name,     sizeof(pkg_name));
    f.Set(node_type,    sizeof(node_type));
    f.Set(node_no,      sizeof(node_no));

    if(f.Fetch(_db) == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
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

    if(_db->Query(sql, strlen(sql)) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get proc no [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    char proc_no[16];

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch(_db) == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    profile_.m_nProcNo = atoi(proc_no);

    // NM 정보
    sprintf(sql,
        "SELECT PROC_NO FROM TAT_PROCESS "
        " WHERE PROC_NAME='NM'");

    if(_db->Query(sql, strlen(sql)) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get nm [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch(_db) == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    NM.nm_node_no_  = profile_.m_nNodeNo;
    NM.nm_proc_no_  = atoi(proc_no);

    // WSM 정보
    sprintf(sql,
        "SELECT PROC_NO FROM TAT_PROCESS "
        " WHERE PROC_NAME='WSM'");

    if(_db->Query(sql, strlen(sql)) <= 0)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't get nm [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    f.Clear();
    f.Set(proc_no,  sizeof(proc_no));

    if(f.Fetch(_db) == false)
    {
        gAlmLog->ERROR("%-24s| SetProfile - can't fetch [%s] [%s]",
            "AlarmCFG",
            _db->GetErrorMsg(),
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