#ifndef __CMAIN_HPP__
#define __CMAIN_HPP__
#include <list>
#include <map>
#include <vector>

#include "CMesgExchSocketServer.hpp"

#include "MariaDB.hpp"
#include "CCliPeer.hpp"
#include "CCliRoute.hpp"
#include "CSession.hpp"

using namespace std;

class CMain{
	private :
		bool m_blnInitFlag;
		unsigned int m_nTimeOut;
		unsigned int m_nSessionSeq;
		CConfig m_cConfig;
		DB *m_cDb;
        CFileLog *m_cHistLog;
		CMesgExchSocketServer m_cSock;
		CCliRoute m_cCliRoute;
		//int m_nSessionIdCount;
		list<CSession*> m_lstSession;
		list<CCliPeer*> m_lstPeer;
		map<unsigned int,list<CCliPeer*>::iterator> m_mapPeer;

		int UpdateHistErr(CSession *a_cSession, unsigned int a_nResultCode);
		int UpdateHist(CCliPeer *a_cPeer, CSession *a_cSession, CCliRsp *a_cDecRsp);

	public :
		CMain();
		~CMain();
		int Init(DB *a_cDb, char *a_szCfgFile);
		int InsertPeer(CSocket *a_cSock);
		int DelPeer(CCliPeer *cPeer);
		CCliPeer* FindPeerByFd(unsigned int a_unFd);
		CSession* FindSession(unsigned int a_nSessionId);
		int ReceiveCliHandler();
		int SessionTimeOutHandler();
		int ReceiveIpcHandler();
		int RouteHandler(CCliPeer *a_cPeer, CProtocol *cProto);
		int Run();
};

#endif
