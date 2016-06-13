
#ifndef __CONTENTS_PROVIDER_HPP__
#define __CONTENTS_PROVIDER_HPP__

#include "CProtocol.hpp"
#include "CModuleIPC.hpp"
#include "TimeOutCheck.hpp"

#include <unordered_map>
#include <list>
#include <string>

/*
    ATOM 은, Request - Response 구조로 되어 있어서,
    예를들어 RSA 로부터 자원 정보를 얻으려면 Request 를 하고 Response 로 정보를 받아야 합니다.

    따라서, RSA 가 등록을 요청하면 ( EventAPI.RegisterProvider() )

    여기서 받아서 시간마다 Request 를 날려줍니다.

*/

class ContentsProvider
{
public:
    explicit ContentsProvider();
    ~ContentsProvider();

    bool Init(int            _my_node_no,
              int            _my_proc_no,
              CModuleIPC *   _ipc,
              int            _period = 60);
    bool Register(std::string & _apiMsg);
    void Register(int           _command_code,
                  int           _proc_no,
                  std::string & _body);
    bool RequestAll();

private:
    void makeRequest(CProtocol & _protocol,
                     int         _proc_no,
                     int         _command_code);
    void makeBody(CProtocol   &  _protocol,
                  std::string &  _body);
    int  genSequence();

private:
    TimeOutCheck      timer_;
    unsigned short    seq_;

    // KEY : CommandCode
    // pair-first  : proc no
    // pair-second : body
    std::unordered_map<int,
                       std::list<std::pair<int, std::string> > > map_;

    int               my_node_no_;
    int               my_proc_no_;
    std::vector<char> vec_;

    CProtocol         protocol_;
    int               period_;
    CModuleIPC      * ipc_;

};

#endif // __CONTENTS_PROVIDER_HPP__
