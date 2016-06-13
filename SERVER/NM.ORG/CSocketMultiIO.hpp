/*

 */

#ifndef CSOCKETMULTIIO_HPP_
#define CSOCKETMULTIIO_HPP_

#include <map>
#include <string>
#include <list>

#include "CMesgExchSocket.hpp"


class CSocketMultiIO
{
public:
	// socket error message
	CSocket::EN_ERROR m_enError;
	std::string	m_strErrorMsg;

public:
	CSocketMultiIO(void);
	~CSocketMultiIO(void);

	CMesgExchSocket*	Select(int a_nTimeoutSec, int a_nTimeoutMSec);
	//CMesgExchSocket*	Epoll(void);
	
	CMesgExchSocket*	FindSocket(int a_nSocket);

	bool		AddSocket(int a_nSocket);
	void		CloseSocket(int a_nSocket);

private:
	void		FdSetAdd(int a_nFd);
	void		FdSetClear(int a_nFd);

	std::map<int, CMesgExchSocket*>	m_mapSockets;
	std::list<CMesgExchSocket*>		m_lstReadable;

	fd_set		m_stReadSet;
	int			m_nMaxFd;
};

#endif //CSOCKETMULTIIO_HPP_
