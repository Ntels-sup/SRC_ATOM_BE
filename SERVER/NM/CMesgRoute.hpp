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


class CMesgRoute
{
public:
	std::string	m_strErrorMsg;

public:
	static CMesgRoute& Instance();

	void	SetLocalNodeNo(int a_nNodeNo) { m_nLocalNodeNo = a_nNodeNo; }
	int		GetLocalNodeNo(void) { return m_nLocalNodeNo; }

	bool	Lookup(int a_nDstNode, int a_nDstProc, int& a_nToRoute, bool& a_bIsSock);

	bool	AddRouteSock(int a_nDstNode, int a_nDstProc, int a_nToRoute);
	bool	AddRouteModule(int a_nDstNode, int a_nDstProc, int a_nToRoute);
	bool	DelRouteSock(int a_nToRoute);
	bool	DelRouteModule(int a_nToRoute);
	
	bool	GetNewRouteSock(std::list<int>& a_lstSock);

	void	PrintTable(CFileLog* a_pclsLog, int a_nLogLevel);


private:
	int		m_nLocalNodeNo;

	struct	ST_ROUTE {
		int		m_nToRoute;
		bool	m_bIsSock;
	};
	// key   first: dst nodeid, second: dst procid
	// value route socket
	typedef std::map<std::pair<int, int>, ST_ROUTE> route_t;
	route_t			m_mapRoute;
	std::list<int>	m_lstNewSock;
	pthread_mutex_t m_tMutex;

	CMesgRoute();
	CMesgRoute(const CMesgRoute&) {}
	~CMesgRoute();

	static CMesgRoute*		m_pclsInstance;
};

#endif // CMESGROUTE_HPP_
