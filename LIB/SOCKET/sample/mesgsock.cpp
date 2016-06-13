#include <string>
#include <cerrno>

#include "CMesgExchSocket.hpp"

using std::string;

int main(void)
{
	CMesgExchSocket sock;

	if (sock.Connect("127.0.0.1", 6000) == false) {
		fprintf(stderr, "connect failed\n");
		return 0;
	}

	// Ping Request
	sock.SetCommand("0000000001");
	sock.SetFlagRequest();
	sock.SetSource(1, 2);
	sock.SetDestination(1, 100);

	if (sock.SendMesg() == false) {
		fprintf(stderr, "message send failed\n");
		fprintf(stderr, "- %s\n", sock.CSocket::m_strErrorMsg.c_str());
		return 0;
	}
	
	//Recv시 수신 메세지가 없으면 wait(기본 5초) 없바로 return 
	//sock.SetNoneBlock();

	// Ping Reponse
	if (sock.RecvMesg(NULL, 5) < 0) {
		fprintf(stderr, "message receive failed, errno=%d\n", errno);
		fprintf(stderr, "Socket, %s\n", sock.CSocket::m_strErrorMsg.c_str());
		fprintf(stderr, "Protocol, %s\n", sock.CProtocol::m_strErrorMsg.c_str());
		return 0;
	}

	// Ping Response 얻는 다른 case 1
	/*
	CProtocol cProto;
	if (sock.RecvMesg(&cProto) == false) {
		fprintf(stderr, "message receive failed, errno=%d\n", errno);
	}
	cProto.GetCommand(strBuf);
	*/

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

	sock.Close();

	return 0;
}
