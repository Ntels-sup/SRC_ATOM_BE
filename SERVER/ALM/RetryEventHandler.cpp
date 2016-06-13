#include "CommonCode.hpp"
#include "RetryEventHandler.hpp"
#include "AlarmCFG.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

RetryEventHandler::
RetryEventHandler()
{
    alarm_h_            = NULL;
    subscribe_h_        = NULL;
}

RetryEventHandler::
~RetryEventHandler()
{
    // Empty
}


bool RetryEventHandler::
Init(AlarmEventHandler           * _alarm_h,
     SubscribeEventHandler      * _subscribe_h)
{
    alarm_h_            = _alarm_h;
    subscribe_h_        = _subscribe_h;

    return true;
}

void RetryEventHandler::
Clear()
{
    buffer_.clear();
}

bool RetryEventHandler::
DecodingJSON()
{
    return alarm_h_->DecodingJSON(stAlarmEventMsg_, buffer_);
}

bool RetryEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    // 강제로 MSG ID 를 맞춰야 함.
    _sequence_by_node.SetId(stAlarmEventMsg_.node_no_,
                            stAlarmEventMsg_.sequence_id_);
    return true;
}

bool RetryEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
   return alarm_h_->Do(_nm_if, _db);
}