
#include "CommandFormat.hpp"

#include "NodeManagerIF.hpp"

#include "AlarmCFG.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

NodeManagerIF::
NodeManagerIF()
{
    // Empty
}

NodeManagerIF::
~NodeManagerIF()
{
    // Empty
}

bool NodeManagerIF::
Init()
{
    bConnected_ = false;

    for(auto nLoop=0; nLoop < gCFG.NM.retry_cnt_; ++nLoop)
    {
        if(Connect(gCFG.NM.ip_.c_str(), gCFG.NM.port_) == true)
        {
            gAlmLog->INFO("%-24s| Init - CONNECT TO NODE MANAGER SUCCESS",
                "NodeManagerIF");
            break;
        }

        gAlmLog->WARNING("%-24s| Init - connect to Node Manager fail",
            "NodeManagerIF");
        poll(NULL, 0, 1 * 1000);
    }

    if(bConnected_ != true)
        return false;

    return Register();

}

bool NodeManagerIF::
Connect(const char * _ip, int _port)
{
    bConnected_ = sock_.Connect(_ip, _port);
    return bConnected_;
}

void NodeManagerIF::
Close()
{
    sock_.Close();
    bConnected_ = false;
}

bool NodeManagerIF::
Register()
{
    return (sendRegister() == true && recvRegister() == true)?true:false;
}

bool NodeManagerIF::
sendRegister()
{
    sock_.SetFlagRequest();
    sock_.SetSequence(0);

    char    regi_command_id[16];
    sprintf(regi_command_id, "%010d", CMD_REGIST);

    sock_.SetCommand(regi_command_id);
    sock_.SetSource(gCFG.profile_.m_nNodeNo,  gCFG.profile_.m_nProcNo);
    sock_.SetDestination(gCFG.NM.nm_node_no_, gCFG.NM.nm_proc_no_);

    char    send_body[256];
    sprintf(send_body,
        "{ \"BODY\" : {"
        " \"pkgname\" :\"ATOM\", "
        " \"nodetype\":\"EMS\", "
        " \"procname\":\"%s\", "
        " \"procno\": %d "
        " } }",
        gCFG.profile_.m_strProcName.c_str(),
        gCFG.profile_.m_nProcNo);

    vec_body_.clear();
    std::copy(send_body,
              send_body + strlen(send_body),
              std::back_inserter(vec_body_));
    vec_body_.push_back('\0');
    sock_.SetPayload(vec_body_);

    if(sock_.SendMesg() == false)
    {
        // TO DO - ErrorMsg 표시 필요.
        gAlmLog->ERROR("%-24s| sendRegister - SendMesg fail",
            "NodeManagerIF");

        return false;
    }

    return true;
}

bool NodeManagerIF::
recvRegister()
{
    int nLoop = 5;
    while(nLoop-- > 0)
    {
        if(GetData() == false)
        {
            poll(NULL, 0, 3 * 1000);
            gAlmLog->WARNING("%-24s| recvRegister - wait register response",
                "NodeManagerIF");
            continue;
        }

        if(GetCommand() != CMD_REGIST)
        {
            gAlmLog->WARNING("%-24s| recvRegister - unexpected command [%d]",
                "NodeManagerIF",
                GetCommand());
            continue;
        }

        if(sock_.IsFlagError() == false)
        {
            gAlmLog->INFO("%-24s| recvRegister - SUCCESS",
                "NodeManagerIF");

            return true;
        }

        std::string     recv_body(128, '\0');

        TakeOutBody(recv_body);

        gAlmLog->WARNING("%-24s| recvRegister - recv [%s]",
            "NodeManagerIF",
            recv_body.c_str());

        break;
    }

    gAlmLog->ERROR("%-24s| Register - fail",
        "NodeManagerIF");

    return false;
}


bool NodeManagerIF::
GetData()
{
    if(bConnected_ != true)
    {
        Init();
        return false;
    }

    int ret = sock_.RecvMesg(NULL, 1);

    if(ret < 0)
    {
        sock_.Close();
        Init();

        return false;
    }

    if(ret == 0)
        return false;

    gAlmLog->DEBUG("RECEIVED MSG");

    // request 수신 -> 작업 -> response 송신 일때,
    // response 메시지 만들라고요..
    saveHeaderForResponse();

    return true;
}

void NodeManagerIF::
saveHeaderForResponse()
{
    header_.Clear();

    header_.version_    =   sock_.GetVersion();
    header_.flag_       =   sock_.GetFlag();
    header_.seq_        =   sock_.GetSequence();

    sock_.GetCommand(header_.command_);
    sock_.GetSource(header_.src_node_, header_.src_proc_);
    sock_.GetDestination(header_.dest_node_, header_.dest_node_);
}

int NodeManagerIF::
GetCommand()
{
    std::string     cmd;
    sock_.GetCommand(cmd);

    gAlmLog->DEBUG("CMD [%s]", cmd.c_str());

    return atoi(cmd.c_str());
}

void NodeManagerIF::
TakeOutBody(std::string & _body)
{
    vec_body_.clear();

    sock_.GetPayload(vec_body_);

    _body = std::string(vec_body_.begin(), vec_body_.end());
}

int NodeManagerIF::
GetRequestNodeNo()
{
    return header_.src_node_;
}

bool NodeManagerIF::
Response(std::string & _body)
{
    sock_.SetFlagResponse();
    sock_.SetSequence(header_.seq_);

    sock_.SetCommand(header_.command_.c_str());
    sock_.SetSource(header_.dest_node_, header_.dest_proc_);
    sock_.SetDestination(header_.src_node_, header_.src_proc_);

    vec_body_.clear();
    std::copy(_body.begin(), _body.end(), std::back_inserter(vec_body_));

    vec_body_.push_back('\0');
    sock_.SetPayload(vec_body_);
    return sock_.SendMesg();
}

bool NodeManagerIF::
Notify(int          _command_id,
       int          _dest_node,
       int          _dest_proc,
       std::string & _body)
{
    gAlmLog->DEBUG("-- dest no [%d] dest proc [%u] [%s]", 
			_dest_node,
			_dest_proc,
			_body.c_str());

    char command_id[16];
    sprintf(command_id, "%010d", _command_id);

    sock_.SetCommand(command_id);
    sock_.SetFlagNotify();
    sock_.SetSource(gCFG.profile_.m_nNodeNo, gCFG.profile_.m_nProcNo);
    sock_.SetDestination(_dest_node, _dest_proc);
    sock_.SetSequence(0);

    if(_body.size() > 0)
    {
        vec_body_.clear();
        std::copy(_body.begin(), _body.end(), std::back_inserter(vec_body_));
        vec_body_.push_back('\0');
        sock_.SetPayload(vec_body_);
    }

    return sock_.SendMesg();
}
