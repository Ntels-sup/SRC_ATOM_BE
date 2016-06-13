
// 본 소스는 LM_CSnmpTrap.cpp 을 그대로 porting 했습니다.

#include <sys/types.h>
#include <sys/wait.h>

#include "SnmpTrap.hpp"
#include "FetchMaria.hpp"
#include "CommonCode.hpp"

#include "CFileLog.hpp"
#include "AlarmCFG.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

void SnmpTrap::ST_TrapConfig::
Clear()
{
    oid_.clear();
    pkg_name_.clear();
    trap_proc_.clear();
    trap_version_.clear();
    agent_ip_.clear();
    agent_hostname_.clear();
}

void SnmpTrap::ST_TrapInfo::
Clear()
{
    pkg_name_.clear();
    code_.clear();
    trap_type_      = 0;
    sepc_trap_num_  = 0;
    trap_name_.clear();
    trap_value_cnt_ = 0;

    trap_value_oid_[0].clear();
    trap_value_type_[0].clear();
    trap_value_oid_[1].clear();
    trap_value_type_[1].clear();
    trap_value_oid_[2].clear();
    trap_value_type_[2].clear();
    trap_value_oid_[3].clear();
    trap_value_type_[3].clear();
    trap_value_oid_[4].clear();
    trap_value_type_[4].clear();
}

void SnmpTrap::ST_TrapTargetIp::
Clear()
{
    ip_.clear();
    pkg_name_.clear();
    port_   = 0;
    community_.clear();
    host_name_.clear();
}


SnmpTrap::
SnmpTrap()
{
    // Empty
}

SnmpTrap::
~SnmpTrap()
{

}

bool SnmpTrap::
Init(DB * _db)
{

    if(setTrapConfig(_db) == false)
    {
        gAlmLog->WARNING("%-24s| Init - can't set trap config",
            "SnmpTrap");

        return false;
    }

    if(setTrapTargetIp(_db) == false)
    {
        gAlmLog->WARNING("%-24s| Init - can't set trap target ip",
            "SnmpTrap");

        return false;
    }

    if(setTrapInfo(_db) == false)
    {
        gAlmLog->WARNING("%-24s| Init - can't set trap info",
            "SnmpTrap");

        return false;
    }

    return true;
}

bool SnmpTrap::
setTrapConfig(DB * _db)
{
    char sql[128];

    sprintf(sql,
        "SELECT ENTERPRISE_OID, PKG_NAME, TRAP_PROC, TRAP_VERSION, AGENT_IP, AGENT_HOSTNAME "
        " FROM TAT_TRAP_CONFIG");

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| setTrapConfig - query fail [%d:%s] [%s]",
            "SnmpTrap",
			ret,
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char    enterprise_oid[DB_TRAP_ENTERPRISE_OID_SIZE+1];
    char    pkg_name[DB_PKG_NAME_SIZE+1];
    char    trap_proc[DB_TRAP_PROC_SIZE+1];
    char    trap_version[DB_TRAP_VERSION_SIZE+1];
    char    agent_ip[DB_TRAP_IP_SIZE+1];
    char    agent_hostname[DB_TRAP_AGENT_HOSTNAME_SIZE+1];

    f.Clear();
    f.Set(enterprise_oid, sizeof(enterprise_oid));
    f.Set(pkg_name,       sizeof(pkg_name));
    f.Set(trap_proc,      sizeof(trap_proc));
    f.Set(trap_version,   sizeof(trap_version));
    f.Set(agent_ip,       sizeof(agent_ip));
    f.Set(agent_hostname, sizeof(agent_hostname));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        ST_TrapConfig   stTrapConfig;
        stTrapConfig.Clear();

        stTrapConfig.oid_           = enterprise_oid;
        stTrapConfig.pkg_name_      = pkg_name;
        stTrapConfig.trap_proc_     = trap_proc;
        stTrapConfig.trap_version_  = trap_version;
        stTrapConfig.agent_ip_      = agent_ip;
        stTrapConfig.agent_hostname_= agent_hostname;

        map_trap_config_[stTrapConfig.pkg_name_] = stTrapConfig;
    }

    gAlmLog->DEBUG("%-24s| setTrapConfig - Success [cnt:%d]",
        "SnmpTrap",
        map_trap_config_.size());

    return true;
}

