
#include "AlarmAgent.hpp"
#include "MessageConverter.hpp"
#include "CFileLog.hpp"
#include "CommandFormat.hpp"
#include "AlaCFG.hpp"
#include "AlarmEncoding.hpp"

#include "AlarmSyslog.hpp"

#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

extern CFileLog *       gAlmLog;
extern AlaCFG           gAlaCFG;

AlarmAgent::
AlarmAgent()
{
    bInitialized_ = false;
}

AlarmAgent::
~AlarmAgent()
{
    // Empty
}

bool AlarmAgent::
Init(void * _pArg)
{
    if(bInitialized_ == true)
        return true;

    // Init 이 실패하면, 10초마다 1회씩 Init 을 시도합니다.
    if(timer_.TimeOut(10) == false)
        return false;
    else
        timer_.Update();

    CModule::ST_MODULE_OPTIONS stOption =
        *static_cast<CModule::ST_MODULE_OPTIONS*>(_pArg);

    if(gAlaCFG.Init(stOption.m_szCfgFile) != true)
    {
        if(gAlmLog != NULL)
            gAlmLog->ERROR("%-24s| Init - Config fail",
                "AlarmAgent");
        return false;
    }

    if(gAlaCFG.SetProfile(stOption) == false)
        return false;

    if(stOption.m_pclsDB == NULL)
    {
        gAlmLog->ERROR("%-24s| Init - DB instancle is null",
            "AlarmAgent");
        return false;
    }

    DB * db = stOption.m_pclsDB;

    if(alarm_decision_.Init(db, gAlaCFG.profile_) == false)
    {
        gAlmLog->ERROR("%-24s| Init - alarm decision fail",
            "AlarmAgent");
        return false;
    }

    vnfm_alarm_.Init(db, gAlaCFG.profile_.m_strPkgName.c_str());

    duplication_.Clear();

    if(audit_.Init() == false)
    {
        gAlmLog->ERROR("%-24s| Init - audit init fail",
            "AlarmAgent");

        return false;
    }

    if(retry_.Init(gAlaCFG.profile_.m_nNodeNo,
                   gAlaCFG.profile_.m_strNodeName,
                   gAlaCFG.profile_.m_nProcNo) == false)
    {
        gAlmLog->ERROR("%-24s| Init - retry init fail",
            "AlarmAgent");

        return false;
    }

    if(interface_.Init(gAlaCFG.profile_,
                       stOption.m_pclsModIpc,
                       stOption.m_pclsAddress) == false)
    {
        gAlmLog->ERROR("%-24s| Init - ala interface init fail",
            "AlarmAgent");
        return false;
    }

    // Sync 요청 메시지를 전달해야 해요.
    if(interface_.SendSync() == false)
    {
        gAlmLog->ERROR("%-24s| Init - ala send sync fail",
            "AlarmAgent");

        return false;
    }

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "AlarmAgent");

    bInitialized_ = true;
    return true;
}


void AlarmAgent::
Do()
{
    for(auto nLoop=0; nLoop < 30; nLoop++)
    {

        if(retry_.IsContinues() == true)
        {
            retry_.Response(interface_, audit_);
            break;
        }

        if(interface_.GetData() == false)
            break;

        if(isInternalMsg(interface_) == true)
        {
            procInternalMsg(interface_);
            continue;
        }

        stAlarmEventMsg_.Clear();

        setAlarmMsgFromProfile(stAlarmEventMsg_,   gAlaCFG.profile_);
        setAlarmMsgFromInterface(stAlarmEventMsg_, interface_);

        if(convertToAlarmMsg(stAlarmEventMsg_, interface_) == false)
        {
            gAlmLog->WARNING("%-24s| Do - can't convert to alarm msg",
                "AlarmAgent");
            continue;
        }

        if(alarm_decision_.Do(stAlarmEventMsg_) == false)
            continue;

         if(duplication_.Is(stAlarmEventMsg_) == true)
            continue;

        sendToVnfm(stAlarmEventMsg_, interface_);

        setSequenceId(stAlarmEventMsg_, audit_);
        AlarmEncoding::MakeBody(body_, stAlarmEventMsg_);
        audit_.Save(stAlarmEventMsg_.sequence_id_, body_);

		// Add For Send Syslog Message to VNFM by weawen 20160616
        AlarmSyslog::SendSyslog( stAlarmEventMsg_ );

        if(interface_.SendAlarm(body_) == false)
        {
            gAlmLog->WARNING("%-24s| Do - can't send msg id[%u]",
                "AlarmAgent",
                stAlarmEventMsg_.sequence_id_);
        }
    }
}

