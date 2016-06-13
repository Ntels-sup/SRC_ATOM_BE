#include <string.h>

#include "AlarmLevelConf.hpp"

#include "CFileLog.hpp"

extern CFileLog * gAlmLog;

void ST_Level::
Clear()
{
    greater_.clear();
    severity_   = ALM::eCLEARED;
}


void ST_AlarmLevelDef::
Clear()
{
    monitor_id_     = 0;

    pkg_name_.clear();
    code_.clear();
    node_type_.clear();
    location_.clear();
    target_.clear();
    complement_.clear();

    range_       = false;

    list_.clear();
}

ST_AlarmLevelDef & ST_AlarmLevelDef::
operator=(const ST_AlarmLevelDef & _st)
{
    if(this != &_st)
    {
        monitor_id_ = _st.monitor_id_;
        pkg_name_   = _st.pkg_name_;
        code_       = _st.code_;
        node_type_  = _st.node_type_;
        location_   = _st.location_;
        target_     = _st.target_;
        complement_ = _st.complement_;
        range_      = _st.range_;

        list_       = _st.list_;
    }

    return *this;
}

AlarmLevelConf::
AlarmLevelConf()
{
    map_.clear();
}

AlarmLevelConf::
~AlarmLevelConf()
{
    // Empty
}

bool AlarmLevelConf::
SetAlarmLevel(DB         * _db,
              const char * _pkg_name,
              const char * _node_type)
{

    int  len = 0;
    char sql[256];

    // Attention ---
    // Ordering 순서는 매우 중요 합니다.
    // 이게 틀어지면, Level 확인 하는 Scheme 이 틀어집니다.
    // ORDER BY TARGET, NODE_TYPE 이면, 우선순위는 TARGET > NODE_TYPE 입니다.
    // getAlarmLevelDef() 참조
    len = sprintf(sql,
            "SELECT MONITOR_ID, PKG_NAME, CODE, NODE_TYPE, "
                " LOCATION, TARGET, COMPLEMENT, RANGE_YN "
            " FROM TAT_ALM_MONITOR "
            " WHERE PKG_NAME='%s' "
            " ORDER BY TARGET, NODE_TYPE",
            _pkg_name);

    char * p = sql + len;

    if(_node_type[0] != '\0')
    {
        len = sprintf(p, " AND NODE_TYPE='%s'", _node_type);
        p += len;
    }

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| SetAlarmLevelConf - select fail [%d:%s] [%s]",
            "AlarmLevelConf",
			ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    char    monitor_id[16];
    char    pkg_name[DB_PKG_TYPE_SIZE+1];
    char    code[DB_ALM_CODE_SIZE+1];
    char    node_type[DB_ALM_NODE_TYPE_SIZE+1];
    char    location[DB_ALM_LOCATION_SIZE+1];
    char    target[DB_ALM_OBJECT_SIZE+1];
    char    complement[DB_ALM_COMPLEMENT_SIZE+1];
    char    range[4];

    f.Set(monitor_id,     sizeof(monitor_id));
    f.Set(pkg_name,       sizeof(pkg_name));
    f.Set(code,           sizeof(code));
    f.Set(node_type,      sizeof(node_type));
    f.Set(location,       sizeof(location));
    f.Set(target,         sizeof(target));
    f.Set(complement,     sizeof(complement));
    f.Set(range,          sizeof(range));

    while(true)
    {
        if(f.Fetch() != true)
            break;

        ST_AlarmLevelDef       def;

        def.Clear();

        def.monitor_id_     = atoi(monitor_id);
        def.pkg_name_       = pkg_name;
        def.code_           = code;
        def.node_type_      = node_type;
        def.location_       = location;
        def.target_         = target;
        def.complement_     = complement;

        if(range[0] == 'Y')
            def.range_      = true;
        else
            def.range_      = false;

        def.list_.clear();

        if(setSeverityByMonitor(_db,
                                def.monitor_id_,
                                def.list_) != true)
        {
             gAlmLog->ERROR("%-24s| setSeverityByMonitor fail",
                "AlarmLevelConf");
            return false;
        }

        setMap(def);
    }

    // prn();
    gAlmLog->INFO("%-24s| SetAlarmLevel SUCCESS",
        "AlarmLevelConf");

    return true;
}

void AlarmLevelConf::
setMap(ST_AlarmLevelDef & _def)
{
    auto iter = map_.find(_def.code_);

    if(iter == map_.end())
    {
        std::list<ST_AlarmLevelDef>     list;
        list.push_back(_def);

        map_[_def.code_] = list;
    }
    else
    {
        (iter->second).push_back(_def);
    }
}

bool AlarmLevelConf::
setSeverityByMonitor(DB * _db,
                     int  _monitor_id,
                     std::list<ST_Level> & _list)
{
    char sql[256];

    sprintf(sql,
        "SELECT MONITOR_ID, SEVERITY_CCD, VALUE "
        " FROM TAT_ALM_LEVEL "
        " WHERE MONITOR_ID=%d ORDER BY VALUE", _monitor_id);

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| setSeverityByMonitor - select fail [%d:%s] [%s]",
            "AlarmLevelConf",
			ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    char monitor_id_buf[16];
    char severity_ccd_buf[4];
    char value_buf[DB_ALM_VALUE_SIZE+1];

    f.Set(monitor_id_buf,   sizeof(monitor_id_buf));
    f.Set(severity_ccd_buf, sizeof(severity_ccd_buf));
    f.Set(value_buf,        sizeof(value_buf));

    ST_Level    level;

    while(true)
    {
        if(f.Fetch() == false)
            break;

        level.Clear();

        level.greater_  = value_buf;
        level.severity_ = (ALM::eSEVERITY)atoi(severity_ccd_buf);

        _list.push_back(level);
    }

    return true;
}

