#ifndef __TCA_ALARM_EVENT_HANDLER_HPP__
#define __TCA_ALARM_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "AlarmEventHandler.hpp"

#include "AlarmDataDef.hpp"
#include "AlarmCodeConf.hpp"

class TcaAlarmEventHandler : public EventHandler
{
public:
    TcaAlarmEventHandler();
    virtual ~TcaAlarmEventHandler();

    bool            Init(DB * _db, AlarmEventHandler * _alarm_h);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    // 사용하지 않음.
    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);

    bool            Do(NodeManagerIF & _nm_if, DB * _db);


private:

    AlarmEventHandler     * alarm_h_;
    std::string             buffer_;

    ST_AlarmEventMsg        stAlarmEventMsg_;

    AlarmCodeConf           alarm_code_conf_;
};

#endif // __TCA_ALARM_EVENT_HANDLER_HPP__
