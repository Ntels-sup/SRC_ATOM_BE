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

#include "CSocketMultiIO.hpp"
#include "CConfigNm.hpp"
#include "CMesgRoute.hpp"
#include "CommandCode.hpp"
#include "CNodeProcTB.hpp"
#include "CCommandNm.hpp"

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
		if (clsIPC.RecvMesg(CModuleIPC::TH_ROUTING, cProto) > 0) {
			#ifdef NM_DEBUG
			g_pclsLog->DEBUG("Receive Message, from module");
			cProto.Print(g_pclsLog, LV_DEBUG, true);
			#endif
			MessageRouting(cProto, clsSockMulti);
		}

		// Routing Socket Message --------------------------------
		pclsClient = clsSockMulti.Select(0, 1000);
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
			CCommandNm::AlarmNodeStatus(pclsClient->GetSocket(), "DISCONT");
			CNodeProcTB::Instance().Delete(pclsClient->GetSocket());
			clsSockMulti.CloseSocket(pclsClient->GetSocket());

		}

	} //end while
	
	return NULL;
}

void MessageRouting(CProtocol& a_clsProto, CSocketMultiIO& a_clsMultiSock)
{
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	
	int nodeid = -1;
	int procid = -1;
	a_clsProto.GetDestination(nodeid, procid);

	// Socket 연결된 Agent, Server module에 message routing 전송
	int tosock = -1;
	if (clsRoute.FindRouteSock(nodeid, procid, tosock)) {
		CMesgExchSocket* client = a_clsMultiSock.FindSocket(tosock);
		if (client == NULL) {
			g_pclsLog->ERROR("ROUTE, not found routing target");
			return;
		}
		
		#ifdef NM_DEBUG
		g_pclsLog->DEBUG("ROUTE, Send Message, to socket: %d", tosock);
		a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
		#endif
		if (client->SendMesg(a_clsProto) == false) {
			g_pclsLog->ERROR("ROUTE, %s", client->CSocket::m_strErrorMsg.c_str());
		}

		return;
	}

	// NM 내부 thread에 message routing 전송
	CModuleIPC::EN_THREAD tothred = CModuleIPC::TH_NM;
	if (clsRoute.FindRouteThread(nodeid, procid, tothred)) {
		#ifdef NM_DEBUG
		g_pclsLog->DEBUG("ROUTE, Send Message, to module");
		a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
		#endif
		CModuleIPC& clsIpc = CModuleIPC::Instance();
		if (clsIpc.SendMesg(tothred, a_clsProto) == false) {
			g_pclsLog->ERROR("ROUTE, %s", clsIpc.m_strErrorMsg.c_str());
		}
	} else {
		g_pclsLog->ERROR("ROUTE, routing failed, unknwon destination, check socket connection");
		a_clsProto.Print(g_pclsLog, LV_ERROR);
		return;
	}
	
	return;
}