void AlarmLevelConf::
prn()
{
    for(auto iter = map_.begin(); iter != map_.end(); ++iter)
    {
        std::list<ST_AlarmLevelDef> & list =
            iter->second;

        for(auto def_iter = list.begin(); def_iter != list.end(); ++def_iter)
        {
            ST_AlarmLevelDef & st = *def_iter;
            gAlmLog->INFO("%-24s| mid[%d] code[%s] location[%s] target[%s] complement[%s]",
                "AlarmLevelConf",
                st.monitor_id_,
                st.code_.c_str(),
                st.location_.c_str(),
                st.target_.c_str(),
                st.complement_.c_str());

            for(auto level_iter = st.list_.begin();
                level_iter != st.list_.end();
                ++level_iter)
            {
                ST_Level & level = *level_iter;
                gAlmLog->INFO("%-24s|     - severity[%d] greater[%s]",
                    "AlarmLevelConf",
                    level.severity_,
                    level.greater_.c_str());
            }
        }
    }
}

bool AlarmLevelConf::
IsExist(std::string & _node_type,
        std::string & _code,
        std::string & _target)
{
    return getAlarmLevelDef(_node_type, _code, _target) != NULL;
}

ST_AlarmLevelDef * AlarmLevelConf::
getAlarmLevelDef(std::string & _node_type,
                 std::string & _code,
                 std::string & _target)
{

    auto iter = map_.find(_code);

    if(iter == map_.end())
        return NULL;

    ST_AlarmLevelDef *    ret   = NULL;

    // 정확하게 일치하지 않는 경우에는, SELECT Query  의 마지막까지 확인합니다.
    // 따라서, SELECT 의 ORDER 가 매우 중요합니다.
    std::list<ST_AlarmLevelDef> & list = iter->second;

    for(auto list_iter = list.begin(); list_iter != list.end(); ++list_iter)
    {
        ST_AlarmLevelDef & def = *list_iter;

        /*--
        gAlmLog->DEBUG("! [%s:%d] [%s:%s] MID [%d] CODE[%s] T[%s] NODE T[%s] first[%d], second[[%d], [%d] [%d]",
            __FILE__,
            __LINE__,
            _target.c_str(),
            _node_type.c_str(),
            def.monitor_id_,
            def.code_.c_str(),
            def.target_.c_str(),
            def.node_type_.c_str(),
            def.target_.compare(_target),
            def.node_type_.compare(_node_type),
            def.target_.size(),
            def.node_type_.size());
        --*/

        if(def.target_.compare(_target) == 0 &&
           def.node_type_.compare(_node_type) == 0)
           return &def;

        if(def.target_.size() > 0 &&
           def.target_.compare(_target) != 0)
            continue;

        if(def.node_type_.size() > 0 &&
           def.node_type_.compare(_node_type) != 0)
            continue;

        /*--
        gAlmLog->DEBUG("[%s:%d] CODE[%s] T[%s] NODE T[%s]",
            __FILE__,
            __LINE__,
            def.code_.c_str(),
            def.target_.c_str(),
            def.node_type_.c_str());
        -*/

        ret = &def;
    }

    return ret;
}

ALM::eSEVERITY AlarmLevelConf::
GetSeverity(std::string & _node_type,
            std::string & _code,
            std::string & _target,
            std::string & _value,
            std::string & _complement)
{

    ST_AlarmLevelDef * sp = getAlarmLevelDef(_node_type, _code, _target);

    if(sp == NULL)
        return ALM::eINDETERMINATE;

    std::list<ST_Level> & list = sp->list_;

    if(_complement.size() <= 0)
        _complement =  sp->complement_;

    return decideSeverity(_value, list, sp->range_);
}

ALM::eSEVERITY AlarmLevelConf::
decideSeverity(std::string         & _value,
               std::list<ST_Level> & _list,
               bool                  _is_range)
{
    ALM::eSEVERITY  _ret = ALM::eCLEARED;

    float   v      = atof(_value.c_str());
    float   object = 0.0;

    for(auto iter = _list.begin(); iter != _list.end(); ++iter)
    {
        if(_is_range)
        {
            /*--
            gAlmLog->DEBUG("------ [%s:%d] VALUE [%s] DIFF [%s]",
            __FILE__,
            __LINE__,
            _value.c_str(),
            (*iter).greater_.c_str());
            --*/

            object = atof((*iter).greater_.c_str());
            if(v < object)
                break;

            _ret = (*iter).severity_;
        }
        else
        {
            /*--
            gAlmLog->DEBUG("------ [%s:%d] VALUE [%s] DIFF [%s]",
            __FILE__,
            __LINE__,
            _value.c_str(),
            (*iter).greater_.c_str());
            --*/

            if(_value.compare((*iter).greater_) == 0)
                return (*iter).severity_;
        }
    }

    return _ret;
}


