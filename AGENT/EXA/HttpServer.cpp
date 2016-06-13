
#include "HttpServer.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gLog;

HttpServer::
HttpServer()
{
    m_pServer  = NULL;
    m_pSession = NULL;
}

HttpServer::
~HttpServer()
{
    if(m_pSession != NULL)
    {
        m_pServer->ClosePeer(m_pSession);
        m_pSession = NULL;
    }

    if(m_pServer != NULL)
    {
        delete m_pServer;
        m_pServer = NULL;
    }
}

bool HttpServer::
Init(std::string a_sIP, int a_nPort)
{
    m_pServer = new (std::nothrow) CSocketServer(CSocket::IP_TCP);

    if(m_pServer == NULL)
    {
        gLog->WARNING("Init - new operator fail");
        return false;
    }

    m_pServer->SetReuseAddress();

    if(m_pServer->Listen(a_sIP.c_str(), a_nPort) != true)
    {
        gLog->WARNING("%-24s| Init - listen fail [%s:%d] [%s]",
            "HttpServer",
            a_sIP.c_str(),
            a_nPort,
            m_pServer->m_strErrorMsg.c_str());
        return false;
    }

    return true;
}

HttpData & HttpServer::
GetData()
{
    m_hData.Clear();

    CSocket * tempSession = NULL;

    if((tempSession = m_pServer->Select(0, 5)) == NULL)
        return m_hData;

    m_pSession = tempSession;

    if(m_hData.RecvLine(m_pSession) < 0)
    {
        m_pServer->ClosePeer(m_pSession);
        m_pSession = NULL;
        return m_hData;
    }

    gLog->DEBUG("---- RECV ---- ");
    gLog->DEBUG("%s", m_hData.GetLine());

    if(m_hData.RecvHeader(m_pSession) < 0)
    {
        m_pServer->ClosePeer(m_pSession);
        m_pSession = NULL;
        return m_hData;
    }
    gLog->DEBUG("%s", m_hData.GetHeader());

    if(m_hData.RecvBody(m_pSession) < 0)
    {
        m_pServer->ClosePeer(m_pSession);
        m_pSession = NULL;
        return m_hData;
    }
    gLog->DEBUG("[%s]", m_hData.GetBody());

    return m_hData;
}

bool HttpServer::
Send(HttpData & a_hData)
{
    if(a_hData.IsFill() != true)
    {
        // gLog->DEBUG("------ HttpServer Send, buf.. Empty");
        return true;
    }

    if(m_pSession == NULL)
    {
        gLog->WARNING("can't send - session is null");
        return true;
    }

    // 결과에 관계 없이 끊습니다.
    a_hData.MakeData();
    m_pSession->Send(a_hData.GetPtr(), a_hData.GetSize());

    m_pServer->ClosePeer(m_pSession);
    m_pSession = NULL;

    gLog->DEBUG("SEND [%s]", a_hData.GetPtr());

    return true;
}

bool HttpServer::
Send(const vnfm::eStatusCode a_eCode, const std::string & a_strMsg)
{
    if(m_pSession == NULL)
    {
        gLog->WARNING("can't send - session is null");
        return true;
    }

    m_hData.Clear();

    m_hData.AddLine("HTTP/1.1",
                     std::to_string((long long)a_eCode).c_str(),
                     a_strMsg.c_str());

    m_hData.MakeData();
    m_pSession->Send(m_hData.GetPtr(), m_hData.GetSize());

    m_pServer->ClosePeer(m_pSession);
    m_pSession = NULL;

    gLog->DEBUG("SEND [%s]", m_hData.GetPtr());

    return true;
}
