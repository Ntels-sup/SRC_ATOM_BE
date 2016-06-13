
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
	
#include "CSocket.hpp"


CSocket::CSocket(EN_TYPE a_enType)
{
    m_nSockFd = -1;
    m_enType = IP_TCP;
    m_strErrorMsg.clear();
    m_enError = SOCK_OK;
    m_enType = a_enType;
	m_bAutoClose = true;
    return;
}

CSocket::CSocket(int a_nSockFd)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	m_bAutoClose = true;

    if (IsValiedSocket(a_nSockFd) == false) {
		m_enError = SOCK_TYPE;
		m_strErrorMsg = "invalied socket descriptor";
		return;		
    }

	int domain = 0;
	socklen_t optlen = sizeof(domain);
	if (getsockopt(a_nSockFd, SOL_SOCKET, SO_DOMAIN, &domain, &optlen) < 0) {
		m_enError = SOCK_TYPE;
		m_strErrorMsg = "unknown socket domain";
		return;
	}
	
	int protocol = 0;
	optlen = sizeof(protocol);
	if (getsockopt(a_nSockFd, SOL_SOCKET, SO_PROTOCOL, &protocol, &optlen) < 0) {
		m_enError = SOCK_TYPE;
		m_strErrorMsg = "unknown socket protocol";
		return;
	}
	
	if (domain == AF_INET && protocol == IPPROTO_TCP) {
		m_enType = IP_TCP;
	} else if (domain == AF_INET && protocol == IPPROTO_UDP) {
		m_enType = IP_UDP;
	} else if (domain == AF_INET6 && protocol == IPPROTO_TCP) {
		m_enType = IP6_TCP;
	} else if (domain == AF_INET6 && protocol == IPPROTO_UDP) {
		m_enType = IP6_UDP;
	} else if (domain == AF_UNIX && protocol == IPPROTO_TCP) {
		m_enType = UN_TCP;
	} else if (domain == AF_UNIX && protocol == IPPROTO_UDP) {
		m_enType = UN_UDP;
	} else {
		m_enError = SOCK_TYPE;
		m_strErrorMsg = "not support socket type, domain or protocol";
		return;			
	}

	m_nSockFd = a_nSockFd;

	return;
}

CSocket::CSocket(const CSocket& a_cSock)
{
	m_nSockFd		= a_cSock.m_nSockFd;
	m_enType		= a_cSock.m_enType;
    m_strErrorMsg.clear();
    m_enError		= SOCK_OK;
    m_enType		= a_cSock.m_enType;
	m_bAutoClose	= a_cSock.m_bAutoClose;

	return;
}

bool CSocket::Connect(const char* a_szIP, int a_nPort, int a_nTimeoutSec)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;

    switch(m_enType) {
        case IP_TCP :
        case IP_UDP :
        case IP6_TCP :
        case IP6_UDP :
        case UN_TCP :
        case UN_UDP :
            break ;
        default :
            m_enError = SOCK_TYPE;
            m_strErrorMsg = "invalid socket type";
            return false;
    }

    // 연결된 session은 close
	Close();
	if (SocketCreate() == false) {
		return false;
	}
    
    socklen_t addrlen = 0;
	sockaddr* addr = NULL;
    sockaddr_in addr4;
	sockaddr_in6 addr6;

    if (m_enType == IP_TCP || m_enType == IP_UDP) {
		memset(&addr4, 0x00, sizeof(addr4));
	    addrlen = sizeof(addr4);
        
        addr4.sin_family = AF_INET;
	    if (inet_pton(AF_INET, a_szIP, &addr4.sin_addr) != 1) {
			Close();
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "invalied IP4 address format";
            return false;
        }
	    addr4.sin_port = htons(a_nPort);

		addr = (sockaddr*)&addr4;
    } else {
		memset(&addr6, 0x00, sizeof(addr6));
	    addrlen = sizeof(addr6);

	    addr6.sin6_family = AF_INET6;
	    if (inet_pton(AF_INET6, a_szIP, &addr6.sin6_addr) != 1) {
			Close();
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "invalied IP4 address format";
            return false;
        }
	    addr6.sin6_port = htons(a_nPort);

		addr = (sockaddr*)&addr6;
    }

    // connect시 timeout을 처리를 위한 NONBLOCK 설정
	int oflags = fcntl(m_nSockFd, F_GETFL, 0);
	fcntl(m_nSockFd, F_SETFL, oflags | O_NONBLOCK);

	if (connect(m_nSockFd, addr, addrlen) == 0) {
        fcntl(m_nSockFd, F_SETFL, oflags);   // BLOCK 설정 해제
		return true;
	} else {
        fcntl(m_nSockFd, F_SETFL, oflags);
        
        switch (errno) {
            case EISCONN :
                return true;
            case EINPROGRESS :            // 연결 진행 중
            case EALREADY :
                break;
            default :
				Close();
                m_enError = SOCK_ERROR;
	    		m_strErrorMsg = "socket connect error";
		    	return false;
		}
	}
    
    // connect 진행 중일 경우 timeout 처리
    fd_set rset, wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(m_nSockFd, &rset);
	wset = rset;
    
    timeval tval;
	if (a_nTimeoutSec > 0) {
    	tval.tv_sec = a_nTimeoutSec;
    	tval.tv_usec = 0;
	} else if (a_nTimeoutSec < 0) {
    	tval.tv_sec = 0;
    	tval.tv_usec = 1;
	}

    int ret = select(m_nSockFd + 1, &rset, &wset, NULL, ((a_nTimeoutSec == 0) ? NULL : &tval)); 
	if (ret <= 0) {
		Close();
        m_enError = SOCK_TIMEOUT;
		m_strErrorMsg = "socket connect timeout";        
		return false;
	} else {
		if (FD_ISSET(m_nSockFd, &rset) || FD_ISSET(m_nSockFd, &wset)) {
            int sockerr = 0;
			socklen_t socklen = sizeof(sockerr);
            
			if (getsockopt(m_nSockFd, SOL_SOCKET, SO_ERROR, &sockerr, &socklen) < 0) {
				Close();
				m_enError = SOCK_ERROR;
                m_strErrorMsg = "socket connect failed, call getsockopt";
				return false;
			} else if (sockerr) {
				Close();
				m_enError = SOCK_ERROR;
                m_strErrorMsg = "socket connect failed";
				return false;
			}
		}
	}

	return true;
}