bool SnmpTrap::
setTrapTargetIp(DB * _db)
{
    char sql[128];

    sprintf(sql,
        "SELECT IP, PKG_NAME, PORT, COMMUNITY, HOST_NAME "
        " FROM TAT_TRAP_IP");

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| setTrapTargetIp - query fail [%d:%s] [%s]",
            "SnmpTrap",
			ret,
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char    ip[DB_TRAP_IP_SIZE+1];
    char    pkg_name[DB_PKG_NAME_SIZE+1];
    char    port[8];
    char    community[DB_TRAP_COMMUNITY_SIZE+1];
    char    host_name[DB_TRAP_HOST_NAME_SIZE+1];

    f.Clear();
    f.Set(ip,            sizeof(ip));
    f.Set(pkg_name,      sizeof(pkg_name));
    f.Set(port,          sizeof(port));
    f.Set(community,     sizeof(community));
    f.Set(host_name,     sizeof(host_name));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        ST_TrapTargetIp   stTrapTargetIp;
        stTrapTargetIp.Clear();

        stTrapTargetIp.ip_              = ip;
        stTrapTargetIp.pkg_name_        = pkg_name;
        stTrapTargetIp.port_            = atoi(port);
        stTrapTargetIp.community_       = community;
        stTrapTargetIp.host_name_       = host_name;

        auto iter = map_trap_target_ip_.find(stTrapTargetIp.pkg_name_);

        if(iter != map_trap_target_ip_.end())
        {
            (iter->second).push_back(stTrapTargetIp);
        }
        else
        {
            std::vector<ST_TrapTargetIp>    vec;
            vec.push_back(stTrapTargetIp);

            map_trap_target_ip_[stTrapTargetIp.pkg_name_] = vec;
        }
    }


    gAlmLog->INFO("%-24s| setTrapTargetIp - Success [cnt:%d]",
        "SnmpTrap",
        map_trap_target_ip_.size());

    return true;
}

bool SnmpTrap::
setTrapInfo(DB * _db)
{
    char sql[320];

    sprintf(sql,
        "SELECT PKG_NAME, CODE, TRAP_TYPE, SEPC_TRAP_NUM, TRAP_NAME, TRAP_VALUE_CNT, "
        " TRAP_VALUE_1_OID, TRAP_VALUE_1_TYPE, "
        " TRAP_VALUE_2_OID, TRAP_VALUE_2_TYPE, "
        " TRAP_VALUE_3_OID, TRAP_VALUE_3_TYPE, "
        " TRAP_VALUE_4_OID, TRAP_VALUE_4_TYPE, "
        " TRAP_VALUE_5_OID, TRAP_VALUE_5_TYPE "
        " FROM TAT_TRAP_INFO");

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| setTrapInfo - query fail [%d:%s] [%s]",
            "SnmpTrap",
			ret,
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    char    pkg_name[DB_PKG_NAME_SIZE+1];
    char    code[DB_ALM_CODE_SIZE+1];
    char    trap_type[8];
    char    sepc_trap_num[8];
    char    trap_name[DB_TRAP_NAME_SIZE+1];
    char    trap_value_cnt[8];
    char    trap_value_0_oid[DB_TRAP_VALUE_OID_SIZE+1];
    char    trap_value_0_type[DB_TRAP_VALUE_TYPE_SIZE+1];
    char    trap_value_1_oid[DB_TRAP_VALUE_OID_SIZE+1];
    char    trap_value_1_type[DB_TRAP_VALUE_TYPE_SIZE+1];
    char    trap_value_2_oid[DB_TRAP_VALUE_OID_SIZE+1];
    char    trap_value_2_type[DB_TRAP_VALUE_TYPE_SIZE+1];
    char    trap_value_3_oid[DB_TRAP_VALUE_OID_SIZE+1];
    char    trap_value_3_type[DB_TRAP_VALUE_TYPE_SIZE+1];
    char    trap_value_4_oid[DB_TRAP_VALUE_OID_SIZE+1];
    char    trap_value_4_type[DB_TRAP_VALUE_TYPE_SIZE+1];

    f.Clear();
    f.Set(pkg_name,             sizeof(pkg_name));
    f.Set(code,                 sizeof(code));
    f.Set(trap_type,            sizeof(trap_type));
    f.Set(sepc_trap_num,        sizeof(sepc_trap_num));
    f.Set(trap_name,            sizeof(trap_name));
    f.Set(trap_value_cnt,       sizeof(trap_value_cnt));
    f.Set(trap_value_0_oid,     sizeof(trap_value_0_oid));
    f.Set(trap_value_0_type,    sizeof(trap_value_0_type));
    f.Set(trap_value_1_oid,     sizeof(trap_value_1_oid));
    f.Set(trap_value_1_type,    sizeof(trap_value_1_type));
    f.Set(trap_value_2_oid,     sizeof(trap_value_2_oid));
    f.Set(trap_value_2_type,    sizeof(trap_value_2_type));
    f.Set(trap_value_3_oid,     sizeof(trap_value_3_oid));
    f.Set(trap_value_3_type,    sizeof(trap_value_3_type));
    f.Set(trap_value_4_oid,     sizeof(trap_value_4_oid));
    f.Set(trap_value_4_type,    sizeof(trap_value_4_type));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        ST_TrapInfo     stTrapInfo;

        stTrapInfo.Clear();

        stTrapInfo.pkg_name_        = pkg_name;
        stTrapInfo.code_            = code;
        stTrapInfo.trap_type_       = atoi(trap_type);
        stTrapInfo.sepc_trap_num_   = atoi(sepc_trap_num);
        stTrapInfo.trap_name_       = trap_name;
        stTrapInfo.trap_value_cnt_  = atoi(trap_value_cnt);

        stTrapInfo.trap_value_oid_[0]  = trap_value_0_oid;
        stTrapInfo.trap_value_type_[0] = trap_value_0_type;
        stTrapInfo.trap_value_oid_[1]  = trap_value_1_oid;
        stTrapInfo.trap_value_type_[1] = trap_value_1_type;
        stTrapInfo.trap_value_oid_[2]  = trap_value_2_oid;
        stTrapInfo.trap_value_type_[2] = trap_value_2_type;
        stTrapInfo.trap_value_oid_[3]  = trap_value_3_oid;
        stTrapInfo.trap_value_type_[3] = trap_value_3_type;
        stTrapInfo.trap_value_oid_[4]  = trap_value_4_oid;
        stTrapInfo.trap_value_type_[4] = trap_value_4_type;

        auto iter = map_trap_info_.find(stTrapInfo.pkg_name_ + stTrapInfo.code_);

        if(iter != map_trap_info_.end())
        {
            (iter->second).push_back(stTrapInfo);
        }
        else
        {
            std::vector<ST_TrapInfo>    vec;
            vec.push_back(stTrapInfo);

            map_trap_info_[stTrapInfo.pkg_name_ + stTrapInfo.code_] = vec;
        }
    }


    gAlmLog->INFO("%-24s| setTrapInfo - Success [cnt:%d]",
        "SnmpTrap",
        map_trap_info_.size());

    return true;
}

