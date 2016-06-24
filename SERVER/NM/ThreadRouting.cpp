/* vim:ts=4:sw=4
 */
/**
 * \file	ThreadClient.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include <iostream>
#include <vector>

#include "CFileLog.hpp"
#include "CProtocol.hpp"
#include "CommonCode.hpp"

#include "CSocketMultiIO.hpp"
#include "CConfigNm.hpp"
#include "CMesgRoute.hpp"
#include "CommandCode.hpp"
#include "CNodeProcTB.hpp"
#include "CCommandNm.hpp"
#include "CModuleIPC.hpp"

using std::vector;
using std::list;


// global 변수
extern CFileLog* g_pclsLog;

// function prototype
void MessageRouting(CProtocol& a_clsProto, CSocketMultiIO& a_clsSockMulti);


void* ThreadRouting(void* a_pArg)
{
	CModuleIPC& clsIPC = CModuleIPC::Instance();
	CMesgRoute& clsRoute = CMesgRoute::Instance();

	CSocketMultiIO clsSockMulti;
	CMesgExchSocket* pclsClient = NULL;
	CProtocol cProto;
	
	int ret = -1;
	list<int> lstNewRoute;
	list<int>::iterator iter;

	g_pclsLog->INFO("Routing Thread running");

	while (true) {

		// 라우팅할 새로운 Socket 확인
		clsRoute.GetNewRouteSock(lstNewRoute);
		for (iter = lstNewRoute.begin(); iter != lstNewRoute.end(); ++iter) {
			g_pclsLog->DEBUG("new routing socket: %d", *iter);
			clsSockMulti.AddSocket(*iter);
		}

		// Routing thread message --------------------------------
		if (clsIPC.RecvMesg(PROCID_ATOM_NM_ROUTE, cProto, -1) > 0) {
			#ifdef NM_DEBUG
			g_pclsLog->DEBUG("Receive Message, from module");
			cProto.Print(g_pclsLog, LV_DEBUG, true);
			#endif
			MessageRouting(cProto, clsSockMulti);
		}

		// Routing Socket Message --------------------------------
		pclsClient = clsSockMulti.Select(0, 10);
		if (pclsClient == NULL) {
			continue;
		}
		
		ret = pclsClient->RecvMesg(&cProto);
		if (ret == 1) {
			#ifdef NM_DEBUG
			g_pclsLog->DEBUG("Receive Message, from socket: %d", pclsClient->GetSocket());
			cProto.Print(g_pclsLog, LV_DEBUG, true);
			#endif
			// 메세지 라우팅
			MessageRouting(cProto, clsSockMulti);
			
		} else if (ret < 0) {
			g_pclsLog->ERROR("ROUTE, client close socket: %d", pclsClient->GetSocket());
			clsRoute.DelRouteSock(pclsClient->GetSocket());

			CNodeProcTB::ST_INFO stInfo;
			if (CNodeProcTB::Instance().GetNode(pclsClient->GetSocket(), stInfo)) {
				CCommandNm::AlarmNodeStatus(stInfo, "DISCONT");
			} else {
				g_pclsLog->ERROR("ROUTE, not found node info");
			}

			CNodeProcTB::Instance().Delete(pclsClient->GetSocket());
			clsSockMulti.CloseSocket(pclsClient->GetSocket());

		} else {
			g_pclsLog->ERROR("ROUTE, client close ZERO----------------------- ");
		}

	} //end while
	
	return NULL;
}

void MessageRouting(CProtocol& a_clsProto, CSocketMultiIO& a_clsMultiSock)
{
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	
	int nNodeNo = -1;
	int nProcNo = -1;
	a_clsProto.GetDestination(nNodeNo, nProcNo);

	// Socket 연결된 Agent, Server module에 message routing 전송
	int nToRoute = -1;
	bool bIsSock = false;
	if (clsRoute.Lookup(nNodeNo, nProcNo, nToRoute, bIsSock) == false) {
		g_pclsLog->ERROR("ROUTE, routing failed, unknwon destination, check socket connection");
		a_clsProto.Print(g_pclsLog, LV_ERROR);
		return;
	}

	if (bIsSock) {
		CMesgExchSocket* client = a_clsMultiSock.FindSocket(nToRoute);
		if (client == NULL) {
			g_pclsLog->ERROR("ROUTE, not found routing target");
			return;
		}
		
		#ifdef NM_DEBUG
		g_pclsLog->DEBUG("ROUTE, Send Message, to socket: %d", nToRoute);
		a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
		#endif
		if (client->SendMesg(a_clsProto) == false) {
			g_pclsLog->ERROR("ROUTE, %s", client->CSocket::m_strErrorMsg.c_str());
		}

	} else {
		// NM 내부 thread에 message routing 전송
		#ifdef NM_DEBUG
		g_pclsLog->DEBUG("ROUTE, Send Message, to module");
		a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
		#endif
		CModuleIPC& clsIpc = CModuleIPC::Instance();
		if (clsIpc.SendMesg(a_clsProto) == false) {
			g_pclsLog->ERROR("ROUTE, %s", clsIpc.m_strErrorMsg.c_str());
		}
	}

	return;
}
