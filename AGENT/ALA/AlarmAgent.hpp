
#ifndef __ALARM_AGENT_HPP__
#define __ALARM_AGENT_HPP__

#include <string>

#include "TimeOutCheck.hpp"
#include "AlaInterface.hpp"
#include "AlarmDecision.hpp"
#include "AlarmDuplication.hpp"
#include "AuditData.hpp"
#include "AlarmRetry.hpp"
#include "VnfmAlarm.hpp"
#include "CommonCode.hpp"
#include "AlarmDataDef.hpp"

class AlarmAgent
{
public:
    AlarmAgent();
    ~AlarmAgent();

    bool    Init(void * _pArg);
    void    Do();
    void    Final();

private:
    void    setAlarmMsgFromProfile(ST_AlarmEventMsg & _stAlarmEventMsg,
                                   ST_Profile       & _profile);
    void    setAlarmMsgFromInterface(ST_AlarmEventMsg & _stAlarmEventMsg,
                                     AlaInterface    & _interface);
    bool    convertToAlarmMsg(ST_AlarmEventMsg & _stAlarmEventMsg,
                              AlaInterface     & _interface);
    bool    sendToVnfm(ST_AlarmEventMsg & _stAlarmEventMsg,
                       AlaInterface     & _interface);
    void    setSequenceId(ST_AlarmEventMsg & _stAlarmEventMsg,
                          AuditData        & _audit);
    void    makeBody(std::string      & _body,
                     ST_AlarmEventMsg & _stAlarmEventMsg);

    bool    isInternalMsg(AlaInterface    & _interface);
    bool    procInternalMsg(AlaInterface  & _interface);
    bool    sendSyncRequest();


private:

    bool                        bInitialized_;
    TimeOutCheck                timer_;

    AlaInterface                interface_;
    AlarmDecision               alarm_decision_;
    AlarmDuplication            duplication_;
    AuditData                   audit_;
    AlarmRetry                  retry_;
    VnfmAlarm                   vnfm_alarm_;

    ST_AlarmEventMsg            stAlarmEventMsg_;
    std::string                 body_;

};


#endif // __ALARM_AGENT_HPP__
