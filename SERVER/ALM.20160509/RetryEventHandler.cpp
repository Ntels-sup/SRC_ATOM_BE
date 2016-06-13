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
    using namespace rapidjson;

    return true;
}

// IF 를 맞추기 위함.
bool RetryEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    return true;
}

bool RetryEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{

   return true;
}