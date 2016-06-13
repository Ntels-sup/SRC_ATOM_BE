/* vim:ts=4:sw=4
 */
/**
 * \file	CAppQueue.hpp
 * \brief	APP 프로세스 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CAPPQUEUE_HPP_
#define CAPPQUEUE_HPP_

#include <list>
#include <map>
#include <vector>

// common header
#include "CProtocol.hpp"
#include "CLQManager.hpp"


class CAppQueue
{
public:
	static CAppQueue& Instance(void);
	 	
	bool	OpenQ(CLQManager* a_pclsQ = NULL);
	bool	CmdRingClear(const char* a_szProcName);
	bool	SendCmd(CProtocol& a_clsProto, const char* a_szTarget);
	int		RecvCmd(CProtocol& a_clsProto);
	
private:
	CLQManager*	m_pclsQ;
	
	static CAppQueue*  m_pclsInstance;          // singleton instance
	CAppQueue();
	CAppQueue(CAppQueue&) {}
	~CAppQueue();
};

#endif //CAPPQUEUE_HPP_
