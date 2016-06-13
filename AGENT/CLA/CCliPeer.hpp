#ifndef __CCLIPEER_HPP__
#define __CCLIPEER_HPP__

#include <list>
#include <string>
#include "CProtocol.hpp"
#include "CMesgExchSocketServer.hpp"

using namespace std;

class CCliPeer{
	private :
		time_t m_timestamp;
		string m_strUserId;
		CSocket *m_cSock;
		list<CProtocol*> m_lstRcvMsgQ;
	public :
		CCliPeer(time_t a_timestamp, CSocket *cSock);
		~CCliPeer();
		string GetUserId() { return m_strUserId; };
		bool GetPeerIpPort(string *a_strIp, int *a_nPort) { return m_cSock->GetPeerIpPort(a_strIp, a_nPort); };
		int ReceiveProcess();
		time_t GetTimestamp() { return m_timestamp; };
		unsigned int GetSocketFd() { return (unsigned int)m_cSock->GetSocket(); };
		CProtocol* Receive();
		int SendError(unsigned int a_nSessionId, unsigned int a_nResultCode);
		int Send(CProtocol &cProto);
};

#endif
