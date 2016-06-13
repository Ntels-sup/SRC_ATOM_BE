
#include <string>
#include <vector>

#include "CSocketServer.hpp"
#include "CFileLog.hpp"
#include "CConfig.hpp"
#include "CAtomAPI.hpp"

int init_sender_api(int _argc, char * _argv[]);
void init_socket_server(CSocketServer * _server);
void extract(char        * _out,
             const char  * _tag,
             const char  * _source);
void alarm(const char * _p);
void connect(const char * _p);
int proc_alarm_event(CSocketServer * _server);
