/*
 */

#include <cstdio>

#include "CSocketServer.hpp"

using std::map;


CSocketServer::CSocketServer(CSocket::EN_TYPE a_enType)
{
	// member 변수 초기화
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
    
	m_nListenSockFd = -1;
		
    FD_ZERO(&m_stReadSet); 
	m_nMaxFd = -1;
    
	m_enType = CSocket::IP_TCP;
	m_bStream = false;

    switch(a_enType) {
        case CSocket::IP_TCP :
            m_nListenSockFd = socket(AF_INET, SOCK_STREAM, 0);
            m_bStream = true;
            break;
        case CSocket::IP_UDP :
            m_nListenSockFd = socket(AF_INET, SOCK_DGRAM, 0);
            break;
        case CSocket::IP6_TCP :
            m_nListenSockFd = socket(AF_INET, SOCK_STREAM, 0);
            m_bStream = true;
            break;
        case CSocket::IP6_UDP :
            m_nListenSockFd = socket(AF_INET, SOCK_DGRAM, 0);
            break;            
        case CSocket::UN_TCP :
            m_nListenSockFd = socket(AF_UNIX, SOCK_STREAM, 0);
            m_bStream = true;
            break;
        case CSocket::UN_UDP :
            m_nListenSockFd = socket(AF_UNIX, SOCK_DGRAM, 0);
            break;
        default :
            m_enError = CSocket::SOCK_TYPE;
            m_strErrorMsg = "invalied socket type";
            return;
	}

	if (m_nListenSockFd < 0) {
		m_enError = CSocket::SOCK_ERROR;
		m_strErrorMsg = "socket create failed";
	}
    
	m_enType = a_enType;
    
	return;
}

CSocketServer::~CSocketServer(void)
{
	CloseServer();
	map<int, CSocket*>::iterator iter = m_mapSockets.begin();
	for (; iter != m_mapSockets.end(); iter++) {
		delete iter->second;
	}
	m_mapSockets.clear();

	return;
}

bool CSocketServer::Listen(const char* a_szIP, int a_nPort)
{
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
		
    if (m_enType == CSocket::UN_TCP || m_enType == CSocket::UN_UDP) {
        m_enError = CSocket::SOCK_TYPE;
        m_strErrorMsg = "invalied socket type";
		return false;
    }

    if (m_nListenSockFd < 0) {
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "socket create failed";
		return false;
	}

	SetReuseAddress();

    socklen_t addrlen = 0;
    sockaddr* addr = NULL;
	sockaddr_in addr4;
	sockaddr_in6 addr6;
	
    if (m_enType == CSocket::IP_TCP || m_enType == CSocket::IP_UDP) { // IP4
    	memset(&addr4, 0x00, sizeof(addr4));
        
        addr4.sin_family = AF_INET;
	    if (inet_pton(AF_INET, a_szIP, &addr4.sin_addr) != 1) {
            m_enError = CSocket::SOCK_ERROR;
            m_strErrorMsg = "invalied IP4 address format";
            return false;
        }
	    addr4.sin_port = htons(a_nPort);
		
		addr = (sockaddr*)&addr4;
	    addrlen = sizeof(addr4);
    } else {                                               			// IP6
		memset(&addr6, 0x00, sizeof(addr6));

	    addr6.sin6_family = AF_INET6;
	    if (inet_pton(AF_INET6, a_szIP, &addr6.sin6_addr) != 1) {
            m_enError = CSocket::SOCK_ERROR;
            m_strErrorMsg = "invalied IP4 address format";
            return false;
        }
	    addr6.sin6_port = htons(a_nPort);        

		addr = (sockaddr*)&addr6;
	    addrlen = sizeof(addr6);
    }

	if (bind(m_nListenSockFd, addr, addrlen) < 0) {
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "listen socket bind failed";
		return false;
	}

    if (m_bStream) {
	    listen(m_nListenSockFd, 5);
    }

	// select 위해 fdset 등록
	FdSetAdd(m_nListenSockFd);

	return true;
}

bool CSocketServer::Listen(const char* a_szPath)
{
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
	
    if (m_enType != CSocket::UN_TCP && m_enType == CSocket::UN_UDP) {
        m_enError = CSocket::SOCK_TYPE;
        m_strErrorMsg = "invalied socket type";
		return false;
    }

    if (m_nListenSockFd < 0) {
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "invalied socket descriptor";
		return false;
	}

	SetReuseAddress();

    // 기존 unix socket file 삭제
	unlink(a_szPath);
    
 	sockaddr_un addru;
	memset(&addru, 0, sizeof(addru));
	
	addru.sun_family = AF_UNIX;
	strncpy(addru.sun_path, a_szPath, sizeof(addru.sun_path)-1);

	if (bind(m_nListenSockFd, (sockaddr *)&addru, sizeof(addru)) < 0) {
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "socket bind failed";
		return false;
	}

    if (m_bStream) {
	    listen(m_nListenSockFd, 5);
    }
	
	// select 위해 fdset 등록
	FdSetAdd(m_nListenSockFd);

	return true;
}

bool CSocketServer::SetReuseAddress(void)
{
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
	
    int opt = 1;
	if (setsockopt(m_nListenSockFd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt,
                                                            sizeof(opt)) < 0) {
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "listen port reused failed"; 
        return false;
    }
    
    return true;
}

