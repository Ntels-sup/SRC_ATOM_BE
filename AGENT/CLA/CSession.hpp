#ifndef __CSESSION_HPP__
#define __CSESSION_HPP__
#include <time.h>
#include "CCliReq.hpp"
#include "CProtocol.hpp"

class CSession{
    private :
        unsigned int m_nSessionId;
        unsigned int m_nClcSessionId;
        unsigned int m_nPeerFd;
		time_t m_sendTime;
        time_t m_peerTimestamp;
		int m_nDstNo;
		int m_nDstNodeNo;
		CCliReq *m_cDecReq;
		CProtocol *m_cProto;

    public :
        CSession();
        ~CSession();
		unsigned int GetSessionId() { return m_nSessionId; };
		unsigned int GetClcSessionId() { return m_nClcSessionId; };
		void SetClcSessionId(unsigned int a_nClcSessionId) { m_nClcSessionId = a_nClcSessionId; };
		unsigned int GetPeerFd() { return m_nPeerFd; };
		time_t GetPeerTimestamp() { return m_peerTimestamp; };
		time_t GetSendTime() { return m_sendTime; };
        int Init(unsigned int a_nSessionId, int a_nPeerFd, time_t a_peerTimestamp, time_t a_sendTime);
		int SetData(int a_nDstNo, int a_nDstNodeNo, CProtocol *a_cProto, CCliReq *a_cDecReq);
		CCliReq* GetDecReqP() { return m_cDecReq; };
};

#endif