bool CSocket::Connect(const char *a_szPath)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;

    if (m_enType != UN_TCP && m_enType != UN_UDP) {
        m_enError = SOCK_TYPE;
        m_strErrorMsg = "invalid socket type";
        return false;
    }
    
    // 연결된 socket close
    Close();
	if (SocketCreate() == false) {
		return false;
	}
	
	struct sockaddr_un addru;
	socklen_t addrlen = sizeof(addru);

	memset(&addru, 0x00, sizeof(addru));
	addru.sun_family = AF_LOCAL;
	strncpy(addru.sun_path, a_szPath, sizeof(addru.sun_path)-1);

    for (int i=0; i < 5; i++) {
        int ret = connect(m_nSockFd, (sockaddr*)&addru, addrlen);
        if (ret >= 0) {
            break;
        } else {
            switch (errno) {
                case EISCONN :
                    return true;
                case EINPROGRESS :            // 연결 진행 중
                case EALREADY :
                    sleep(1);
                    break;
                default :
                    m_enError = SOCK_ERROR;
                    m_strErrorMsg = "socket connect error";
                    return false;
            }
        }
    }

    return true;
}

void CSocket::Close(void)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();

    if (m_nSockFd >= 0 && m_bAutoClose) {
        close(m_nSockFd);
    }
    m_nSockFd = -1;

    return;
}

int CSocket::Send(const char* a_szBuff, int a_nSendLen)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
    
	if (m_nSockFd < 0) {
		m_enError = SOCK_DISCONNECT;
		m_strErrorMsg = "not connected";
		return -1;
	}

	if (a_szBuff == NULL || a_nSendLen == 0) {
		return 0;
	}
	
	int offset = 0;
	int sendn = 0;
	int stt = 0;
    bool loop = false;
	int bufflen = a_nSendLen;

	do {
		sendn = send(m_nSockFd, a_szBuff + offset, bufflen, MSG_NOSIGNAL);
		if (bufflen == sendn) {		    /* OK */
			stt += sendn;
			loop = false;
		} else if (sendn >= 0) {	    /* retry */
			offset += sendn;
			bufflen -= sendn;
			stt += sendn;
			loop = true;
		} else if (sendn < 0) {
			switch (errno) {
				case EAGAIN :			/* none blocking */
				case EINTR :			/* interrupt */
					loop = true;
					break;
				default :				/* other errno */
					stt = -1;
					loop = false;
			} //end switch
		}
	} while (loop);

	return stt;
}

