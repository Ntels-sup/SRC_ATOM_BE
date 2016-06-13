
#include "CommonCode.hpp"
#include "AlarmSyncEventHandler.hpp"
#include "AlarmCFG.hpp"

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
    buffer_.clear();
    stAlarmStatus_.Clear();
}

bool AlarmSyncEventHandler::
DecodingJSON()
{
    using namespace rapidjson;

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
    return true;
}

