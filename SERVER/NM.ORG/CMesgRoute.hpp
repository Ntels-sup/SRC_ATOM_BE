/* vim:ts=4:sw=4
 */
/**
 * \file	CMesgRoute.hpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */
 
#ifndef CMESGROUTE_HPP_
#define CMESGROUTE_HPP_

#include <iostream>
#include <map>
#include <list>

#include "CModuleIPC.hpp"


class CMesgRoute
{
public:
	std::string	m_strErrorMsg;

public:
	static CMesgRoute& Instance();

	void	SetLocalNodeNo(int a_nNodeNo) { m_nLocalNodeNo = a_nNodeNo; }
	int		GetLocalNodeNo(void) { return m_nLocalNodeNo; }

	bool	AddRouteSock(int a_nDstNode, int a_nDstProc, int a_nToSock);
	bool	FindRouteSock(int a_nDstNode, int a_nDstProc, int& a_nToSock);
	bool	DelRouteSock(int a_nToProc);
	bool	GetNewRouteSock(std::list<int>& a_lstSock);
	
	bool	AddRouteThread(int a_nDstNode, int a_nDstProc, CModuleIPC::EN_THREAD a_nDthread);
	bool	FindRouteThread(int a_nDstNode, int a_nDstProc, CModuleIPC::EN_THREAD& a_nDthread);
	bool	DelRouteThread(CModuleIPC::EN_THREAD a_nDthread);

	void	PrintTable(CFileLog* a_pclsLog, int a_nLogLevel);


private:
	int		m_nLocalNodeNo;
	
	// key   first: dst nodeid, second: dst procid
	// value route socket
	typedef std::map<std::pair<int, int>, int> rtsock_t;
	rtsock_t		m_mapSockRoute;
	std::list<int>	m_lstNewSock;
	pthread_mutex_t m_tSockLock;

	// key   first: dst nodeid, second: dst procid
	// value route thread
	typedef std::map<std::pair<int, int>, CModuleIPC::EN_THREAD> rtthread_t; 
	rtthread_t		m_mapThreadRoute;
	pthread_mutex_t m_tThreadLock;
	
	CMesgRoute();
	CMesgRoute(const CMesgRoute&) {}
	~CMesgRoute();

	static CMesgRoute*		m_pcInstance;
};

#endif // CMESGROUTE_HPP_
