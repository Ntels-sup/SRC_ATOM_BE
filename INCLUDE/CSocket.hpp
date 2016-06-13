#ifndef CSOCKET_HPP_
#define CSOCKET_HPP_

#include <ctime>
#include <sys/time.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/poll.h>

#include <string>

class CSocket
{
public:
    // socket type 정의
    enum EN_TYPE 
    {
        IP_TCP,
        IP_UDP,
        IP6_TCP,
        IP6_UDP,
        UN_TCP,
        UN_UDP
    };

    // socket error 정의
    enum EN_ERROR
    {
        SOCK_OK,
        SOCK_TYPE,
        SOCK_TIMEOUT,
        SOCK_INTERRUPT,
        SOCK_DISCONNECT,
        SOCK_ERROR
    } m_enError;
    
    // socket error message
    std::string m_strErrorMsg;

public:
    CSocket(EN_TYPE a_enType = IP_TCP);
    CSocket(int a_nSockFd);
	CSocket(const CSocket& a_cSock);
    ~CSocket() { Close(); }
	
	bool    Connect(const char* a_szIP, int a_nPort, int a_nTimeoutSec = 0);
	bool    Connect(const char* a_szPath);
	void    Close(void);

	int     Send(const char* a_szBuff, int a_nSendLen);
	int     Recv(char* a_szBuff, int a_nRecvLen, int a_nTimeout = 0);

	bool    SetNoDelay(void);
	bool    SetNonBlock(void);
	bool    SetBlock(void);
	bool    SetRTSEventSocket(int a_nSigno);
    bool    SetSocketOption(int a_nLevel, int a_nOptname, void* a_nOptval,
                                                         socklen_t a_nOptlen);
	void	SetNoAutoClose(void) { m_bAutoClose = false; }
	                                                        
    int     GetSocket(void) { return this->m_nSockFd; }
	bool	GetPeerIpPort(std::string* a_strIp, int* a_nPort);
	bool	IsConnected(void);

private:
	bool	SocketCreate(void);
    bool    IsValiedSocket(int a_nSockFd);
    
    int     m_nSockFd;
    EN_TYPE m_enType;
    int		m_bAutoClose;
};

#endif //CSOCKET_HPP_
