

#ifndef     __ALARM_STS_HPP__
#define     __ALARM_STS_HPP__

#include <string>
#include <vector>
#include <array>
#include <unordered_map>

#include "CommonCode.hpp"
#include "DB.hpp"

class AlarmSTS
{
public:
    AlarmSTS();
    ~AlarmSTS();

    void Add(int           _node_no,
             int           _severity_id,
             std::string & _prc_date,
             std::string & _dst_yn);
    void Save(DB * _db);

private:
    typedef std::array<unsigned int, ALM::eTOTAL_CNT>  arr_severity_t;

private:
    void makeSQL(char *             _sql,
                std::string &       _key,
                arr_severity_t &    _arr);
    bool isOnTime();

private:

    int saved_min_;

    // key : prc_date + dst_yn + node_no
    std::unordered_map<std::string, arr_severity_t>  map_;
};

#endif //   __ALARM_STS_HPP__

