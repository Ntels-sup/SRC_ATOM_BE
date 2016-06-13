#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "CSocket.hpp"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage) %s ip port\n", argv[0]);
		return 1;
	}

	CSocket sock(CSocket::IP_TCP);

	if (sock.Connect(argv[1], atoi(argv[2])) == false) {
		fprintf(stderr, "connect failed, %s\n", sock.m_strErrorMsg.c_str());
		return 1;
	}

	int sendn = sock.Send("http://www.naver.com", 20);
	if (sendn != 20) {
		fprintf(stderr, "send failed, %s\n", sock.m_strErrorMsg.c_str());
		return 1;
	}

	char buff[21] = {0x00,};
	int readn = sock.Recv(buff, 20);
	if (readn != 20) {
		fprintf(stderr, "recv failed, %d, %s\n", readn, sock.m_strErrorMsg.c_str());
		return 1;
	}
	fprintf(stdout, "%s\n", buff);

	sock.Close();

	return 0;
}
