#ifndef CMESGEXCHSOCKET_HPP_
#define CMESGEXCHSOCKET_HPP_

#include <string>
#include <vector>

#include "CProtocol.hpp"
#include "CSocket.hpp"


class CMesgExchSocket : public CSocket, public CProtocol
{
public:
	std::string	m_strErrorMsg;

public:
	CMesgExchSocket();
	CMesgExchSocket(int a_nSocket);
	~CMesgExchSocket();

	bool	SendMesg(void);
	bool	SendMesg(CProtocol& a_clsProto);
	int		RecvMesg(CProtocol* a_pcProto = NULL, int a_nWaitTime = 0);

private:
	std::vector<char> m_vecBuff;
	char	m_szTempBuff[CProtocol::MAX_LENGTH];

	CMesgExchSocket(const CMesgExchSocket&) {}	// not support copy instructor 
};

#endif //CMESGEXCHSOCKET_HPP_
