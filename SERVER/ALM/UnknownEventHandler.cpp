
#include "UnknownEventHandler.hpp"

#include "AlarmCFG.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

UnknownEventHandler::
UnknownEventHandler()
{
    buffer_.resize(gCFG.DEF.buffer_size_);
}


UnknownEventHandler::
~UnknownEventHandler()
{
    // Empty
}

// 처리 말미에
bool UnknownEventHandler::
Init()
{
    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "UnknownEventHandler");
    return true;
}

void UnknownEventHandler::
Clear()
{
    buffer_.clear();
}

bool UnknownEventHandler::
DecodingJSON()
{
    return true;
}

int UnknownEventHandler::
GetNodeId()
{
    return 0;
}

unsigned int UnknownEventHandler::
GetSequenceId()
{
    return 0;
}

bool UnknownEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    // 정밀하게 처리가 필요 합니다.
    // Unkown 이여도 체크하도록 합니다. 그래야 구멍이 없어요..
    // 그러나, 어떤 메시지인지 알지 못하는데 어떻게 체크를 해요.. ㅠ.ㅜ
    // 아무 Action 하지 않는 것이 맞지요..

    return true;
}

bool UnknownEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    gAlmLog->WARNING("%-24s| Do",
        "UnknownEventHandler");

    return true;
}

