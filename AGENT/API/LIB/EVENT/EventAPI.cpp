
#include "EventAPI.hpp"

#include "CommonCode.hpp"
#include "CommandFormat.hpp"

#include "CTimeUtil.hpp"

EventAPI::
EventAPI()
{
    // 512 는 특별히 의미가 있는 것은 아니기 때문에 별도로 정의하지 않았습니다.
    vec_.resize(512);
    vec_.assign('\0', vec_.size());

    my_node_.m_nNo = -1;
    my_node_.m_strName.clear();

    my_proc_.m_nNo = -1;
    my_proc_.m_strName.clear();

    ipc_    = NULL;
    addr_   = NULL;
}

EventAPI::
~EventAPI()
{
    my_node_.m_nNo = -1;
    my_node_.m_strName.clear();

    my_proc_.m_nNo = -1;
    my_proc_.m_strName.clear();

    ipc_    = NULL;
    addr_   = NULL;
}

bool EventAPI::
Init()
{
    if(my_node_.m_nNo   < 0 || my_proc_.m_nNo < 0)
        return false;

    if(ipc_ == NULL || addr_ == NULL)
        return false;

    return true;
}

void EventAPI::
SetNodeInfo(int          _no,
            const char * _name)
{
    my_node_.m_nNo      = _no;
    my_node_.m_strName  = _name;
}

void EventAPI::
SetProcInfo(int          _no,
            const char * _name)
{
    my_proc_.m_nNo      = _no;
    my_proc_.m_strName  = _name;
}

bool EventAPI::
SetIPCandAddr(CModuleIPC * _ipc, CAddress * _addr)
{
    ipc_    = _ipc;
    addr_   = _addr;

    if(addr_->LookupAtom("ATOM_NA_ALA",  my_node_.m_nNo, ala_proc_.m_nNo) <= 0)
        return false;

    if(addr_->LookupAtom("ATOM_NA_EXA",  my_node_.m_nNo, vna_proc_.m_nNo) <= 0)
        return false;

    if(addr_->LookupAtom("ATOM_ALM", alm_node_.m_nNo, alm_proc_.m_nNo) <= 0)
        return false;

    return true;
}

bool EventAPI::
SendTrap(const char *  _code,
         const char *  _target,
         int           _value,
         const char *  _complement,
         const char *  _text)
{
    char    buf[DB_ALM_VALUE_SIZE+1];
    sprintf(buf, "%d", _value);

    return SendTrap(_code, _target, buf, _complement, _text);
}

bool EventAPI::
SendTrap(const char *  _code,
         const char *  _target,
         double        _value,
         const char *  _complement,
         const char *  _text)
{
    char    buf[DB_ALM_VALUE_SIZE+1];
    sprintf(buf, "%.2f", _value);

    return SendTrap(_code, _target, buf, _complement, _text);
}

bool EventAPI::
SendTrap(const char *   _code,
         const char *   _target,
         const char *   _value,
         const char *   _complement,
         const char *   _text)
{
    vec_.assign(my_proc_.m_strName.begin(), my_proc_.m_strName.end());
    vec_.push_back('\0');

    if(_code != NULL)
        vec_.insert(vec_.end(), _code,       _code + strlen(_code));
    vec_.push_back('\0');

    if(_target != NULL)
        vec_.insert(vec_.end(), _target,     _target + strlen(_target));
    vec_.push_back('\0');

    if(_value != NULL)
        vec_.insert(vec_.end(), _value,      _value+ strlen(_value));
    vec_.push_back('\0');

    if(_complement != NULL)
        vec_.insert(vec_.end(), _complement, _complement + strlen(_complement));
    vec_.push_back('\0');

    if(_text != NULL)
        vec_.insert(vec_.end(), _text,       _text + strlen(_text));
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_ALM_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);
}


void EventAPI::
addPrcDateAndDstYn(std::vector<char> & _vec)
{

  std::string   timestamp;
  char          dst;

  CTimeUtil::SetTimestampAndDstYn(timestamp, dst, 2);

  //printf("---------- jhchoi [%s] [%c]\n",
  //  timestamp.c_str(),
  //  dst);

  _vec.insert(_vec.end(), timestamp.begin(), timestamp.end());
  _vec.push_back('\0');

  _vec.push_back(dst);
  _vec.push_back('\0');
}


bool EventAPI::
Notify(int  _command_id,
       int  _dest_node,
       int  _dest_proc,
       std::vector<char> & _vec)
{
    char    command_id[16];
    sprintf(command_id, "%010d", _command_id);

    protocol_.Clear();
    protocol_.SetCommand(command_id);
    protocol_.SetFlagNotify();
    protocol_.SetSource(my_node_.m_nNo, my_proc_.m_nNo);
    protocol_.SetDestination(_dest_node, _dest_proc);
    protocol_.SetSequence(0);
    protocol_.SetPayload(_vec);

    return ipc_->SendMesg(protocol_);
}

bool EventAPI::
Response(CProtocol         & _protocol,
         std::vector<char> & _vec)
{
    protocol_.Clear();
    protocol_.SetCommand(_protocol.GetCommand().c_str());
    protocol_.SetFlagResponse();
    protocol_.SetSource(_protocol.GetDestination());
    protocol_.SetDestination(_protocol.GetSource());
    protocol_.SetSequence(_protocol.GetSequence());
    protocol_.SetPayload(_vec);

    return ipc_->SendMesg(protocol_);
}