int CSocket::Recv(char* a_szBuff, int a_nRecvLen, int a_nTimeout)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
    
	if (m_nSockFd < 0) {
		m_enError = SOCK_DISCONNECT;
		m_strErrorMsg = "not connected";
		return -1;
	}


	fd_set rset;
    FD_ZERO(&rset);
    
    int readn = 0;
    int rtt = 0;
    bool loop = false;

	int ret = 0;
	int offset = 0;
	int reqlen = a_nRecvLen;

	timeval tval = {0, 0};

	do {
        FD_ZERO(&rset);
		FD_SET(m_nSockFd, &rset);
        
		if (a_nTimeout < 0) {
			tval.tv_sec = 0;
			tval.tv_usec = 1;
		} else if (a_nTimeout > 0) {
			tval.tv_sec = a_nTimeout;
			tval.tv_usec = 0;
		}

		ret = select(m_nSockFd + 1, &rset, NULL, NULL, (a_nTimeout == 0) ? NULL : &tval);
		if (ret == 0) {
			m_enError = SOCK_TIMEOUT;
            m_strErrorMsg = "socket receive timeout";
            rtt = 0;
			break;
		} else if (ret < 0) {
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "socket receive failed";
            rtt = -1;
			break;
		}

		if (FD_ISSET(m_nSockFd, &rset)) {
			readn = recv(m_nSockFd, a_szBuff + offset, reqlen, 0);
			if (readn == 0) {					
				if (errno == EINPROGRESS) {		// socket none blocking
					return 0;
				}
                m_enError = SOCK_DISCONNECT;
                m_strErrorMsg = "socket disconnected";
				return -1;
			} else if (readn < 0) {
				switch (errno) {
					case EAGAIN :				// non-blocking
						loop = true;
						break;
					case EINTR :				// interrupt
						m_enError = SOCK_INTERRUPT;
						m_strErrorMsg = "socket receive interrupted";
                        rtt = -1;
						loop = false;
						break;
					default :
						m_enError = SOCK_ERROR;
						m_strErrorMsg = "socket receive failed";                    
						rtt = -1;
						loop = false;
				}
			} else if (reqlen == readn) {	/* OK */
				rtt += readn;
				loop = false;
			} else if (readn > 0 && reqlen != readn) {
				offset += readn;
				reqlen -= readn;
				rtt += readn;
				loop = true;
			}
		}
	} while (loop);

	return rtt;
}

bool CSocket::SetNoDelay(void)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
        
    if (m_enType == IP_TCP && m_enType == UN_TCP) {
        m_enError = SOCK_TYPE;
        m_strErrorMsg = "socket type invalied, only TCP";
        return false;
    }

    int optval = 1;
    return SetSocketOption(IPPROTO_TCP, TCP_NODELAY, (void *)&optval, sizeof(optval));    
}

bool CSocket::SetNonBlock(void)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
        
    if (m_nSockFd >= 0) {
        int oldflags = 0;

        if ((oldflags = fcntl(m_nSockFd, F_GETFL, 0)) < 0) {
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "socket control failed";
            return false;
        }

        int ret  = fcntl(m_nSockFd, F_SETFL, oldflags | O_NONBLOCK);
        if (ret < 0) {
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "socket control failed";
            return false;
        }

    } else {
        m_enError = SOCK_TYPE;
        m_strErrorMsg = "invalied socket descriptor";
        return false;
    }
    
    return true;
}

bool CSocket::SetBlock(void)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
        
    if (m_nSockFd >= 0) {
        int oldflags = 0;

        if ((oldflags = fcntl(m_nSockFd, F_GETFL, 0)) < 0) {
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "socket block1 failed";
            return false;
        }

        int ret  = fcntl(m_nSockFd, F_SETFL, oldflags & ~O_NONBLOCK);
        if (ret < 0) {
            m_enError = SOCK_ERROR;
            m_strErrorMsg = "socket block2 failed";
            return false;
        }

    } else {
        m_enError = SOCK_TYPE;
        m_strErrorMsg = "invalied socket descriptor";
        return false;
    }
    
    return true;
}

bool CSocket::SetRTSEventSocket(int a_nSigno)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
        
    if (a_nSigno < SIGRTMIN || a_nSigno > SIGRTMAX) {
        m_enError = SOCK_INTERRUPT;
        m_strErrorMsg = "invalid RTS signal";
        return false;
    } else {
        int oldflags = fcntl(m_nSockFd, F_GETFD, 0);
        fcntl(m_nSockFd, F_SETFL, oldflags|O_NONBLOCK|O_ASYNC);
        fcntl(m_nSockFd, F_SETSIG, a_nSigno);
        fcntl(m_nSockFd, F_SETOWN, getpid());
        // fcntl(m_nSockFd,F_SETAUXFL, O_ONESIGFD);
    }
    
    return true;
}

