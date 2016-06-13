#ifndef __HTTP_CLIENT_HPP__
#define __HTTP_CLIENT_HPP__

#include "CSocket.hpp"
#include "HttpData.hpp"

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    bool        Init();
    void        Close();
    bool        Send(HttpData & a_rHttpData);
    HttpData &  GetData();


private:
    bool        extractHostAndPortFromUri(char * a_pHost,
                                          char * a_pPort,
                                          const char * a_pUri);

private:
    CSocket *   m_pSession;

    HttpData    m_rHttpData;

};

#endif // __HTTP_CLIENT_HPP__