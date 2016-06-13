#include <cstdio>
#include <cstdlib>
#include <string>


#include "CSocketServer.hpp"

using std::string;


int main(void)
{
	CSocketServer server(CSocket::IP_TCP);

	if (server.Listen("127.0.0.1", 2222) == false) {
		fprintf(stderr, "%s\n", server.m_strErrorMsg.c_str());
		return 0;
	}

/*
	CSocket* client = server.Accept();
	if (client == NULL) {
		fprintf(stderr, "%s\n", server.m_strErrorMsg.c_str());
		return 0;
	}
*/

	CSocket* client = NULL;

	while (true) {
		client = server.Select(0, 0);
		if (client == NULL) {
			fprintf(stderr, "error %s\n", server.m_strErrorMsg.c_str());
			return 0;
		}


		string strIp;
		int nPort = 0;
		client->GetPeerIpPort(&strIp, &nPort);
		fprintf(stdout, "Connected, peer IP: %s PORT: %d\n", strIp.c_str(), nPort);


		char buff[80] = {0x00,};
		if (client->Recv(buff, 20) != 20) {
			fprintf(stderr, "%s\n", server.m_strErrorMsg.c_str());
			return 0;
		}
		
		fprintf(stdout, "%s\n", buff);

		if (client->Send(buff, 20) != 20) {
			fprintf(stderr, "%s\n", server.m_strErrorMsg.c_str());
			return 0;
		}
	}

	server.CloseServer();
	server.ClosePeer(client);

	return 0;
}
