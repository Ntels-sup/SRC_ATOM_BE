#ifndef CMESGEXCHSOCKETSERVER_HPP_
#define CMESGEXCHSOCKETSERVER_HPP_

#include <string>
#include <vector>

#include "CProtocol.hpp"
#include "CSocketServer.hpp"


class CMesgExchSocketServer : public CSocketServer, public CProtocol
{
public:
	std::string m_strErrorMsg;

public:
	CMesgExchSocketServer();
	~CMesgExchSocketServer();

	bool	SendMesg(CSocket *a_pclsClient);
	int		RecvMesg(CSocket *a_pclsClient, CProtocol* a_pcProto = NULL, int a_nWaitTime = 0);

private:
	std::vector<char> m_vecBuff;
	char	m_szTempBuff[CProtocol::MAX_LENGTH];

//	CMesgExchSocketServer(const CMesgExchSocketServer&) {}	// not support copy instructor 
};

#endif //CMESGEXCHSOCKET_HPP_