bool AlarmAgent::
isInternalMsg(AlaInterface & _interface)
{
    switch(_interface.GetCommand())
    {
    case CMD_MANAGE_DUPLICATE:
    case CMD_ALM_RETRY_REQ:
    case CMD_ALM_SYNC_REQ:
        return true;
    default:
        ;
    }

    return false;
}

bool AlarmAgent::
procInternalMsg(AlaInterface & _interface)
{
    body_.clear();

    _interface.TakeOutBody(body_);

    switch(_interface.GetCommand())
    {
    case CMD_ALM_SYNC_REQ:
    case CMD_MANAGE_DUPLICATE:
        return duplication_.Manage(body_);
    case CMD_ALM_RETRY_REQ:
        return retry_.Request(body_, audit_);

    default:
        ;
    }

    return false;
}

void AlarmAgent::
setAlarmMsgFromProfile(ST_AlarmEventMsg & _stAlarmEventMsg,
                       ST_Profile       & _profile)

{
    _stAlarmEventMsg.location_ =    "/";
    _stAlarmEventMsg.location_.append(_profile.m_strPkgName);
    _stAlarmEventMsg.location_.append("/");
    _stAlarmEventMsg.location_.append(_profile.m_strNodeName);
    _stAlarmEventMsg.location_.append("/");

    _stAlarmEventMsg.node_version_  = _profile.m_strNodeVersion;
    _stAlarmEventMsg.node_type_     = _profile.m_strNodeType;
    _stAlarmEventMsg.pkg_name_      = _profile.m_strPkgName;
    _stAlarmEventMsg.node_name_     = _profile.m_strNodeName;

}

void AlarmAgent::
setAlarmMsgFromInterface(ST_AlarmEventMsg & _stAlarmEventMsg,
                          AlaInterface    & _interface)
{
    _interface.GetSource(_stAlarmEventMsg.node_no_,
                         _stAlarmEventMsg.proc_no_);
}

bool AlarmAgent::
convertToAlarmMsg(ST_AlarmEventMsg & _stAlarmEventMsg,
                  AlaInterface     & _interface)
{
    body_.clear();

    _interface.TakeOutBody(body_);

    switch(_interface.GetCommand())
    {
    case    CMD_ALM_EVENT:
            return MessageConverter::Alarm(_stAlarmEventMsg, body_);

    case    CMD_STATUS_PROC_EVENT:
            return MessageConverter::Process(_stAlarmEventMsg, body_);

    case    CMD_STATUS_CONNECT_EVENT:
            return MessageConverter::Connect(_stAlarmEventMsg, body_);

    case    CMD_STATUS_PING_EVENT:
            return MessageConverter::Ping(_stAlarmEventMsg, body_);

    default:
        gAlmLog->WARNING("%-24s| convertToAlarmMsg - Unknown Command Code [%d]",
            "AlarmAgent",
            _interface.GetCommand());
    }

    return false;
}

bool AlarmAgent::
sendToVnfm(ST_AlarmEventMsg & _stAlarmEventMsg,
           AlaInterface     & _interface)
{
    // TO DO :
    //    Send 하면, vnfm_yn 에 값 변경해야 합니다.

    std::string     body;

    if(vnfm_alarm_.MakeBody(body, _stAlarmEventMsg) == false)
        return false;

    if(_interface.SendToEXA(body) == false)
    {
        gAlmLog->WARNING("%-24s| sendToVnfm - send fail [%s] [%s]",
            "AlarmAgent",
            _interface.GetErrorMsg(),
            body.c_str());

        return false;
    }

    return true;
}

void AlarmAgent::
setSequenceId(ST_AlarmEventMsg & _stAlarmEventMsg,
              AuditData        & _audit)
{
    _stAlarmEventMsg.sequence_id_ = _audit.GenerateSequenceId();
}

void AlarmAgent::
Final()
{
    // Empty
}

