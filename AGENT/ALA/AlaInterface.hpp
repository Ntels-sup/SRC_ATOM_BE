
#ifndef __ALA_INTERFACE_HPP__
#define __ALA_INTERFACE_HPP__

#include "CProtocol.hpp"
#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

#include "EventAPI.hpp"

class AlaInterface
{
public:
    explicit AlaInterface();
    ~AlaInterface();

    bool    Init(ST_Profile & _profile,
                 CModuleIPC * _ipc,
                 CAddress   * _addr);

    bool    GetData();

    int     GetCommand();
    void    GetSource(int & _node_no,
                      int & _proc_no) { protocol_.GetSource(_node_no, _proc_no); }
    const char * GetErrorMsg() { return ipc_->m_strErrorMsg.c_str(); }
    void    TakeOutBody(std::string & _body);
    bool    SendSync();
    bool    SendAlarm(std::string & _body);
    bool    SendRetryAlarm(std::string & _body);
    bool    SendToEXA(std::string & _body);

public:
    EventAPI            event_api_;

private:
    CProtocol           protocol_;
    CModuleIPC *        ipc_;
    CAddress   *        addr_;

    std::vector<char>   vec_;

    ST_AtomAddr         my_node_;
    ST_AtomAddr         ala_proc_;
    ST_AtomAddr         exa_proc_;

    ST_AtomAddr         alm_node_;
    ST_AtomAddr         alm_proc_;

};


#endif // __ALA_INTERFACE_HPP__
