
#include <vector>

#include "CommonCode.hpp"
#include "AlarmSyncEventHandler.hpp"
#include "AlarmCFG.hpp"
#include "FetchMaria.hpp"
#include "AlarmDuplication.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

AlarmSyncEventHandler::
AlarmSyncEventHandler()
{
    // Empty
}

AlarmSyncEventHandler::
~AlarmSyncEventHandler()
{
    // Empty
}

bool AlarmSyncEventHandler::
Init()
{
    // Empty
    return true;
}

void AlarmSyncEventHandler::
Clear()
{
}

bool AlarmSyncEventHandler::
DecodingJSON()
{
    return true;
}

// IF 를 위함
bool AlarmSyncEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    return true;
}

bool AlarmSyncEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    std::vector<ST_AlarmStatus>  vec(10);

    gatherSameNodeAlarm(vec,
                        _db,
                        _nm_if.GetRequestNodeNo());

    if(vec.size() > 0)
    {
        AlarmDuplication    duplication;
        std::string         temp;
        duplication.MakeBodyForNotify(temp, vec);

        return _nm_if.Response(temp);
    }

    return true;
}

void AlarmSyncEventHandler::
gatherSameNodeAlarm(std::vector<ST_AlarmStatus> & _vec,
                    DB                          * _db,
                    int                           _node_no)
{
    _vec.clear();

    char sql[256];
    sprintf(sql,
        "SELECT MSG_ID, PROC_NO, CODE, SEVERITY_CCD, LOCATION, TARGET, COMPLEMENT "
        " FROM TAT_ALM_STATUS "
        " WHERE NODE_NO=%d",
        _node_no);

    FetchMaria  f;
    if(_db->Query(&f, sql, strlen(sql)) <= 0)
        return ;

    char    msg_id[16];
    char    proc_no[16];
    char    code[DB_ALM_CODE_SIZE+1];
    char    severity_id[4];
    char    location[DB_ALM_LOCATION_SIZE+1];
    char    target[DB_ALM_OBJECT_SIZE+1];
    char    complement[DB_ALM_COMPLEMENT_SIZE+1];

    f.Clear();
    f.Set(msg_id,       sizeof(msg_id));
    f.Set(proc_no,      sizeof(proc_no));
    f.Set(code,         sizeof(code));
    f.Set(severity_id,  sizeof(severity_id));
    f.Set(location,     sizeof(location));
    f.Set(target,       sizeof(target));
    f.Set(complement,   sizeof(complement));

    while(f.Fetch() == true)
    {
        ST_AlarmStatus  stAlarmStatus;

        stAlarmStatus.Clear();
        stAlarmStatus.node_no_      = _node_no;
        stAlarmStatus.proc_no_      = atoi(proc_no);

        stAlarmStatus.msg_id_       = atoi(msg_id);
        stAlarmStatus.code_         = code;
        stAlarmStatus.severity_id_  = atoi(severity_id);
        stAlarmStatus.location_     = location;
        stAlarmStatus.target_       = target;
        stAlarmStatus.complement_   = complement;

        _vec.push_back(stAlarmStatus);
    }
}
