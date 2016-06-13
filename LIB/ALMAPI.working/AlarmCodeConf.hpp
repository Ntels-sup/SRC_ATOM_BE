
#ifndef __ALARM_CODE_CONF_HPP__
#define __ALARM_CODE_CONF_HPP__

#include <string>
#include <unordered_map>

#include "DBConfig.hpp"
#include "FetchMaria.hpp"
#include "CommonCode.hpp"

// #include "AlarmDataDef.hpp"

struct ST_AlarmCodeDef
{
    void            Clear();
    ST_AlarmCodeDef & operator=(const ST_AlarmCodeDef & _stAlamCodeDef);

    std::string     pkg_name_;
    std::string     code_;
    int             event_type_id_;
    int             event_group_id_;
    std::string     alias_code_;
    std::string     probable_cause_;
    int             severity_id_;
    std::string     sms_yn_;
    std::string     email_yn_;
    std::string     snmp_yn_;
};

class AlarmCodeConf
{
public:
    explicit AlarmCodeConf();
    AlarmCodeConf & operator=(const AlarmCodeConf & _alarm_code_conf);
    ~AlarmCodeConf();

    bool SetAlarmCode(DBConfig   *   _dbConfig,
                     std::string &   _code,
                     const char *    _pkg_name = "");

    bool SetAlarmCode(DBConfig  *   _dbConfig,
                     const char *   _pkg_name);

    bool IsExist(std::string & _code);
    ST_AlarmCodeDef * GetAlarmCodeDef(std::string & _code);

    std::string & GetAliasCode(std::string & _code);
    std::string & GetProbableCause(std::string & _code);
    int           GetEventTypeId(std::string & _code);
    int           GetEventGroupId(std::string & _code);
    int           GetSeverity(std::string & _code);
    bool          IsSMS(std::string & _code);
    bool          IsEmail(std::string & _code);
    bool          IsSNMP(std::string & _code);


private:
    bool queryAlarmCode(DBConfig    *  _dbConfig,
                       FetchMaria  & _f,
                       const char  * _code,
                       const char  * _pkg_name);

    void setFetchData(FetchMaria & _f);
    void setAlarmCodeDef();

private:
    // code
    std::unordered_map<std::string, ST_AlarmCodeDef>    map_;

    char    pkg_name_[DB_PKG_NAME_SIZE+1];
    char    code_[DB_ALM_CODE_SIZE+1];
    char    event_type_id_[4];
    char    event_group_id_[4];
    char    alias_code_[DB_ALM_ALIAS_CODE_SIZE+1];
    char    probable_cause_[DB_ALM_PROBABLE_CAUSE_SIZE+1];
    char    severity_id_[4];
    char    sms_yn_[4];
    char    email_yn_[4];
    char    snmp_yn_[4];

    std::string     empty_;

};


#endif // __ALARM_CODE_CONF_HPP__
