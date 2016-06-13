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
	bool	ProcMesgApp(CProtocol& a_clsProto, CProcessManager& a_clsPM);

private:
	bool	Command(CProtocol& a_clsProto, CProcessManager& a_clsPMng);
	
	void	CmdPing(CProtocol& a_clsReq);
	void	CmdAppReconfig(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	
	void	CmdAppStatus(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppStart(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppStop(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppKill(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppInit(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppSuspend(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppResume(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdAppLogLevel(CProtocol& a_clsReq, CProcessManager& a_clsPMng);

	void	CmdStaticApp(CProtocol& a_clsReq, CProcessManager& a_clsPMng);

	void	CmdBatchStart(CProtocol& a_clsReq, CProcessManager& a_clsPMng);

	void	CmdCli(CProtocol& a_clsReq, CProcessManager& a_clsPMng);
	void	CmdCli_DispProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq, CProcessManager& a_clsPM);
	void	CmdCli_StartProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq, CProcessManager& a_clsPM);
	void	CmdCli_StopProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq, CProcessManager& a_clsPM);
	void	CmdCli_InitProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq, CProcessManager& a_clsPM);

	
	void	GetAppStatus(CProcessManager& a_clsPMng, CCmdBase& a_clsBase);
};

#endif //CCOMMANDPRA_HPP_
