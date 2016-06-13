/*

 */

#ifndef CSOCKETMULTIIO_HPP_
#define CSOCKETMULTIIO_HPP_

#include <map>
#include <string>

#include "CSocket.hpp"


class CSocketMultiIO
{
public:
	// socket error message
	CSocket::EN_ERROR m_enError;
	std::string	m_strErrorMsg;

public:
    CSocketMultiIO(void);
    ~CSocketMultiIO(void);

    CSocket*    Select(int a_nTimeoutSec, int a_nTimeoutMSec);
    //CSocket*    Epoll(void);

	bool		AddSocket(int a_nSockFd);
	bool		AddSocket(CSocket* a_pclsSock);

	bool		DelSocket(int a_nSockFd);
	bool		DelSocket(CSocket* a_pclsSock);

	void		ClosePeer(CSocket* a_pclsSock);

private:
	void		FdSetAdd(int a_nFd);
	void		FdSetClear(int a_nFd);

    std::map<int, CSocket*> m_mapSockets;

    fd_set      m_stReadSet;
	int			m_nMaxFd;
};

#endif //CSOCKETMULTIIO_HPP_
