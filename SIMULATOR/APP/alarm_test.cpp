
#include "AlarmTest.hpp"

extern CFileLog * g_pclsLog;
extern ATOM_API * g_pSenderAPI;

int init_sender_api(int _argc, char * _argv[])
{
    g_pSenderAPI = new ATOM_API();
    g_pSenderAPI->Init(_argc, _argv, LV_DEBUG, 0, DEF_CMD_TYPE_SEND);

    return 0;
}

void init_socket_server(CSocketServer * _server)
{
    if(_server != NULL)
        return ;

    _server = new (std::nothrow) CSocketServer(CSocket::IP_TCP);

    _server->SetReuseAddress();

    if(_server->Listen("127.0.0.1", 20000) != true)
    {
        g_pclsLog->WARNING("%-24s| Init - Listen fail",
            "init_sender_api");

        if(_server != NULL)
        {
            delete _server;
            _server = NULL;
        }
    }
}

void extract(char        * _out,
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


void alarm(const char * _p)
{
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

    g_pclsLog->DEBUG("CODE      - [%s]", code);
    g_pclsLog->DEBUG("TARGET    - [%s]", target);
    g_pclsLog->DEBUG("VALUE     - [%s]", value);
    g_pclsLog->DEBUG("COMPLEMENT- [%s]", complement);
    g_pclsLog->DEBUG("TEXT      - [%s]", text);

    if(g_pSenderAPI->SendTrap(code,
                              target,
                              value,
                              complement,
                              text) != true)
    {
        g_pclsLog->ERROR("%-24s| alarm - can't SendTrap [%s]",
            "alarm",
            _p);
        return ;
    }

    g_pclsLog->INFO("%-24s| Proc - SendTrap SUCCESS",
        "alarm");

}


void connect(const char * _p)
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

    g_pclsLog->DEBUG("MY_IP      - [%s]", my_ip);
    g_pclsLog->DEBUG("PEER_IP    - [%s]", peer_ip);
    g_pclsLog->DEBUG("PORT       - [%s]", port);
    g_pclsLog->DEBUG("STATUS     - [%s]", status);

    int nPort   = atoi(port);

    if(strstr(status, "DIS") == NULL)
    {
        if(g_pSenderAPI->ConnectReport(my_ip,
                              peer_ip,
                              nPort) != true)
        {
            g_pclsLog->ERROR("%-24s| connect - can't ConnectReport [%s]",
                "connect",
                _p);
            return ;
        }
    }
    else
    {
        if(g_pSenderAPI->DisconnectReport(my_ip,
                                 peer_ip,
                                 nPort) != true)
        {
            g_pclsLog->ERROR("%-24s| connect - can't DisconnectReport [%s]",
                "connect",
                _p);
            return ;
        }
    }

    g_pclsLog->INFO("%-24s| connect - send",
        "connect");
}



int proc_alarm_event(CSocketServer * _server)
{
    init_socket_server(_server);

    if(_server == NULL)
        return 0;

    // Recv
    CSocket * temp = NULL;
    if((temp = _server->Select(0, 0)) == NULL)
        return 0;

    // len ( 4 byte char ) + cmd data
    char    len[4+1];
    memset(len, 0, sizeof(len));

    std::vector<char>   vec(256, '\0');

    if(temp->Recv(len, 4) <= 0)
    {
        temp->Close();

        g_pclsLog->WARNING("%-24s| proc_event - alarm sim is closed",
            "proc_event");
        return 0;
    }

    int size = atoi(len);
    char & p = vec.at(0);

    if(temp->Recv(&p, size) <= 0)
    {
        g_pclsLog->WARNING("%-24s| proc_event - recv fail.. alarm sim is closed",
            "proc_event");

        temp->Close();
        return 0;
    }

    std::string body = std::string(vec.begin(), vec.end());
    g_pclsLog->INFO("%-24s| proc_event - RECV : [%s]",
        "proc_event",
        body.c_str());

    // Data Check
    if(body.find("TARGET") != std::string::npos)
        alarm(body.c_str());
    else if(body.find("PORT") != std::string::npos)
        connect(body.c_str());
    else
        g_pclsLog->WARNING("%-24s| proc_event - unknown cmd",
            "proc_event");

    return 1;
}
