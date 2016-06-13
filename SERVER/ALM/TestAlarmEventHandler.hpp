
#ifndef __TEST_ALARM_EVENT_HANDLER_HPP__
#define __TEST_ALARM_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "SubscribeEventHandler.hpp"

class TestAlarmEventHandler : public EventHandler
{
public:
    TestAlarmEventHandler();
    virtual ~TestAlarmEventHandler();

    bool            Init(SubscribeEventHandler * _subscribe_h);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();
    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }

    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);
    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private:

    SubscribeEventHandler * subscribe_h_;

    std::string             buffer_;

    ST_AlarmEventMsg        stAlarmEventMsg_;
    ST_AlarmStatus          stAlarmStatus_;
};



#endif // __TEST_ALARM_EVENT_HANDLER_HPP__
