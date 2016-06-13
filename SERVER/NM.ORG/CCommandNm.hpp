/* vim:ts=4:sw=4
 */
/**
 * \file	CCommandNm.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCOMMANDNM_HPP_
#define CCOMMANDNM_HPP_

#include <string>

#include "CProtocol.hpp"
#include "CMesgExchSocket.hpp"
#include "CMesgExchSocketServer.hpp"


class CCommandNm
{
public:
	int		m_nLastProcCmd;

public:
	CCommandNm() { m_nLastProcCmd = 0; }
	~CCommandNm() {}

	bool	MesgProcess(CProtocol& a_clsProto, CMesgExchSocketServer& a_clsServ,
															CSocket* a_pclsCli);
	static bool	AlarmNodeStatus(int a_nId, const char* a_szStatus);

private:
	bool	CmdRegist(CProtocol& a_clsReq, CMesgExchSocketServer& a_clsServ, 
															CSocket* a_pclsCli);
	bool	CmdRegistNode(CProtocol& a_clsReq, CMesgExchSocketServer& a_clsServ,
															CSocket* a_pclsCli);
	bool	CmdPing(CProtocol& a_clsReq);
};

#endif //CCOMMANDNM_HPP_
