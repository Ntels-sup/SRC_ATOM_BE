
#ifndef __APP_FOR_ALARM_HPP__
#define __APP_FOR_ALARM_HPP__

#include "CSocketServer.hpp"
#include "EventAPI.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"

class AppForAlarm
{
public:
    AppForAlarm();
    ~AppForAlarm();

    bool Init();
    bool SetIF(CModuleIPC * _ipc, CAddress * _addr);
    bool IsEmpty();
    const char * GetBody();

    void Proc(const char * _p);

private:
    void extract(char        * _out,
                 const char  * _tag,
                 const char  * _source);

    void connect(const char * _p);
    void ping(const char *    _p);
    void process(const char * _p);
    void alarm(const char *   _p);

private:

    int             node_id_;
    int             proc_id_;
    std::string     body_;

    EventAPI        api_;

    CSocketServer * server_;

    CModuleIPC *    ipc_;
    CAddress *      addr_;
};

#endif // __APP_FOR_ALARM_HPP__




#include "CFileLog.hpp"