bool SnmpTrap::
SendTrap(const char * _pkg_name, const char * _code, char * _token)
{

    int sent_cnt = 0;

#ifdef _USE_TRAP_

    char    cmd_buf[1024];

    ST_TrapConfig * spTrapConfig = getTrapConfig(_pkg_name);

    if(spTrapConfig == NULL || spTrapConfig->trap_proc_.size() <= 0)
    {
        gAlmLog->DEBUG("%-24s| SendTrap - can't get trap config pkgname[%s]",
            "SnmpTrap",
            _pkg_name);
        return false;
    }

    ST_TrapInfo * spTrapInfo = getTrapInfo(_pkg_name, _code);

    if(spTrapInfo == NULL)
    {
        gAlmLog->DEBUG("%-24s| SendTrap - can't get trap info pkgname[%s] code [%s]",
            "SnmpTrap",
            _pkg_name,
            _code);
        return false;
    }

    int     len = 0;
    char    cmd_arg[1024];
    memset(cmd_arg, 0, sizeof(cmd_arg));

    for(int nLoop =0; nLoop < spTrapInfo->trapValueCnt_; nLoop++)
    {
        len += sprintf(cmd_arg + len,
            " %s %s \"%s\"",
            spTrapInfo_->trap_value_oid_[0],
            spTrapInfo_->trap_value_type_[0],
            _token);
    }

    auto iter = map_trap_target_ip_.find(_pkg_name);

    if(iter == map_trap_target_ip_.end())
    {
        gAlmLog->DEBUG("%-24s| SendTrap - can't get trap target ip [%s] code [%s]",
            "SnmpTrap",
            _pkg_name,
            _code);

        return false;
    }

    std::vector<ST_TrapTargetIp> & vec_for_targets = iter->second;
    std::string cmd;

    for(auto vec_iter = vec_for_targets.begin();
             vec_iter != vec_for_targets.end();
             ++vec_iter)
    {
        ST_TrapTargetIp & stTrapTargetIp = *iter;

        if(setCommand(cmd, spTrapConfig, spTrapInfo_, &stTrapTargetIp) == false)
        {
            gAlmLog->WARNING("%-24s| SendTrap - unsupported SNMP trap version [%s] conf[%d] [%s:%s]",
                "SnmpTrap",
                spTrapConfig->trap_version_,
                gCFG.SVC.trap_type_,
                _pkg_name,
                _code);

            return false;
        }

        cmd.append(" ");
        cmd.append(cmd_arg);
        cmd.append(" > /dev/null 2>&1");

        if(execute(cmd) == false)
        {
            gAlmLog->WARNING("%-24s| SendTrap - Fail [%s]",
                "SnmpTrap",
                cmd.c_str());
            continue;
        }

        gAlmLog->DEBUG("%-24s| SendTrap - SUCCESS [%s]",
            "SnmpTrap",
            cmd.c_str());

        sent_cnt++;
    }
#else
    return false;
#endif // _USE_TRAP_

    return sent_cnt > 0;
}

