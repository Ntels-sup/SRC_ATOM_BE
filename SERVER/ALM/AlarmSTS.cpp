
#include "AlarmSTS.hpp"
#include "CFileLog.hpp"

#include <algorithm>
#include <time.h>

extern CFileLog *       gAlmLog;

AlarmSTS::
AlarmSTS()
{
    saved_min_ = time(NULL) / 60;
    map_.clear();
}

AlarmSTS::
~AlarmSTS()
{
    // Empty
}


void AlarmSTS::
Add(int _node_no, int _severity_id, std::string & _prc_date, std::string & _dst_yn)
{
    if(_severity_id >= ALM::eTOTAL_CNT)
    {
        gAlmLog->WARNING("%-24s| severity id is overflow [total:%d] [recved:%d]",
            "AlarmSTS",
            ALM::eTOTAL_CNT,
            _severity_id);

        return ;
    }

    std::string     key;
    key.reserve(64);

    // 분단위까지만
    key.assign(_prc_date.begin(), _prc_date.begin()+_prc_date.rfind(":"));
    key.append(";");
    key.append(_dst_yn);
    key.append(";");
    key.append(std::to_string((long long)_node_no));


    auto iter = map_.find(key);

    if(iter == map_.end())
    {
        arr_severity_t  arr = {{0}};

        arr[_severity_id]++;

        map_[key] = arr;
    }
    else
        (iter->second)[_severity_id]++;

}


void AlarmSTS::
Save(DB * _db)
{
    if(isOnTime() == false)
        return ;

    char sql[630];
    std::string     key;

    for(auto iter = map_.begin(); iter != map_.end(); ++iter)
    {
        key = iter->first;
        arr_severity_t &   arr = iter->second;
        makeSQL(sql, key, arr);

        if(_db->Execute(sql, strlen(sql)) < 0)
        {
            gAlmLog->WARNING("%-24s| Save - DB Execute fail [%s]",
                "AlarmSTS",
                sql);
        }
    }

    map_.clear();
}

bool AlarmSTS::
isOnTime()
{
    int now_min = time(NULL) / 60;

    if(saved_min_ == now_min)
        return false;

    saved_min_ = now_min;
    return true;
}

void AlarmSTS::
makeSQL(char *              _sql,
        std::string &       _key,
        arr_severity_t &    _arr)
{

    std::string         timestamp(_key.begin(), _key.begin() + _key.find(";"));
    char                dstYn   = _key.at(_key.find(";") + 1);
    std::string         node_str= _key.substr(_key.rfind(";")+1);

    sprintf(_sql,
        "INSERT INTO TAT_STS_ALM "
        "(PRC_DATE, DST_YN, NODE_NO,"
        " INDETERMINATE, CRITICAL, MAJOR, MINOR, WARNING, CLEARED, FAULT, NOTICE) "
        "VALUES ('%s:00', '%c', %s, "
        " %d, %d, %d, %d, %d, %d, %d, %d) "
        " on duplicate key "
        " UPDATE INDETERMINATE=INDETERMINATE+%d, CRITICAL=CRITICAL+%d, "
        " MAJOR=MAJOR+%d, MINOR=MINOR+%d, "
        " WARNING=WARNING+%d, CLEARED=CLEARED+%d,"
        " FAULT=FAULT+%d, NOTICE=NOTICE+%d ",
        timestamp.c_str(),
        dstYn,
        node_str.c_str(),
        _arr[0],
        _arr[1],
        _arr[2],
        _arr[3],
        _arr[4],
        _arr[5],
        _arr[6],
        _arr[7],
        _arr[0],
        _arr[1],
        _arr[2],
        _arr[3],
        _arr[4],
        _arr[5],
        _arr[6],
        _arr[7]);
}