CSocket* CSocketServer::Accept(void)
{
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
	
    socklen_t addrlen = 0;
	sockaddr* addr = NULL;
    sockaddr_in addr4;
	sockaddr_in6 addr6;
	sockaddr_un addru;

    switch (m_enType) {
        case CSocket::IP_TCP :
			addr = (sockaddr*)&addr4;
            addrlen = sizeof(addr4);
            break;
        case CSocket::IP6_TCP :
			addr = (sockaddr*)&addr6;
            addrlen = sizeof(addr6);
            break;
        case CSocket::UN_TCP :
			addr = (sockaddr*)&addru;
            addrlen = sizeof(addru);
            break;
        default :
            m_enError = CSocket::SOCK_TYPE;
            m_strErrorMsg = "invalied socket type";
            return NULL;
    }
    
    int sfd = accept(m_nListenSockFd, addr, &addrlen);
    if (sfd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return NULL;
        }

        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "socket accept failed";
        return NULL;
    }
    
    CSocket* client = new CSocket(sfd);
    if (client->m_enError != CSocket::SOCK_OK) {
		delete client;
        m_enError = CSocket::SOCK_ERROR;
        m_strErrorMsg = "create client socket failed";
        return NULL;
    }

    m_mapSockets.insert(m_mapSockets.end(), 
						map<int, CSocket*>::value_type(sfd, client));
	FdSetAdd(sfd);

    return client;
}

bool CSocketServer::AddPeer(int a_nSockFd)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();

	CSocket* client = new CSocket(a_nSockFd);
	if (client->m_enError == CSocket::SOCK_OK) {
		delete client;
		m_enError = CSocket::SOCK_ERROR;
		m_strErrorMsg = "add client socket failed";
		return false;
	}

    m_mapSockets.insert(m_mapSockets.end(), 
						map<int, CSocket*>::value_type(a_nSockFd, client));
	FdSetAdd(a_nSockFd);

	return true;
}

bool CSocketServer::DelPeer(int a_nSockFd)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();

	// 등록된 socket을 삭제한다. 삭제시 socket fd는 close 하지 않는다.
	map<int, CSocket*>::iterator iter = m_mapSockets.find(a_nSockFd);
	if (iter != m_mapSockets.end()) {
		iter->second->SetNoAutoClose();
		delete iter->second;
		m_mapSockets.erase(a_nSockFd);
	}
	FdSetClear(a_nSockFd);

	return true;
}

CSocket* CSocketServer::Select(int a_nTimeoutSec, int a_nTimeoutMSec)
{
    m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();

	timeval waitTime;
    waitTime.tv_sec  = a_nTimeoutSec;
    waitTime.tv_usec = a_nTimeoutMSec;

	timeval* pWait = &waitTime;
	if (a_nTimeoutSec == 0 && a_nTimeoutMSec == 0) {
		pWait = NULL;
	}


	fd_set rfdset;
    int ret;
	map<int, CSocket*>::iterator iter;

    for(;;) {
		rfdset = m_stReadSet;
        ret = select(m_nMaxFd + 1, &rfdset, 0, 0, pWait);
        if (ret < 0 &&  errno == EINTR) {
	        continue ;
        }

	    if (ret < 0) {
			m_enError = CSocket::SOCK_ERROR;
			m_strErrorMsg = "select failed";
			return NULL;
    	}
    	if (ret == 0) {
			m_enError = CSocket::SOCK_TIMEOUT;
        	m_strErrorMsg = "select timeout";
			return NULL;
    	}
	
		// check server listen fd
    	if (FD_ISSET(m_nListenSockFd, &rfdset)) {
			Accept();
    	}
	
		// check client socket fd
		iter = m_mapSockets.begin();
		for (; iter != m_mapSockets.end(); iter++) {
			if (FD_ISSET(iter->first, &rfdset)) {
				return iter->second;
			}
		}
    }

    return NULL;
}

 
void CSocketServer::CloseServer(void)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();
	
    if (this->m_nListenSockFd >= 0) {
		FdSetClear(m_nListenSockFd);
        close(this->m_nListenSockFd) ;
    }
    this->m_nListenSockFd = -1;
    
    return;
}

void CSocketServer::ClosePeer(CSocket* a_pcPeer)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();

	if (a_pcPeer == NULL) {
		return;
	}

	int fd = a_pcPeer->GetSocket();
	FdSetClear(fd);

	map<int, CSocket*>::iterator iter = m_mapSockets.find(fd);
	if (iter != m_mapSockets.end()) {
		delete iter->second;
		m_mapSockets.erase(iter);
	}

	return;
}

void CSocketServer::FdSetAdd(int a_nFd)
{
	if (a_nFd < 0) {
		return;
	}

	FD_SET(a_nFd, &m_stReadSet);
	if (m_nMaxFd < a_nFd) {
		m_nMaxFd = a_nFd;
	}

	return;
}

void CSocketServer::FdSetClear(int a_nFd)
{
	if (a_nFd < 0) {
		return;
	}

	FD_CLR(a_nFd, &m_stReadSet);

	if (m_nMaxFd > a_nFd) {
		return;
	}

	for (int fd = a_nFd-1; fd >= 0; fd--) {
		if (FD_ISSET(fd, &m_stReadSet)) {
			m_nMaxFd = fd;
			return;
		}
	}
	
	m_nMaxFd = -1;

	return;
}
