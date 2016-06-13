
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

#include "CFileLog.hpp"


class CMesgRoute
{
public:
	static CMesgRoute& Instance();
		
	void	SetLocalNodeNo(int a_nNodeNo);
	int		GetLocalNodeNo(void) { return m_nLocalNodeNo; }
	void	SetDefaultProcNo(int a_nProcNo);
	
	bool	Lookup(int a_nDstNodeNo, int a_nDstProcNo, int& a_nToProcNo);
	bool	AddRoute(int a_nDstProcNo, int a_nToProcNo);
	bool	DelRoute(int a_nDstProcNo = -1);

	void	PrintTable(CFileLog* a_pclsLog, int a_nLogLevel);
	
private:
	int		m_nDefaultProcNo;
	int		m_nLocalNodeNo;
	
	// key   first: dst nodeid, second: dst procid
	// value route procid
	typedef std::map<std::pair<int, int>, int> route_t; 
	route_t	m_mapRoute;
	pthread_mutex_t m_tRTlock;

	CMesgRoute();
	CMesgRoute(const CMesgRoute&) {}
	~CMesgRoute();
	
	static CMesgRoute*	m_pcInstance;
};

#endif // CMESGROUTE_HPP_