bool SnmpTrap::
setCommand(std::string     & _out,
           ST_TrapConfig   * _spTrapConfig,
           ST_TrapInfo     * _spTrapInfo,
           ST_TrapTargetIp * _spTrapTargetIp)
{
    char cmd_buf[512];
    if(gCFG.SVC.trap_type_ == 1)
    {
        if(_spTrapConfig->trap_version_.compare("1") == 0)
        {
            sprintf(cmd_buf,
                "%s -v %s -c %s %s:%d %s %s %d %d '' ",
                _spTrapConfig->trap_proc_.c_str(),
                _spTrapConfig->trap_version_.c_str(),
                _spTrapTargetIp->community_.c_str(),
                _spTrapTargetIp->ip_.c_str(),
                _spTrapTargetIp->port_,
                _spTrapConfig->oid_.c_str(),
                _spTrapTargetIp->ip_.c_str(),
                _spTrapInfo->trap_type_,
                _spTrapInfo->sepc_trap_num_);
        }
        else if(_spTrapConfig->trap_version_.compare("2c") == 0)
        {
            sprintf(cmd_buf,"%s -v %s -c %s %s:%d '' %s.%d ",
                _spTrapConfig->trap_proc_.c_str(),
                _spTrapConfig->trap_version_.c_str(),
                _spTrapTargetIp->community_.c_str(),
                _spTrapTargetIp->ip_.c_str(),
                _spTrapTargetIp->port_,
                _spTrapConfig->oid_.c_str(),
                _spTrapInfo->sepc_trap_num_);
        }
        else
            return false;
    }
    else if(gCFG.SVC.trap_type_ == 2)
    {
        /* HP Trap */
        //2010-06-21 trap 전송 실행파일에 따라 형식 구분
        //net-snmp or HP snmp
        //  snmptrap: too few arguments: 0, expect at least 6.
        //  usage: snmptrap [options] node enterprise agent-addr generic specific timestamp
        //       [variable type value]...
        //
        //       Node, enterprise, agent-addr and time-stamp
        //       can be defaulted with a null ("") string.
        //
        //       Options:
        //           -d                  dump ASN.1 packet trace
        //           -c community        community string
        //           -p port             remote port
        //[20100621 03:02:21:97] nTrapNum=[110],TrapCommBuf=[/opt/OV/bin/snmptrap 60.11.8.53 1.3.6
        //stringascii "ipms41" 1.3.6.1.4.1.9608.7.2.1.3 integer "95" 1.3.6.1.4.1.9608.7.2.3.3 inte
        //proc destip enterpiseoid agentaddr traptype specnum  trapdef oid1 type1 val1....n

        sprintf(cmd_buf,"%s %s %s %s %d %d 0 ", //trap define은 0으로 고정
            _spTrapConfig->trap_proc_.c_str(),
            _spTrapTargetIp->ip_.c_str(),
            _spTrapConfig->oid_.c_str(),
            _spTrapConfig->agent_ip_.c_str(),
            _spTrapInfo->trap_type_,
            _spTrapInfo->sepc_trap_num_);
    }
    else
    {
        gAlmLog->WARNING("%-24s| invalid trap type [%d]",
            "SnmpTrap",
            gCFG.SVC.trap_type_);

        return false;
    }

    _out = cmd_buf;
    return true;
}

bool SnmpTrap::
execute(std::string & _cmd)
{
    int rv = system(_cmd.c_str());

    gAlmLog->DEBUG("%-24s| system [%s]",
        "SnmpTrap",
        _cmd.c_str());

    if(rv == -1)
    {
        gAlmLog->WARNING("%-24s| execute - system command error [%d:%s] [%s]",
            "SnmpTrap",
            errno,
            strerror(errno),
            _cmd.c_str());
        return false;
    }

    if(WIFEXITED(rv))
    {
        if(WEXITSTATUS(rv) == 0)
            return true;

        gAlmLog->WARNING("%-24s| execute - normal exit. but abnormal terminated [%d]",
            "SnmpTrap",
            WEXITSTATUS(rv));
    }
    else if(WIFSIGNALED(rv))
    {
        gAlmLog->WARNING("%-24s| execute - exit by signal [%d]",
            "SnmpTrap",
            WTERMSIG(rv));
    }
    else if(WIFSTOPPED(rv))
    {
        gAlmLog->WARNING("%-24s| execute - stopped",
            "SnmpTrap");
    }
    else
        ;

    return false;
}
