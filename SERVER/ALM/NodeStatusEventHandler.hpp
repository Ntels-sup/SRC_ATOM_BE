#ifndef __NODE_STATUS_EVENT_HANDLER_HPP__
#define __NODE_STATUS_EVENT_HANDLER_HPP__

#include <string>
#include <vector>
#include <unordered_map>

#include "EventHandler.hpp"
#include "AlarmEventHandler.hpp"

#include "AlarmDataDef.hpp"
#include "AlarmDecision.hpp"

// NM 으로부터 메시지를 수신하여, AlarmEventMsg 를 만들어서 AlarmEventHandler 에게 전달한다.

class NodeStatusEventHandler : public EventHandler
{
public:
    NodeStatusEventHandler();
    virtual ~NodeStatusEventHandler();

    bool            Init(DB                 * _db,
                         AlarmEventHandler  * _alarm_h);

    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();

    int             GetNodeId()     { return 0; }
    unsigned int    GetSequenceId() { return 0; }
    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);

    bool            Do(NodeManagerIF & _nm_if, DB * _db);

private :
    bool            initAlarmDecisionMap(DB * _db);
    bool            getPkgNameAndNodeType(std::vector<std::pair<std::string,
                                                                std::string> > & _vec,
                                          DB  * _db);

    bool            fillAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg);
    void            notify(NodeManagerIF & _nm_if);


private:

    AlarmEventHandler *         alarm_h_;
    std::string                 buffer_;

    ST_AlarmEventMsg            stAlarmEventMsg_;


    // key : pkg_name + node_type
    std::unordered_map<std::string, AlarmDecision>  map_alarm_decision_;

};


#endif // __NODE_STATUS_EVENT_HANDLER_HPP__
