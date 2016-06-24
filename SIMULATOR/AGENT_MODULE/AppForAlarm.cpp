
#include "AppForAlarm.hpp"
#include "CFileLog.hpp"

extern CFileLog  *   gLog;

AppForAlarm::
AppForAlarm()
{
    server_         = NULL;

    ipc_            = NULL;
    addr_           = NULL;
}

AppForAlarm::
~AppForAlarm()
{
    if(server_ != NULL)
    {
        delete server_;
        server_     = NULL;
    }
}

bool AppForAlarm::
Init()
{
    server_ = new (std::nothrow) CSocketServer(CSocket::IP_TCP);

    server_->SetReuseAddress();

    if(server_->Listen("127.0.0.1", 20000) != true)
    {
        gLog->WARNING("%-24s| Init - Listen fail",
            "AppForAlarm");
        return false;
    }

    return true;
}

bool AppForAlarm::
SetIF(CModuleIPC * _ipc, CAddress * _addr)
{
    ipc_    = _ipc;
    addr_   = _addr;

    if(addr_->LookupAtom("ATOM_NA_STA", node_id_, proc_id_) <= 0)
    {
        gLog->ERROR("%-24s| SetIF - LookupAtom fail",
            "AppForAlarm");
        return false;
    }

    gLog->INFO("%-24s| SetIF - my node[%d] proc[%d]",
        "AppForAlarm",
        node_id_,
        proc_id_);

    api_.SetNodeInfo(node_id_, "");
    api_.SetProcInfo(proc_id_, "");
    api_.SetIPCandAddr(_ipc,   _addr);

    if(api_.Init() == false)
    {
        gLog->ERROR("%-24s| EventAPI initialize fail" ,
            "AppForAlarm");

        return false;
    }

    return true;
}

bool AppForAlarm::
IsEmpty()
{
    CSocket * temp = NULL;
    if((temp = server_->Select(0, 5)) == NULL)
        return true;

    // 그냥 테스트를 위한 데이터 입니다.
    // length : 4 byte
    // Body

    char len[4+1];
    memset(len, 0, sizeof(len));

    std::vector<char>   vec(256, '\0');

    if(temp->Recv(len, 4) <= 0)
    {
        temp->Close();

        gLog->ERROR("%-24s| IsEmpty - Recv for len fail [%s]",
            "AppForAlarm",
            temp->m_strErrorMsg.c_str());
        return true;
    }

    gLog->DEBUG("RECV - [%s]", len);

    int size = atoi(len);
    char & p =  vec.at(0);

    if(temp->Recv(&p, size) <= 0)
    {
        gLog->ERROR("%-24s| IsEmpty - Recv for vec fail [%s]",
            "AppForAlarm",
            temp->m_strErrorMsg.c_str());
        return true;
    }

    body_ = std::string(vec.begin(), vec.end());

    gLog->DEBUG("RECV - [%s]", body_.c_str());

    return false;
}

const char * AppForAlarm::
GetBody()
{
    return body_.c_str();
}

void AppForAlarm::
Proc(const char * _p)
{
    if(strstr(_p, "TARGET") != NULL)
        alarm(_p);
    else if(strstr(_p, "WORST_STATUS") != NULL)
        process(_p);
    else if(strstr(_p, "PEER_NODE") != NULL)
        ping(_p);
    else if(strstr(_p, "PORT") != NULL)
        connect(_p);
    else
        gLog->WARNING("Unknown Type Message [%s]", _p);
}

void AppForAlarm::
connect(const char * _p)
{
    char my_ip[64];
    char peer_ip[64];
    char port[64];
    char status[64];

    memset(my_ip,       0, sizeof(my_ip));
    memset(peer_ip,     0, sizeof(peer_ip));
    memset(port,        0, sizeof(port));
    memset(status,      0, sizeof(status));

    extract(my_ip,      "MY_IP",      _p);
    extract(peer_ip,    "PEER_IP",    _p);
    extract(port,       "PORT",       _p);
    extract(status,     "STATUS",     _p);

    gLog->DEBUG("MY_IP      - [%s]", my_ip);
    gLog->DEBUG("PEER_IP    - [%s]", peer_ip);
    gLog->DEBUG("PORT       - [%s]", port);
    gLog->DEBUG("STATUS     - [%s]", status);

    if(strstr(status, "DIS") == NULL)
    {
        if(api_.ConnectReport(my_ip,
                              peer_ip,
                              port) != true)
        {
            gLog->ERROR("%-24s| connect - can't ConnectReport [%s] [%s]",
                "AppForAlarm",
                _p,
                api_.GetErrorMsg());
            return ;
        }
    }
    else
    {
        if(api_.DisconnectReport(my_ip,
                                 peer_ip,
                                 port) != true)
        {
            gLog->ERROR("%-24s| connect - can't DisconnectReport [%s] [%s]",
                "AppForAlarm",
                _p,
                api_.GetErrorMsg());
            return ;
        }
    }

    gLog->INFO("%-24s| connect - [%s]SUCCESS",
        "AppForAlarm",
        status);
}

