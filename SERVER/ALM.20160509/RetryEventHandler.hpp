#ifndef __RETRY_EVENT_HANDLER_HPP__
#define __RETRY_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "AlarmEventHandler.hpp"
#include "SubscribeEventHandler.hpp"

#include "AlarmDataDef.hpp"


class RetryEventHandler : public EventHandler
{
public:
    RetryEventHandler();
    virtual ~RetryEventHandler();

    bool            Init(AlarmEventHandler          * _alarm_h,
                         SubscribeEventHandler      * _subscribe_h);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);

    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private:

    std::string                 buffer_;

    AlarmEventHandler *         alarm_h_;
    SubscribeEventHandler *     subscribe_h_;
};

#endif //__RETRY_EVENT_HANDLER_HPP__
