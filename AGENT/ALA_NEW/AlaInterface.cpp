
#include "AlaInterface.hpp"
#include "CFileLog.hpp"

#include "CommonCode.hpp"
#include "CommandFormat.hpp"

extern CFileLog * gAlmLog;

AlaInterface::
AlaInterface()
{
    ipc_    = NULL;
    addr_   = NULL;

    vec_.resize(512);
}

AlaInterface::
~AlaInterface()
{
    // Empty
}

bool AlaInterface::
Init(ST_Profile & _profile,
     CModuleIPC * _ipc,
     CAddress   * _addr)
{

    ipc_            = _ipc;
    addr_           = _addr;

    my_node_.m_nNo      = _profile.m_nNodeNo;
    // my_node_.m_strName  = _profile.m_strNodeName;

    ala_proc_.m_nNo     = _profile.m_nProcNo;
    ala_proc_.m_strName = _profile.m_strProcName;

    if(addr_->LookupAtom("ATOM_NA_EXA", my_node_.m_nNo, exa_proc_.m_nNo) <= 0)
    {
        gAlmLog->ERROR("%-24s| Init - LookupAtom fail [ATOM_NA_EXA]",
            "AlaInterface");
        return false;
    } else {
        exa_proc_.m_strName    = "EXA";
    }

    if(addr_->LookupAtom("ATOM_ALM", alm_node_.m_nNo, alm_proc_.m_nNo) <= 0)
    {
        gAlmLog->ERROR("%-24s| Init - LookupAtom fail [ATOM_NM_ALM]",
            "AlaInterface");
        return false;
    } else {
        alm_node_.m_strName    = "NM";
        alm_proc_.m_strName    = "ALM";
    }

    event_api_.SetNodeInfo(_profile.m_nNodeNo,  _profile.m_strNodeName.c_str());
    event_api_.SetProcInfo(_profile.m_nProcNo,  _profile.m_strProcName.c_str());
    if(event_api_.SetIPCandAddr(_ipc, _addr) == false)
    {
        gAlmLog->ERROR("%-24s| Init - event api set ipc and addr fail",
            "AlaInterface");
        return false;
    }

    if(event_api_.Init() == false)
    {
        gAlmLog->ERROR("%-24s| Init - event api init fail",
            "AlaInterface");
        return false;
    }

    gAlmLog->INFO("%-24s| Init - my node[%d] ala proc[%d] exa proc[%d] alm node[%d] alm proc[%d]",
        "AlaInterface",
        my_node_.m_nNo,
        ala_proc_.m_nNo,
        exa_proc_.m_nNo,
        alm_node_.m_nNo,
        alm_proc_.m_nNo);

    return true;
}


bool AlaInterface::
GetData()
{
    int ret = 0;
    if((ret = ipc_->RecvMesg(ala_proc_.m_nNo, protocol_, -1)) < 0)
    {
        gAlmLog->ERROR("%-24s| Do - RecvMesg Error [%s]",
            "AlaInterface",
            ipc_->m_strErrorMsg.c_str());
        return false;
    }

    if(ret == 0)
        return false;

    return true;
}

int AlaInterface::
GetCommand()
{
    return atoi(protocol_.GetCommand().c_str());
}

void AlaInterface::
TakeOutBody(std::string & _body)
{
    vec_.clear();

    protocol_.GetPayload(vec_);

    _body = std::string(vec_.begin(), vec_.end());
}

bool AlaInterface::
SendSync()
{
    gAlmLog->DEBUG("%-24s| SendSync", "AlaInterface");

    char    command_id[16];
    sprintf(command_id, "%010d", CMD_ALM_SYNC_REQ);

    protocol_.Clear();
    protocol_.SetCommand(command_id);
    protocol_.SetFlagNotify();
    protocol_.SetSource(my_node_.m_nNo, ala_proc_.m_nNo);
    protocol_.SetDestination(alm_node_.m_nNo, alm_proc_.m_nNo);
    protocol_.SetSequence(0);

    return ipc_->SendMesg(protocol_);
}

bool AlaInterface::
SendAlarm(std::string & _body)
{
    gAlmLog->DEBUG("%-24s| SendAlarm", "AlaInterface");

    vec_.clear();
    vec_.assign(_body.begin(), _body.end());

    char    command_id[16];
    sprintf(command_id, "%010d", CMD_ALM_EVENT);

    protocol_.Clear();
    protocol_.SetCommand(command_id);
    protocol_.SetFlagNotify();
    protocol_.SetSource(my_node_.m_nNo, ala_proc_.m_nNo);
    protocol_.SetDestination(alm_node_.m_nNo, alm_proc_.m_nNo);
    protocol_.SetSequence(0);
    protocol_.SetPayload(vec_);

    return ipc_->SendMesg(protocol_);
}

bool AlaInterface::
SendRetryAlarm(std::string & _body)
{
    gAlmLog->DEBUG("%-24s| SendRetryAlarm", "AlaInterface");

    vec_.clear();
    vec_.assign(_body.begin(), _body.end());

    char    command_id[16];
    sprintf(command_id, "%010d", CMD_ALM_RETRY_RESP);

    protocol_.Clear();
    protocol_.SetCommand(command_id);
    protocol_.SetFlagNotify();
    protocol_.SetSource(my_node_.m_nNo, ala_proc_.m_nNo);
    protocol_.SetDestination(alm_node_.m_nNo, alm_proc_.m_nNo);
    protocol_.SetSequence(0);
    protocol_.SetPayload(vec_);

    return ipc_->SendMesg(protocol_);
}


bool AlaInterface::
SendToEXA(std::string & _body)
{
    gAlmLog->DEBUG("%-24s| SendToEXA", "AlaInterface");

    vec_.clear();
    vec_.assign(_body.begin(), _body.end());

    char    command_id[16];
    sprintf(command_id, "%010d", CMD_ALM_EVENT);

    protocol_.Clear();
    protocol_.SetCommand(command_id);
    protocol_.SetFlagNotify();
    protocol_.SetSource(my_node_.m_nNo, ala_proc_.m_nNo);
    protocol_.SetDestination(my_node_.m_nNo, exa_proc_.m_nNo);
    protocol_.SetSequence(0);
    protocol_.SetPayload(vec_);

    return ipc_->SendMesg(protocol_);
}


