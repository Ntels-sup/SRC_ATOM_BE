/* vim:ts=4:sw=4
 */
/**
 * \file	CModuleIPC.hpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */
 
#ifndef CMODULEIPC_HPP_
#define CMODULEIPC_HPP_

#include <map>
#include <deque>

#include "CProtocol.hpp"
#include "CMesgRoute.hpp"


class CModuleIPC
{
public:
	// error log
	std::string	m_strErrorMsg;
	
public:
	static CModuleIPC& Instance();
	
	bool	SendMesg(CProtocol& a_cMesg);
	int 	RecvMesg(int a_nMyProcNo, CProtocol& a_cMesg, int n_WaitTime = 0);
	int		FindProcNo(const char* a_szProcName);

	// NM에서만 호출 
	bool	Regist(const char* a_szProcName, int a_nProcNo);
	bool	UnRegist(int a_nProcNo);

private:
	struct ST_MSG {
		std::string				m_strProcName;
		std::deque<CProtocol>	m_deqMsg;
		pthread_mutex_t			m_tMutex;
		pthread_cond_t			m_tCond;
	};
	typedef std::map<int, ST_MSG> modipc_t;		//key: procno
	modipc_t	m_mapIPC;

	// routing talbe singletoon
	CMesgRoute&			m_clsRoute;

	CModuleIPC();
	CModuleIPC(const CModuleIPC&) : m_clsRoute(CMesgRoute::Instance()) {}
	~CModuleIPC() {}

	static CModuleIPC*  m_pcInstance;
};

#endif // CMODULEIPC_HPP_
