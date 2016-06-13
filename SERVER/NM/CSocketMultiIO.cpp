/*
 */

#include <cstdio>

#include "CSocketMultiIO.hpp"

using std::map;


CSocketMultiIO::CSocketMultiIO()
{
	// member 변수 초기화
	m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();
    
    FD_ZERO(&m_stReadSet); 
	m_nMaxFd = -1;
    
    return;
}

CSocketMultiIO::~CSocketMultiIO(void)
{
	map<int, CMesgExchSocket*>::iterator iter = m_mapSockets.begin();
	for (; iter != m_mapSockets.end();) {
		delete iter->second;
	}
	m_mapSockets.clear();

	m_lstReadable.clear();
	
	return;
}

CMesgExchSocket* CSocketMultiIO::Select(int a_nTimeoutSec, int a_nTimeoutMSec)
{
	m_enError = CSocket::SOCK_OK;
    m_strErrorMsg.clear();

	if (!m_lstReadable.empty()) {
		CMesgExchSocket* sock = m_lstReadable.front();
		m_lstReadable.pop_front();
		return sock;
	}

	timeval waitTime;
    waitTime.tv_sec  = a_nTimeoutSec;
    waitTime.tv_usec = a_nTimeoutMSec * 1000;

	timeval* pWait = &waitTime;
	if (a_nTimeoutSec == 0 && a_nTimeoutMSec == 0) {
		pWait = NULL;
	}


	fd_set rfdset;
    int nRead;
	map<int, CMesgExchSocket*>::iterator iter;

    for(;;) {
		rfdset = m_stReadSet;
        nRead = select(m_nMaxFd + 1, &rfdset, 0, 0, pWait);

        if (nRead > 0) {
        	break;
        }
		if (nRead == 0) {
			m_enError = CSocket::SOCK_TIMEOUT;
        	m_strErrorMsg = "Select timeout";
			return NULL;
    	}
        if (nRead < 0) {
        	if (errno == EINTR) {
				continue ;
			}
			m_enError = CSocket::SOCK_ERROR;
			m_strErrorMsg = "Select failed";
			return NULL;
        }
    }
    
	// check client socket fd
	iter = m_mapSockets.begin();
	for (int cnt=0; iter != m_mapSockets.end(); iter++) {
		if (FD_ISSET(iter->first, &rfdset)) {
			m_lstReadable.push_back(iter->second);
			cnt++;
		}
		if (cnt == nRead) {				// select event 갯수 만큰만 loop 돌도록
			break;
		}
	}

	if (m_lstReadable.empty()) {
		return NULL;
	}
	CMesgExchSocket* sock = m_lstReadable.front();
	m_lstReadable.pop_front();
	return sock;
}

CMesgExchSocket* CSocketMultiIO::FindSocket(int a_nSocket)
{
	map<int, CMesgExchSocket*>::iterator iter = m_mapSockets.find(a_nSocket);
	if (iter == m_mapSockets.end()) {
		return NULL;
	}
	
	return iter->second;
}

bool CSocketMultiIO::AddSocket(int a_nSocket)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();

	CMesgExchSocket* client = new CMesgExchSocket(a_nSocket);
	if (client->m_enError != CSocket::SOCK_OK) {
		delete client;
		m_enError = CSocket::SOCK_ERROR;
		m_strErrorMsg = "Add client socket failed";
		return false;
	}

    m_mapSockets.insert(m_mapSockets.end(), 
					map<int, CMesgExchSocket*>::value_type(a_nSocket, client));
	FdSetAdd(a_nSocket);

	return true;
}

void CSocketMultiIO::CloseSocket(int a_nSocket)
{
	m_enError = CSocket::SOCK_OK;
	m_strErrorMsg.clear();

	if (a_nSocket < 0) {
		return;
	}

	FdSetClear(a_nSocket);

	map<int, CMesgExchSocket*>::iterator iter = m_mapSockets.find(a_nSocket);
	if (iter != m_mapSockets.end()) {
		delete iter->second;
		m_mapSockets.erase(iter);
	}

	return;
}

void CSocketMultiIO::FdSetAdd(int a_nFd)
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

void CSocketMultiIO::FdSetClear(int a_nFd)
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
