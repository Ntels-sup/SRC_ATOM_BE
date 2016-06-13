
#include "HttpClient.hpp"
#include "CFileLog.hpp"

extern CFileLog * gLog;

HttpClient::
HttpClient()
{
    m_pSession = NULL;
}

HttpClient::
~HttpClient()
{
    if(m_pSession != NULL)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

bool HttpClient::
Init()
{
    m_pSession = new (std::nothrow) CSocket();

    if(m_pSession == NULL)
    {
        gLog->WARNING("Init fail");
        return false;
    }

    return true;
}

void HttpClient::
Close()
{
    if(m_pSession != NULL)
    {
        delete m_pSession;
        m_pSession = NULL;
    }
}

bool HttpClient::
Send(HttpData & a_sHttpData)
{
    if(a_sHttpData.IsFill() != true)
        return true;

    char    ip[64];
    char    port[8];

    memset(ip,      0, sizeof(ip));
    memset(port,    0, sizeof(port));

    char    tempUri[URI_MAX_SIZE];
    memset(tempUri, 0, sizeof(tempUri));

    if(extractHostAndPortFromUri(ip,
                                 port,
                                 a_sHttpData.GetUri(tempUri, sizeof(tempUri))) != true)
    {
        gLog->WARNING("can't parsing URI [%s]", tempUri);
        return false;
    }

    if(m_pSession == NULL && Init() != true)
    {
        return false;
    }

    if(m_pSession->Connect(ip, atoi(port)) != true)
    {
        gLog->WARNING("%-24s| send - can't connect [%s:%s] [%s]",
            "HttpClient",
            ip,
            port,
            m_pSession->m_strErrorMsg.c_str());

        delete m_pSession;
        m_pSession = NULL;

        return false;
    }

    a_sHttpData.MakeData();
    if(m_pSession->Send(a_sHttpData.GetPtr(), a_sHttpData.GetSize()) < 0)
    {
        gLog->WARNING("%-24s| can't send [%s:%s] [%s] [%s]",
            "HttpClient",
            ip,
            port,
            tempUri,
            m_pSession->m_strErrorMsg.c_str());

        delete m_pSession;
        m_pSession = NULL;

        return false;
    }

    gLog->DEBUG("%-24s| Send [%s]",
        "HttpClient",
        a_sHttpData.GetPtr());

    return true;
}

HttpData & HttpClient::
GetData()
{
    m_rHttpData.Clear();

    if(m_pSession == NULL)
        return m_rHttpData;

    // TO DO : 3초는 설정 값으로 해야 합니다.
    if(m_rHttpData.RecvLine(m_pSession, 3) <= 0)
    {
        gLog->WARNING("recvLine fail");
        return m_rHttpData;
    }

    gLog->DEBUG("---- RECV ---- ");
    gLog->DEBUG("%s", m_rHttpData.GetLine());

    // TO DO : 3초는 설정 값으로 해야 합니다.
    if(m_rHttpData.RecvHeader(m_pSession, 3) <= 0)
        return m_rHttpData;

    gLog->DEBUG("%s", m_rHttpData.GetHeader());

    // TO DO : 3초는 설정 값으로 해야 합니다.
    m_rHttpData.RecvBody(m_pSession, 3);
    gLog->DEBUG("%s", m_rHttpData.GetBody());

    return m_rHttpData;
}

bool HttpClient::
extractHostAndPortFromUri(char * a_pHost, char * a_pPort, const char * a_pUri)
{
    // example : http://127.0.0.1:8000/...
    // example : http://www.ntels.com:8000/...

    char * p = const_cast<char *>(a_pUri);

    if((p = strstr(p, "//")) == NULL)
    {
        gLog->WARNING("%-24s| extractHostAndPortFromUri - Can't parsing URI [%s]",
            "HttpClient",
            a_pUri);
        return false;
    }

    p++;
    p++;

    if(sscanf(p, "%[^:]:%[^\x01-\x2f]", a_pHost, a_pPort) != 2)
    {
        gLog->WARNING("extractHostAndPortFromUri fail [%s]", a_pUri);
        return false;
    }

    if(strlen(a_pHost) == 0 || strlen(a_pPort) == 0)
    {
       gLog->WARNING("%-24s| extractHostAndPortFromUri fail - host[%s] or port[%s]",
            "HttpClient",
            a_pHost,
            a_pPort);
       return false;
    }

    return true;
}