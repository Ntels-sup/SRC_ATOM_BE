/* vim:ts=4:sw=4
 */
/**
 * \file	CCommandNa.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCOMMANDNA_HPP_
#define CCOMMANDNA_HPP_

#include <string>
#include <set>

#include "CProtocol.hpp"
#include "CMesgExchSocket.hpp"


class CCommandNa
{
public:
	int		m_nLastCmd;

public:
	CCommandNa() { m_nLastCmd = 0; }
	~CCommandNa() {}

	void	PermitCommand(int a_nCmd) {	m_setPermitCmd.insert(a_nCmd); }

	bool	ProcMesgSock(CProtocol& a_clsProto, CMesgExchSocket* a_pclsMsgEx);
	bool	ProcMesgModule(CProtocol& clsProto, CMesgExchSocket* a_pclsMsgEx);

private:
	bool	Command(CProtocol& a_clsProto, CMesgExchSocket* a_pclsExSock);
	bool	CmdPing(CProtocol& a_clsReq, CMesgExchSocket* a_pclsExSock);
	bool	CmdVnfReady(CProtocol& a_clsReq);
	bool	CmdVnfInstall(CProtocol& a_clsProto);
	bool	CmdVnfStart(CProtocol& a_clsProto);
	bool	CmdVnfStop(CProtocol& a_clsProto);

private:
	std::set<int> m_setPermitCmd;
};

#endif //CCOMMANDNA_HPP_
