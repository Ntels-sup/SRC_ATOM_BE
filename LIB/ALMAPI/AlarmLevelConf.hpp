
#ifndef __ALARM_LEVEL_CONF_HPP__
#define __ALARM_LEVEL_CONF_HPP__

#include <string>
#include <unordered_map>
#include <list>

#include "DB.hpp"
#include "FetchMaria.hpp"

#include "CommonCode.hpp"

struct ST_Level
{
    void    Clear();
    std::string      greater_;
    ALM::eSEVERITY   severity_;
};

struct ST_AlarmLevelDef
{
    void    Clear();
    ST_AlarmLevelDef & operator=(const ST_AlarmLevelDef & _st);

    int                     monitor_id_;
    std::string             pkg_name_;
    std::string             code_;
    std::string             node_type_;
    std::string             location_;
    std::string             target_;
    std::string             complement_;
    bool                    range_;

    std::list<ST_Level>     list_;
};

class AlarmLevelConf
{
public:
    explicit    AlarmLevelConf();
    ~AlarmLevelConf();

    bool SetAlarmLevel(DB           * _db,
                       const char   * _pkg_name = "",
                       const char   * _node_type = "");

    bool IsExist(std::string & _node_type,
                 std::string & _code,
                 std::string & _target);
    ALM::eSEVERITY  GetSeverity(std::string & _node_type,
                                std::string & _code,
                                std::string & _target,
                                std::string & _value,
                                std::string & _complement);

private:
    void            setMap(ST_AlarmLevelDef & _def);
    bool            setSeverityByMonitor(DB                  *  _db,
                                         int                    _monitor_id,
                                         std::list<ST_Level> &  _list);
    ST_AlarmLevelDef *  getAlarmLevelDef(std::string & _node_type,
                                         std::string & _code,
                                         std::string & _target);

    void            prn();
    ALM::eSEVERITY  decideSeverity(std::string         & _value,
                                   std::list<ST_Level> & _list,
                                   bool                  _is_range);

private:

    // KEY = code
    std::unordered_map<std::string,
                       std::list<ST_AlarmLevelDef> >   map_;

};

#endif // __ALARM_LEVEL_CONF_H__PP


