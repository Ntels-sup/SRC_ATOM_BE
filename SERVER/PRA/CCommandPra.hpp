/* vim:ts=4:sw=4
 */
/**
 * \file	CCommandPra.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCOMMANDPRA_HPP_
#define CCOMMANDPRA_HPP_

#include <string>

#include "CProtocol.hpp"
#include "CCliReq.hpp"

#include "CCmdBase.hpp"
#include "CProcessManager.hpp"


class CCommandPra
{
public:
	CCommandPra() {}
	~CCommandPra() {}

	bool	ProcMesgModule(CProtocol& a_clsProto, CProcessManager& a_clsPM);

private:
	void	CmdPing(CProtocol& a_clsReq);
	void	CmdReconfig(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	
	void	CmdStaticApp(CProtocol& a_clsReq, CProcessManager& a_clsPMng);

	void	GetAppStatus(CProcessManager& a_clsPMng, CCmdBase& a_clsBase);
};

#endif //CCOMMANDPRA_HPP_
