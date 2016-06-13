#ifndef __SUBSCRIBE_EVENT_HANDLER_HPP__
#define __SUBSCRIBE_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include <unordered_map>
#include <list>

class SubscribeEventHandler : public EventHandler
{
public:
    SubscribeEventHandler();
    virtual ~SubscribeEventHandler();

    bool            Init(DB * _db);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    // 의미 없음 - interface 를 맞추기 위함 //
    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);
    //////////////

    bool            Do(NodeManagerIF & _nm_if, DB * _db);


    int             Notify(NodeManagerIF   & _nm_if,
                           ST_AlarmStatus   & _stAlarmStatus,
                           std::string      & _body);

private:
    void            addSubscriber(ST_Subscriber & _stSubscriber);
    void            delSubscriber(ST_Subscriber & _stSubscriber);


private:

    std::string             buffer_;
    ST_Subscriber           stSubscriber_;

    std::unordered_map<std::string, std::list<ST_Subscriber> >  map_;

};

#endif // __SUBSCRIBE_EVENT_HANDLER_HPP__
