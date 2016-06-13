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
#include <pthread.h>

#include "CProtocol.hpp"


class CModuleIPC
{
public:
	// error log
	std::string	m_strErrorMsg;

	enum EN_THREAD {
		TH_ROUTING = 1,				// agent, atom server process
		TH_PRA,						// NM PRA module
		TH_RSA,						// NM RSA module
		TH_NM						// NM
	};
	
public:
	static CModuleIPC& Instance();
	
	bool	SendMesg(EN_THREAD a_enTo, CProtocol& a_cMesg);
	int 	RecvMesg(EN_THREAD a_enTo, CProtocol& a_cMesg, int n_WaitTime = -1);

private:
	struct ST_MSG {
		std::deque<CProtocol>	m_deqMsg;
		pthread_mutex_t			m_tMutex;
		pthread_cond_t			m_tCond;
	};
	typedef std::map<EN_THREAD, ST_MSG> thipc_t;
	thipc_t						m_mapIPC;

	CModuleIPC();
	CModuleIPC(const CModuleIPC&) {}
	~CModuleIPC() {}
	
	static CModuleIPC*		m_pcInstance;
};

#endif // CTHREADIPC_HPP_
