
#include "TestAlarmEventHandler.hpp"

#include "AlarmCFG.hpp"

extern AlarmCFG     gCFG;

TestAlarmEventHandler::
TestAlarmEventHandler()
{
    subscribe_h_ = NULL;
    buffer_.resize(gCFG.DEF.buffer_size_);
}


TestAlarmEventHandler::
~TestAlarmEventHandler()
{
    // Empty
}

// 처리 말미에
bool TestAlarmEventHandler::
Init(SubscribeEventHandler * _subscribe_h)
{
    DEBUG_PRINT("inside init\n");

    subscribe_h_ = _subscribe_h;

    return true;
}

void TestAlarmEventHandler::
Clear()
{
    DEBUG_PRINT("inside Clear\n");

    buffer_.clear();
}

bool TestAlarmEventHandler::
DecodingJSON()
{
    DEBUG_PRINT("inside DecodingJSON\n");
    return true;
}

bool TestAlarmEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    // Test 에서는  Sequence Id 를 증가하면 안되요..
    // Alarm History 에 저장하지 않기 때문에, 증가시키면 Alarm Hist 에서는 찾을 수 없어요.
    return true;
}

bool TestAlarmEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    DEBUG_PRINT("inside Do\n");

    // 여기서 alarm_h_ 를 가지고 적정하게 처리합니다... DB 에 적용 안 받게..
    // AlarmStatus 를 만들어서,

    stAlarmStatus_.Clear();
    stAlarmStatus_.SetFromAlarmEventMsg(stAlarmEventMsg_);

    subscribe_h_->NotifyToWSM(_nm_if, stAlarmStatus_);

    return true;

}

