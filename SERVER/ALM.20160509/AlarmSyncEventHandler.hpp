
#ifndef __ALARM_SYNC_EVENT_HANDLER_HPP__
#define __ALARM_SYNC_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "AlarmDataDef.hpp"

class AlarmSyncEventHandler : public EventHandler
{
public:
    AlarmSyncEventHandler();
    virtual ~AlarmSyncEventHandler();

    bool            Init();
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);

    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private:

    std::string             buffer_;
    ST_AlarmStatus          stAlarmStatus_;
};

#endif // __ALARM_SYNC_EVENT_HANDLER_HPP__