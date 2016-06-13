#include <string>
#include <cerrno>

#include "CMesgExchSocketServer.hpp"

using std::string;

int main(void)
{
	CMesgExchSocketServer sock;

	if (sock.Listen("127.0.0.1", 6000) == false) {
		fprintf(stderr, "connect failed\n");
		return 0;
	}


	CSocket *client = NULL;

	while(true) {
		client = sock.Select(0, 0);
		if (client == NULL) {
			fprintf(stderr, "%s\n", client->m_strErrorMsg.c_str());
			break;
		}

		string strIp;
		int nPort = 0;
		client->GetPeerIpPort(&strIp, &nPort);
		fprintf(stdout, "Connected, peer IP : %s, PORT: %d\n", strIp.c_str(), nPort);

		//Recv시 수신 메세지가 없으면 wait(기본 5초) 없바로 return 
		//sock.SetNoneBlock();

		// Ping Reponse
		if (sock.RecvMesg(client, NULL, 5) < 0) {
			fprintf(stderr, "message receive failed, errno=%d\n", errno);
			fprintf(stderr, "Socket, %s\n", client->m_strErrorMsg.c_str());
			fprintf(stderr, "Protocol, %s\n", sock.CProtocol::m_strErrorMsg.c_str());
			break;
		}

		string strBuf;
		fprintf(stdout, "Version: %d\n", sock.GetVersion());
		sock.GetCommand(strBuf);
		fprintf(stdout, "Command: %s\n", strBuf.c_str());
		fprintf(stdout, "Flag   : %x\n", sock.GetFlag());

		// Flag 확인 다른 방법 case 1;
		if (sock.IsFlagRequest())
			fprintf(stdout, "  - Request\n");
		if (sock.IsFlagResponse())
			fprintf(stdout, "  - Response\n");
		if (sock.IsFlagNotify())
			fprintf(stdout, "  - Notify\n");
		if (sock.IsFlagRetransmit())
			fprintf(stdout, "  - Retrasnmit\n");
		if (sock.IsFlagBroadcast())
			fprintf(stdout, "  - Broadcast\n");
		if (sock.IsFlagError())
			fprintf(stdout, "  - Error\n");

		// Flag 확인 다른 방법 case 2;
		char flag = sock.GetFlag();
		if (flag & CProtocol::FLAG_REQUEST)
			fprintf(stdout, "  - Request\n");
		if (flag & CProtocol::FLAG_RESPONSE)
			fprintf(stdout, "  - Response\n");
		if (flag & CProtocol::FLAG_NOTIFY)
			fprintf(stdout, "  - Notify\n");
		if (flag & CProtocol::FLAG_RETRNS)
			fprintf(stdout, "  - Retrasnmit\n");
		if (flag & CProtocol::FLAG_BROAD)
			fprintf(stdout, "  - Broadcast\n");
		if (flag & CProtocol::FLAG_ERROR)
			fprintf(stdout, "  - Error\n");

		int node, proc;
		sock.GetSource(node, proc);
		fprintf(stdout, "Source Node : %d\n", node);
		fprintf(stdout, "Source Proc : %d\n", proc);
		sock.GetDestination(node, proc);
		fprintf(stdout, "Dest   Node : %d\n", node);
		fprintf(stdout, "Dest   Proc : %d\n", proc);
		fprintf(stdout, "Sequence    : %d\n", sock.GetSequence());
		fprintf(stdout, "Length      : %d\n", sock.GetLength());

	}

	sock.CloseServer();
	sock.ClosePeer(client);

	return 0;
}
