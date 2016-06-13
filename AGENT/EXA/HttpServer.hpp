
#ifndef __HTTP_SERVER_HPP__
#define __HTTP_SERVER_HPP__

#include "CSocketServer.hpp"
#include "CSocket.hpp"
#include "HttpData.hpp"

class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

    bool        Init(std::string a_sIP, int a_nPort);
    HttpData &  GetData();
    bool        Send(HttpData & a_hData);
    bool        Send(const vnfm::eStatusCode a_eCode,
                     const std::string & a_strMsg);

private:
    CSocketServer   *   m_pServer;
    CSocket         *   m_pSession;

    HttpData    m_hData;

};

#endif // __HTTP_SERVER_HPP__

