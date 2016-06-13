#ifndef __NODE_MANAGER_IF_HPP__
#define __NODE_MANAGER_IF_HPP__

#include <cstring>
#include <string>
#include <vector>

#include "CMesgExchSocket.hpp"
#include "AlarmDataDef.hpp"

class NodeManagerIF
{
public:
    NodeManagerIF();
    ~NodeManagerIF();

    bool    Init();
    bool    Connect(const char * _ip, int _port);
    void    Close();
    bool    Register();

    bool    IsEmpty();
    int     GetCommand();
    void    TakeOutBody(std::string & _body);

    bool    Response(std::string & _body);
    bool    Notify(int          _command_id,
                   int          _dest_node,
                   int          _dest_proc,
                   std::string & _body);

    bool    NotifyForConnect();

private:
    void    saveHeaderForResponse();
    bool    sendRegister();
    bool    recvRegister();

private:

    bool                bConnected_;

    CMesgExchSocket     sock_;
    std::vector<char>   vec_body_;

    ST_ProtocolHeader   header_;
};

#endif // __NODE_MANAGER_IF_HPP__
