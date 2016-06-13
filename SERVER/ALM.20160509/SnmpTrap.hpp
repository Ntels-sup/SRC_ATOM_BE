
#ifndef __SNMP_TRAP_HPP__
#define __SNMP_TRAP_HPP__

#include <string>
#include <vector>
#include <unordered_map>

#include "DB.hpp"


class SnmpTrap
{
public:
    explicit    SnmpTrap();
    ~SnmpTrap();

    bool    Init(DB * _db);
    void    SendTrap(const char * _pkg_name, const char * _code, char * _token);

public:
    struct ST_TrapConfig {

        void    Clear();

        std::string     oid_;
        std::string     pkg_name_;
        std::string     trap_proc_;
        std::string     trap_version_;
        std::string     agent_ip_;
        std::string     agent_hostname_;
    };

    struct ST_TrapInfo {

        void    Clear();

        std::string     pkg_name_;
        std::string     code_;
        int             trap_type_;
        int             sepc_trap_num_;
        std::string     trap_name_;
        int             trap_value_cnt_;
        std::string     trap_value_oid_[5];
        std::string     trap_value_type_[5];
    };

    struct ST_TrapTargetIp {

        void    Clear();

        std::string     ip_;
        std::string     pkg_name_;
        int             port_;
        std::string     community_;
        std::string     host_name_;

    };

private:
    bool setTrapConfig(DB * _db);
    bool setTrapTargetIp(DB * _db);
    bool setTrapInfo(DB * _db);

    bool setCommand(std::string     & _out,
                   ST_TrapConfig   * _spTrapConfig,
                   ST_TrapInfo     * _spTrapInfo,
                   ST_TrapTargetIp * _spTrapTargetIp);
    bool execute(std::string & _cmd);

private:
    std::unordered_map<std::string, ST_TrapConfig>  map_trap_config_;
    std::unordered_map<std::string,
                       std::vector<ST_TrapTargetIp> > map_trap_target_ip_;
    std::unordered_map<std::string,
                       std::vector<ST_TrapInfo> >   map_trap_info_;

};

#endif // __SNMP_TRAP_HPP__
