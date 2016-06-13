/*

 */

#ifndef CSOCKETSERVER_HPP_
#define CSOCKETSERVER_HPP_

#include <map>
#include <string>

#include "CSocket.hpp"


class CSocketServer
{
public:
    // socket error
    CSocket::EN_ERROR m_enError;
    std::string m_strErrorMsg;


public:
    CSocketServer(CSocket::EN_TYPE a_enType);
    ~CSocketServer(void);

    bool        Listen(const char* a_szIP, int a_nPort);
    bool        Listen(const char* a_szPath);
    bool        SetReuseAddress(void);

    CSocket*    Accept(void);
	bool		AddPeer(int a_nSockFd);
	bool		DelPeer(int a_nSockFd);

    CSocket*    Select(int a_nTimeoutSec, int a_nTimeoutMSec);
    //CSocket*    Epoll(void);

    void        CloseServer(void);
	void		ClosePeer(CSocket* a_pcPeer);


private:
	void		FdSetAdd(int a_nFd);
	void		FdSetClear(int a_nFd);


    int         m_nListenSockFd;
    std::map<int, CSocket*> m_mapSockets;

    fd_set      m_stReadSet;
	int			m_nMaxFd;

    CSocket::EN_TYPE m_enType;
    bool        m_bStream;
};

#endif //CSOCKETSERVER_HPP_
