#include "CMesgExchSocketServer.hpp"

CMesgExchSocketServer::CMesgExchSocketServer() : CSocketServer(CSocket::IP_TCP)
{
	m_vecBuff.reserve(CProtocol::MAX_LENGTH);
	memset(m_szTempBuff, 0x00, sizeof(m_szTempBuff));

	return;
}

CMesgExchSocketServer::~CMesgExchSocketServer()
{
	//NodeOP	
	return;
}

bool CMesgExchSocketServer::SendMesg(CSocket *a_pclsClient)
{
	m_strErrorMsg.clear();

	GenStream(m_vecBuff);

	size_t sn = a_pclsClient->Send((char*)&m_vecBuff[0], m_vecBuff.size());
	if (sn != m_vecBuff.size()) {
		m_strErrorMsg = a_pclsClient->m_strErrorMsg;
		return false;
	}

	m_vecBuff.clear();

	return true;
}

int CMesgExchSocketServer::RecvMesg(CSocket *a_pclsClient, CProtocol* a_pcProto, int a_nWaitTime)
{
	m_strErrorMsg.clear();

	int readn = sizeof(CProtocol::ST_HEAD) - m_vecBuff.size();

	// receive header
	if (readn > 0) {
		readn = a_pclsClient->Recv(m_szTempBuff, readn, a_nWaitTime);
		if (readn <= 0) {
			if (a_pclsClient->CSocket::m_enError == CSocket::SOCK_TIMEOUT) {
				return 0;
			}
			m_strErrorMsg = a_pclsClient->m_strErrorMsg;
			m_vecBuff.clear();
			return -1;
		}

		m_vecBuff.insert(m_vecBuff.end(), m_szTempBuff, m_szTempBuff + readn);
		if (m_vecBuff.size() < sizeof(CProtocol::ST_HEAD)) {
			return 0;
		}
	}

	// header의 length를 확인하여 전체 message length를 얻는다. 
	CProtocol::ST_HEAD* h = (CProtocol::ST_HEAD*)&m_vecBuff[0];
	readn = ntohl(h->m_nLength);
	if (readn > CProtocol::MAX_LENGTH) {
		m_strErrorMsg = "invalied body size, too big";
		m_vecBuff.clear();
		return -1;
	}

	// 전체 수신 length에서 이미 수신한 length를 뺀 실제 receive 할 length를 계산
	readn -= m_vecBuff.size();
	if (readn == 0) {						// header로만 구성된 메세지 
		if (this->SetAll((char*)&m_vecBuff[0], m_vecBuff.size()) == false) {
			m_vecBuff.clear();
			m_strErrorMsg = CProtocol::m_strErrorMsg;
			return -1;
		}
		if (a_pcProto != NULL) {
			a_pcProto->SetAll((char*)&m_vecBuff[0], m_vecBuff.size());
		}

		m_vecBuff.clear();
		return 1;
	}
	
	// receive body
	int blen = a_pclsClient->Recv(m_szTempBuff, readn, a_nWaitTime);
	if (blen <= 0) {
		if (a_pclsClient->CSocket::m_enError == CSocket::SOCK_TIMEOUT) {
			return 0;
		}
		m_strErrorMsg = "receive body failed";
		m_vecBuff.clear();
		return -1;
    }

	m_vecBuff.insert(m_vecBuff.end(), m_szTempBuff, m_szTempBuff + blen);

	if (blen < readn) {						// body 가 모두 수신되지 않았다.
		return 0;
	}

	if(this->SetAll((char*)&m_vecBuff[0], m_vecBuff.size()) == false) {
		m_vecBuff.clear();
		m_strErrorMsg = CProtocol::m_strErrorMsg;
		return -1;
	}
	if (a_pcProto != NULL) {
		a_pcProto->SetAll((char*)&m_vecBuff[0], m_vecBuff.size());
	}

	m_vecBuff.clear();
   	
	return 1;
}
