#include "CLA.h"
#include "CSession.hpp"

CSession::CSession()
{
	m_nSessionId = 0;
	m_nPeerFd = 0;
	m_peerTimestamp = 0;
	m_sendTime = 0;
	m_cDecReq = NULL;
}

CSession::~CSession()
{
	if(m_cProto != NULL){
		delete m_cProto;
	}

	if(m_cDecReq != NULL){
		delete m_cDecReq;
	}
}

int CSession::Init(unsigned int a_nSessionId, int a_nPeerFd, time_t a_peerTimestamp, time_t a_sendTime)
{
    m_nSessionId = a_nSessionId;
    m_nPeerFd = a_nPeerFd;
    m_peerTimestamp = a_peerTimestamp;
	m_sendTime = a_sendTime;

    return CLA_OK;
}

int CSession::SetData(int a_nDstNo, int a_nDstNodeNo, CProtocol *a_cProto, CCliReq *a_cDecReq)
{
	m_nDstNo = a_nDstNo;
	m_nDstNodeNo = a_nDstNodeNo;
	m_cProto = a_cProto;
	m_cDecReq = a_cDecReq;

	return CLA_OK;
}

