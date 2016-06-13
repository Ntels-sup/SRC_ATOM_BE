
#ifndef __USER_CHANGE_EVENT_HANDLER_HPP__
#define __USER_CHANGE_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "AlarmEventHandler.hpp"

class UserChangeEventHandler : public EventHandler
{
public:
    UserChangeEventHandler();
    virtual ~UserChangeEventHandler();

    bool            Init(AlarmEventHandler     * _alarm_h);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    // 의미 없음 - interface 맞추기 위함 //
    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);
    /////////////////////////////////////

    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private:
    bool            makeAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg,
                                      ST_UserChangeMsg & _stUserChange,
                                      DB               * _db);
    bool            response(NodeManagerIF & _nm_if,
                             bool            _result,
                             const char *    _reason);


private:

    AlarmEventHandler     *     alarm_h_;

    std::string                 buffer_;

    ST_UserChangeMsg            stUserChangeMsg_;

};



#endif // __USER_CHANGE_EVENT_HANDLER_HPP__