const char * EventAPI::
GetErrorMsg()
{
    return ipc_->m_strErrorMsg.c_str();
}

bool EventAPI::
PingSuccess(int             _peer_node,
            const char *    _my_ip,
            const char *    _peer_ip)
{
    vec_.assign(my_proc_.m_strName.begin(), my_proc_.m_strName.end());
    vec_.push_back('\0');

    std::string     buffer(std::to_string((long long)_peer_node));
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _my_ip,     _my_ip + strlen(_my_ip));
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _peer_ip,   _peer_ip + strlen(_peer_ip));
    vec_.push_back('\0');

    buffer.assign("SUCCESS");
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_PING_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);
}

bool EventAPI::
PingFail(int             _peer_node,
         const char *    _my_ip,
         const char *    _peer_ip)
{
    vec_.assign(my_proc_.m_strName.begin(), my_proc_.m_strName.end());
    vec_.push_back('\0');

    std::string     buffer(std::to_string((long long)_peer_node));
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _my_ip,     _my_ip + strlen(_my_ip));
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _peer_ip,   _peer_ip + strlen(_peer_ip));
    vec_.push_back('\0');

    buffer.assign("FAIL");
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_PING_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);
}

bool EventAPI::
ProcessReport(const char *  _worst_status,
              int           _proc_no,
              const char *  _proc_name,
              const char *  _proc_status)
{
    vec_.assign(_worst_status, _worst_status + strlen(_worst_status));
    vec_.push_back('\0');

    std::string     buffer(std::to_string((long long)_proc_no));
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _proc_name,   _proc_name + strlen(_proc_name));
    vec_.push_back('\0');
    vec_.insert(vec_.end(), _proc_status,  _proc_status + strlen(_proc_status));
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_PROC_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);
}


bool EventAPI::
NodeReport(const char *  _node_status)
{
    vec_.assign(_node_status, _node_status + strlen(_node_status));
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_NODE_EVENT,
                  alm_node_.m_nNo,
                  alm_proc_.m_nNo,
                  vec_);
}

bool EventAPI::
ConnectReport(const char * _my_ip,
              const char * _peer_ip,
              const char * _service_name)
{
    vec_.assign(_my_ip, _my_ip + strlen(_my_ip));
    vec_.push_back('\0');

    vec_.insert(vec_.end(), _peer_ip,   _peer_ip + strlen(_peer_ip));
    vec_.push_back('\0');

    vec_.insert(vec_.end(), _service_name, _service_name + strlen(_service_name));
    vec_.push_back('\0');

    std::string buffer = "CONNECT";
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_CONNECT_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);

}

bool EventAPI::
DisconnectReport(const char * _my_ip,
                 const char * _peer_ip,
                 const char * _service_name)
{
    vec_.assign(_my_ip, _my_ip + strlen(_my_ip));
    vec_.push_back('\0');

    vec_.insert(vec_.end(), _peer_ip,   _peer_ip + strlen(_peer_ip));
    vec_.push_back('\0');

    vec_.insert(vec_.end(), _service_name, _service_name + strlen(_service_name));
    vec_.push_back('\0');

    std::string buffer = "DISCONNECT";
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_CONNECT_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);

}

bool EventAPI::
Subscribe(const char *  _code,
          int           _severity_id,
          char *        _pkg_name)
{
    vec_.assign(_code, _code + strlen(_code));
    vec_.push_back('\0');

    std::string     buffer(std::to_string((long long)_severity_id));
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    vec_.insert(vec_.end(), _pkg_name, _pkg_name + strlen(_pkg_name));
    vec_.push_back('\0');

    addPrcDateAndDstYn(vec_);

    return Notify(CMD_STATUS_CONNECT_EVENT,
                  my_node_.m_nNo,
                  ala_proc_.m_nNo,
                  vec_);

}

// 자신이 Request 받고자 하는 CMD 를 등록합니다.
bool EventAPI::
RegisterProvider(int    _command_id,
                 int    _dest_proc)
{
    std::string     buffer(std::to_string((long long)my_node_.m_nNo));

    vec_.assign(buffer.begin(), buffer.end());
    vec_.push_back('\0');

    buffer = std::to_string((long long)my_proc_.m_nNo);

    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    buffer = std::to_string((long long)_command_id);

    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    return Notify(CMD_REGISTER_PROVIDER,
                  my_node_.m_nNo,
                  _dest_proc,
                  vec_);
}

bool EventAPI::
RegisterTps()
{
  return RegisterProvider(CMD_VNF_PERF_TPS,
                          vna_proc_.m_nNo);
}

bool EventAPI::
SendTps(const char * _tps_title,
        const int    _value,
        CProtocol  & _protocol)
{
    vec_.clear();

    // PRC_DATE
    _protocol.GetPayload(vec_);

    // TPS TITLE
    vec_.insert(vec_.end(), _tps_title, _tps_title+strlen(_tps_title));
    vec_.push_back('\0');

    // VALUE
    std::string     buffer(std::to_string((long long)_value));
    vec_.insert(vec_.end(), buffer.begin(), buffer.end());
    vec_.push_back('\0');

    return Notify(CMD_VNF_PERF_TPS,
                  alm_node_.m_nNo,
                  alm_proc_.m_nNo,
                  vec_);
}
