

#include "AlarmDuplication.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CFileLog.hpp"

extern CFileLog * gAlmLog;

AlarmDuplication::
AlarmDuplication()
{
    map_.clear();
}

AlarmDuplication::
~AlarmDuplication()
{
    // Empty
}

// For ALA
bool AlarmDuplication::
Is(ST_AlarmEventMsg & _stAlarmEventMsg)
{

    // Connection 은 Dup 체크를 하지 않습니다.
    if(_stAlarmEventMsg.code_.compare("90200102") == 0)
        return false;

    std::string     super_key;

    MakeSuperKey(super_key, _stAlarmEventMsg);

    auto iter = map_.find(super_key);

    if(iter == map_.end())
    {
        if(_stAlarmEventMsg.severity_id_ == ALM::eCLEARED)
        {
            gAlmLog->DEBUG("%-24s| Is - No Alarm",
                "AlarmDuplication");

            return true;
        }

        map_[super_key] = _stAlarmEventMsg.severity_id_;
        return false;
    }

    if(iter->second == _stAlarmEventMsg.severity_id_)
    {
        gAlmLog->DEBUG("%-24s| Is - Dulplicated",
            "AlarmDuplication");
        return true;
    }

    if(_stAlarmEventMsg.severity_id_ == ALM::eCLEARED)
        map_.erase(iter);

    return false;
}

void AlarmDuplication::
MakeSuperKey(std::string      & _super_key,
             ST_AlarmEventMsg & _stAlarmEventMsg)
{
    _super_key = _stAlarmEventMsg.code_;

    _super_key.append(std::to_string((long long)_stAlarmEventMsg.proc_no_));
    _super_key.append(_stAlarmEventMsg.location_);
    _super_key.append(_stAlarmEventMsg.target_);
    _super_key.append(_stAlarmEventMsg.complement_);
}

void AlarmDuplication::
MakeSuperKey(std::string    & _super_key,
             ST_AlarmStatus & _stAlarmStatus)
{
    _super_key = _stAlarmStatus.code_;

    _super_key.append(std::to_string((long long)_stAlarmStatus.proc_no_));
    _super_key.append(_stAlarmStatus.location_);
    _super_key.append(_stAlarmStatus.target_);
    _super_key.append(_stAlarmStatus.complement_);
}

// For ALM
void AlarmDuplication::
MakeBodyForNotify(std::string & _out,
                  std::vector<ST_AlarmStatus> & _vec)
{
    rabbit::object  root;
    rabbit::object  body = root["BODY"];

    std::string     super_key;
    rabbit::array   metrix = body["metrix"];
    for(auto iter = _vec.begin(); iter != _vec.end(); ++iter)
    {
        MakeSuperKey(super_key, *iter);

        rabbit::object  item;
        item["superkey"] = super_key;
        item["severity"] = (*iter).severity_id_;
        metrix.push_back(item);
    }

    _out = root.str();
}

// For ALA
bool AlarmDuplication::
Manage(std::string & _msg)
{
    /*--
    "BODY": {
        "metrx"  : [
                     {"superkey":severity, "severity:"
                     {"superkey":severity,
                     {"superkey":severity,
                    ...
                    ]
    }

    --*/

    try {

        rabbit::document    doc;
        doc.parse(_msg);

        rabbit::object  body = doc["BODY"];

        rabbit::array   a = body["metrix"];
        rabbit::value   v;

        std::string     key;
        int             severity_id;

        for(auto nLoop=0u; nLoop < a.size(); ++nLoop)
        {
            v           = a.at(nLoop);
            key         = v["superkey"].as_string();
            severity_id = v["severity"].as_int();

            if(severity_id == ALM::eCLEARED)
            {
                map_.erase(key);
                continue;
            }

            auto iter = map_.find(key);

            if(iter != map_.end())
                iter->second = severity_id;
            else
                map_[key] = severity_id;
        }

    } catch(rabbit::type_mismatch   e) {

        gAlmLog->WARNING("%-24s| getSuperKey - type mismatch [%s]",
            "AlarmDuplication",
            _msg.c_str());
        return false;
    } catch(rabbit::parse_error e) {

        gAlmLog->WARNING("%-24s| getSuperKey - parse error [%s]",
            "AlarmDuplication",
            _msg.c_str());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| getSuperKey - [%s]",
            "AlarmDuplication",
            _msg.c_str());
        return false;
    }

    return true;
}
