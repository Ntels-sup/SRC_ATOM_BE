
#ifndef __EVENT_API_HPP__
#define __EVENT_API_HPP__

#include <string>
#include <vector>

#include "CommonCode.hpp"

#include "CProtocol.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

class EventAPI
{
public:
    EventAPI();
    ~EventAPI();

    bool    Init();

    void    SetNodeInfo(int          _no,
                        const char * _name);
    void    SetProcInfo(int          _no,
                        const char * _name);
    bool    SetIPCandAddr(CModuleIPC * _ipc, CAddress * _addr);

    bool    SendTrap(const char *  _code,
                     const char *  _target = NULL,
                     int           _value  = 0,
                     const char *  _complement = NULL,
                     const char *  _text   = NULL);

    bool    SendTrap(const char *  _code,
                     const char *  _target = NULL,
                     double        _value  = 0.0,
                     const char *  _complement = NULL,
                     const char *  _text   = NULL);

    bool    SendTrap(const char *   _code,
                     const char *   _target = NULL,
                     const char *   _value  = NULL,
                     const char *   _complement = NULL,
                     const char *   _text   = NULL);

    bool    Notify(int                  _command_id,
                   int                  _dest_node,
                   int                  _dest_proc,
                   std::vector<char> &  _vec);

    bool    Response(CProtocol         & _protocol,
                     std::vector<char> & _vec);

    bool    PingSuccess(int             _peer_node,
                        const char *    _my_ip,
                        const char *    _peer_ip);

    bool    PingFail(int             _peer_node,
                     const char *    _my_ip,
                     const char *    _peer_ip);

    bool    ProcessReport(const char *  _worst_status,
                          int           _proc_no,
                          const char *  _proc_name,
                          const char *  _status);

    bool    NodeReport(const char *  _node_status);

    bool    ConnectReport(const char * _my_ip,
                          const char * _peer_ip,
                          const char * _service_name);

    bool    DisconnectReport(const char * _my_ip,
                             const char * _peer_ip,
                             const char * _service_name);

    bool    Subscribe(const char * _code,
                      int          _severity_id,
                      char *       _pkg_name = NULL);

    bool    RegisterProvider(int   _command_id,
                             int   _dest_proc);

    bool    RegisterTps();

    bool    SendTps(const char * _tps_title,
                    const int    _value,
                    CProtocol  & _protocol);

    const char *    GetErrorMsg();

private:
    void    addPrcDateAndDstYn(std::vector<char> & _vec);

private:
    std::vector<char>       vec_;
    CProtocol               protocol_;

    CModuleIPC *            ipc_;
    CAddress   *            addr_;

    ST_AtomAddr             my_node_;
    ST_AtomAddr             my_proc_;
    ST_AtomAddr             ala_proc_;
    ST_AtomAddr             vna_proc_;

    ST_AtomAddr             alm_node_;
    ST_AtomAddr             alm_proc_;
};

#endif // __EVENT_API_HPP__




