#ifndef __ALARM_EVENT_HANDLER_HPP__
#define __ALARM_EVENT_HANDLER_HPP__

#include "EventHandler.hpp"
#include "SubscribeEventHandler.hpp"

#include "AlarmDataDef.hpp"
#include "AlarmDecision.hpp"
#include "MessageId.hpp"

class AlarmEventHandler : public EventHandler
{
public:
    AlarmEventHandler();
    virtual ~AlarmEventHandler();

    bool            Init(DB * _db, SubscribeEventHandler * _subscribe_h);
    void            Clear();

    std::string &   Buffer()        { return buffer_; }
    bool            DecodingJSON();
    int             GetNodeId()     { return stAlarmEventMsg_.node_no_; }
    unsigned int    GetSequenceId() { return stAlarmEventMsg_.sequence_id_; }

    bool            IsValidSequenceId(SequenceByNode & _sequence_by_node);
    bool            Do(NodeManagerIF & _nm_if, DB * _db);

    void            DoIfConnectStatus(ST_AlarmStatus & _stAlarmStatus,
                                      DB             * _db,
                                      NodeManagerIF  & _nm_if);
    void            DoIfPingStatus(ST_AlarmStatus & _stAlarmStatus,
                                   DB             * _db,
                                   NodeManagerIF  & _nm_if);
    void            DoIfProcessStatus(ST_AlarmStatus & _stAlarmStatus,
                                      DB             * _db,
                                      NodeManagerIF  & _nm_if);
    void            DoIfNodeStatus(ST_AlarmStatus & _stAlarmStatus,
                                   DB             * _db,
                                   NodeManagerIF  & _nm_if);

    // For UserChangeEventHandler
    void            SetAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg);


private:
    bool            applyAlarmStatus(NodeManagerIF & _nm_if, DB * _db);
    bool            applyAlarmHistory(DB * _db);

    bool            findLeaderAlarmMsg(ST_AlarmStatus &  _stLeaderAlarmStatus,
                                       DB             *  _db,
                                       ST_AlarmStatus &  _stAlarmStatus);
    void            addBlackList(NodeManagerIF & _nm_if);
    bool            updateHistoryToClear(DB             * _db,
                                         unsigned int     _associated_msg_id,
                                         ST_AlarmStatus & _stAlarmStatus);
    bool            updateLeaderAlarm(DB             * _db,
                                      unsigned int     _to_msg_id,
                                      ST_AlarmStatus & _stAlarmStatus);

    std::string     makeBodyForNotify(ST_AlarmStatus & _stAlarmStatus);

    bool            updateConnectStatus(DB             * _db,
                                        ST_AlarmStatus & _stAlarmStatus);
    bool            insertConnectStatus(DB * _db,
                                        ST_AlarmStatus & _stAlarmStatus);
    bool            updatePingStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus);
    bool            insertPingStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus);
    bool            updateProcessStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus);
    void            gatherSameProcessFromStatusTBL(std::vector<ST_AlarmStatus> & _vec,
                                                   DB                          * _db,
                                                   int                           _node_no,
                                                   int                           _proc_no);
    bool            updateNodeStatus(DB             * _db,
                                     ST_AlarmStatus & _stAlarmStatus);
    void            gatherSameNodeFromStatusTBL(std::vector<unsigned int> & _vec,
                                                DB                        * _db,
                                                int                         _node_no);
    void            setCCD(std::unordered_map<int, std::string> & _map,
                           const char                           * _group_cd,
                           DB                                   * _db);

    bool            sendToALAForSync(std::vector<ST_AlarmStatus> & _vec,
                                     int                           _node_no,
                                     int                           _proc_no,
                                     NodeManagerIF               & _nm_if);

    void            makeAlarmStatusForDBFail(ST_AlarmStatus & _stAlarmStatus,
                                             const char     * _value);

private:

    SubscribeEventHandler * subscribe_h_;
    std::string             buffer_;

    MessageId               mid_;

    ST_AlarmEventMsg        stAlarmEventMsg_;
    ST_AlarmStatus          stAlarmStatus_;
    ST_AlarmHistory         stAlarmHistory_;

    // TO DO :
    // NbiIF                nbi_if_;
    // SmsIF                sms_if_;
    // EmailIF              email_if_;


    std::unordered_map <int, std::string>   map_group_ccd_;
    std::unordered_map <int, std::string>   map_type_ccd_;

    AlarmDecision           alarm_decision_;
    bool                    set_db_alarm_;

};

#endif // __ALARM_EVENT_HANDLER_HPP__