bool CSocket::SetSocketOption(int a_nLevel, int a_nOptname, void* a_pOptval, 
                                                            socklen_t a_nOptlen)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;
        
    if (m_nSockFd < 0) {
        m_enError = SOCK_ERROR;
        m_strErrorMsg = "invalied socket descriptor";
        return false;
    }

    if (setsockopt(m_nSockFd, a_nLevel, a_nOptname, a_pOptval, a_nOptlen) < 0) {
        m_enError = SOCK_ERROR;
        m_strErrorMsg = "socket control failed, setsokopt";
        return false;
    }

    return true;
}

bool CSocket::GetPeerIpPort(std::string* a_strIp, int* a_nPort)
{
    m_enError = SOCK_OK;
    m_strErrorMsg.clear();
	errno = 0;

	if (m_nSockFd < 0) { 
		m_enError = SOCK_ERROR;
		m_strErrorMsg = "socket is not connect";
		return NULL;
	}
			
	sockaddr* addr = NULL;
	sockaddr_in addr4;
	sockaddr_in6 addr6;
	socklen_t addrlen = 0;

	switch (m_enType) {
		case IP_TCP :
		case IP_UDP :
			addr = (sockaddr*)&addr4;
			addrlen = sizeof(&addr4);
			break;
		case IP6_TCP :
		case IP6_UDP :
			addr = (sockaddr*)&addr6;
			addrlen = sizeof(&addr6);
			break;
		default :
			m_enError = SOCK_TYPE;
			m_strErrorMsg = "invalied socket type, only IP socket";
			return NULL;
	}
		
	if (getpeername(m_nSockFd, addr, &addrlen) != 0) {
		m_enError = SOCK_ERROR;
		m_strErrorMsg = "socket peer name failed";
		return NULL;
	}


	char buffer[80];
	memset(buffer, 0x00, sizeof(buffer));

	switch (m_enType) {
		case IP_TCP:
		case IP_UDP:
			inet_ntop(AF_INET, &addr4.sin_addr, buffer, sizeof(buffer)-1);
			if (a_strIp) *a_strIp = buffer;
			if (a_nPort) *a_nPort = ntohs(addr4.sin_port);
			break;
		case IP6_TCP:
		case IP6_UDP:
			inet_ntop(AF_INET6, &addr6.sin6_addr, buffer, sizeof(buffer)-1);
			if (a_strIp) *a_strIp = buffer;
			if (a_nPort) *a_nPort = ntohs(addr6.sin6_port);
			break;
		default:
			break;
	}

	
	return true;
}

bool CSocket::IsConnected(void)
{
	if (m_nSockFd < 0) {
		return false;
	}
	return IsValiedSocket(m_nSockFd);
}

bool CSocket::SocketCreate(void)
{
	errno = 0;

	switch(m_enType) {
        case IP_TCP :
            m_nSockFd = socket(AF_INET, SOCK_STREAM, 0);
            break;
        case IP_UDP :
            m_nSockFd = socket(AF_INET, SOCK_DGRAM, 0);
            break;
        case IP6_TCP :
            m_nSockFd = socket(AF_INET6, SOCK_STREAM, 0);
            break;
        case IP6_UDP :
            m_nSockFd = socket(AF_INET6, SOCK_DGRAM, 0);
            break;
        case UN_TCP :
            m_nSockFd = socket(AF_UNIX, SOCK_STREAM, 0);
            break;
        case UN_UDP :
            m_nSockFd = socket(AF_UNIX, SOCK_DGRAM, 0);
            break;
        default :
            m_enError = SOCK_TYPE;
            m_strErrorMsg = "invalied socket type";
            return false;
    }

    if (m_nSockFd < 0) {
        m_enError = SOCK_ERROR;
        m_strErrorMsg = "socket create failed";
		return false;
    }

	return true;
}

bool CSocket::IsValiedSocket(int a_nSockFd)
{
	struct stat fdstat;
	errno = 0;

	if (fstat(a_nSockFd, &fdstat) < 0) {
        m_enError = SOCK_ERROR;
        m_strErrorMsg = "socket status failed, fstat";
        return false;
	}

	if ((fdstat.st_mode & S_IFMT) != S_IFSOCK) {
        m_enError = SOCK_ERROR;
        m_strErrorMsg = "invalied socket descriptor";
        return false;
	}
    
    return true;   
}