void AppForAlarm::
ping(const char * _p)
{
    char peer_node[64];
    char my_ip[64];
    char peer_ip[64];
    char status[64];

    memset(peer_node,   0, sizeof(peer_node));
    memset(my_ip,       0, sizeof(my_ip));
    memset(peer_ip,     0, sizeof(peer_ip));
    memset(status,      0, sizeof(status));

    extract(peer_node,  "PEER_NODE",  _p);
    extract(my_ip,      "MY_IP",      _p);
    extract(peer_ip,    "PEER_IP",    _p);
    extract(status,     "STATUS",     _p);

    gLog->DEBUG("PEER_NODE  - [%s]", peer_node);
    gLog->DEBUG("MY_IP      - [%s]", my_ip);
    gLog->DEBUG("PEER_IP    - [%s]", peer_ip);
    gLog->DEBUG("STATUS     - [%s]", status);

    int nPeerNode  = atoi(peer_node);

    if(strstr(status, "SUCCESS") != NULL)
    {
        if(api_.PingSuccess(nPeerNode,
                            my_ip,
                            peer_ip) != true)
        {
            gLog->ERROR("%-24s| ping - can't PingSuccess [%s] [%s]",
                "AppForAlarm",
                _p,
                api_.GetErrorMsg());
            return ;
        }
    }
    else
    {
        if(api_.PingFail(nPeerNode,
                         my_ip,
                         peer_ip) != true)
        {
            gLog->ERROR("%-24s| ping - can't PingFail [%s] [%s]",
                "AppForAlarm",
                _p,
                api_.GetErrorMsg());
            return ;
        }
    }

    gLog->INFO("%-24s| ping - SUCCESS",
        "AppForAlarm");
}

void AppForAlarm::
process(const char * _p)
{
    char    worst_status[64];
    char    proc_no[16];
    char    proc_name[64];
    char    proc_status[64];

    memset(worst_status, 0,  sizeof(worst_status));
    memset(proc_no, 0,  sizeof(proc_no));
    memset(proc_name,0, sizeof(proc_name));
    memset(proc_status, 0, sizeof(proc_status));

    extract(worst_status, "WORST_STATUS",  _p);
    extract(proc_no,    "PROC_NO",  _p);
    extract(proc_name,  "PROC_NAME",_p);
    extract(proc_status,"PROC_STATUS", _p);

    gLog->DEBUG("WORST_STATUS   - [%s]", worst_status);
    gLog->DEBUG("PROC_NO        - [%s]", proc_no);
    gLog->DEBUG("PROC_NAME      - [%s]", proc_name);
    gLog->DEBUG("PROC_STATUS    - [%s]", proc_status);

    if(api_.ProcessReport(worst_status,
                          atoi(proc_no),
                          proc_name,
                          proc_status) != true)
    {
        gLog->ERROR("%-24s| process - can't ProcessReport [%s] [%s]",
            "AppForAlarm",
            _p,
            api_.GetErrorMsg());
        return ;
    }

    gLog->INFO("%-24s| process - ProcessReport SUCCESS",
        "AppForAlarm");
}

void AppForAlarm::
alarm(const char * _p)
{
    // _p 는 데이터 입니다.
    // 데이터 포멧
    // code:XXXX, target:XXXX...

    char code[64];
    char target[64];
    char value[64];
    char complement[64];
    char text[64];

    memset(code,   0, sizeof(code));
    memset(target, 0, sizeof(target));
    memset(value,  0, sizeof(value));
    memset(complement, 0, sizeof(complement));
    memset(text,   0, sizeof(text));

    extract(code,        "CODE",        _p);
    extract(target,      "TARGET",      _p);
    extract(value,       "VALUE",       _p);
    extract(complement,  "COMPLEMENT",  _p);
    extract(text,        "TEXT",        _p);

    gLog->DEBUG("CODE      - [%s]", code);
    gLog->DEBUG("TARGET    - [%s]", target);
    gLog->DEBUG("VALUE     - [%s]", value);
    gLog->DEBUG("COMPLEMENT- [%s]", complement);
    gLog->DEBUG("TEXT      - [%s]", text);

    if(api_.SendTrap(code,
                     target,
                     value,
                     complement,
                     text) != true)
    {
        gLog->ERROR("%-24s| alarm - can't SendTrap [%s] [%s]",
            "AppForAlarm",
            _p,
            api_.GetErrorMsg());
        return ;
    }

    gLog->INFO("%-24s| Proc - SendTrap SUCCESS",
        "AppForAlarm");

}

void AppForAlarm::
extract(char        * _out,
        const char  * _tag,
        const char  * _source)
{
    char * p = const_cast<char *>(_source);

    if((p = strstr(p, _tag)) == NULL)
        return;

    p += strlen(_tag);

    p++;    // :

    char * q = NULL;

    if((q = strstr(p, ",")) == NULL)
        strcpy(_out, p);
    else
        strncpy(_out, p, q - p);

}
