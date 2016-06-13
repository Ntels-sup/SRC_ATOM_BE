#include <unistd.h>
//#include "CLC.h"
//#include "CGlobal.hpp"
#include "CommandFormat.hpp"
//#include "CTimerHandler.hpp"
#include "CNMInterface.hpp"

#define NMINT_LOG(...){\
	snprintf(errBuffer,MAX_BUFFER_LEN,__VA_ARGS__);\
	if(m_cLog != NULL){\
		m_cLog->ERROR(errBuffer);\
	}\
}

CNMInterface::CNMInterface()
: TimerEvent(TIMER_TYPE_100M)
{
	m_eNmState = SOCK_STATE_CLOSED;
	SockTimeOutNode = NULL;

	m_cLog = NULL;

	m_nPort = 0;
	m_nConnRetryTime = 3;
}

CNMInterface::~CNMInterface()
{
	while(lstRcvMsgQ.size()){
		delete lstRcvMsgQ.front();
		lstRcvMsgQ.pop_front();
	}
}

void CNMInterface::SetLog(CFileLog *a_cLog)
{
	m_cLog = a_cLog;
}

int CNMInterface::SetTransportAddr(const char *a_strConnIp, unsigned int a_nPort, unsigned int a_nConnRetryTime)
{
	m_strConnIp = a_strConnIp;
	m_nPort = a_nPort;
	m_nConnRetryTime = a_nConnRetryTime * 10;

	if(m_eNmState != SOCK_STATE_CLOSED){
		Close(true);
	}

	//ConnReq();

	return RESULT_OK;
}

#if 0
CMesgExchSocket *CNMInterface::GetExchSocket()
{
    return &m_cSock;
}
#endif


int CNMInterface::TryConnect(string &a_strIp, int a_nPort)
{
	if(m_cSock.IsConnected() == false){
		if(m_cSock.Connect(a_strIp.c_str(), a_nPort) == false){
			if(m_cSock.CSocket::m_enError== m_cSock.CSocket::SOCK_TIMEOUT){
				m_eNmState = SOCK_STATE_CONN_WAIT;
				return RESULT_CONN_WAIT;
			}

			NMINT_LOG("Socket connect failed(err=%d:%s)(errno=%d(%s))\n",
					m_cSock.m_enError, m_cSock.CSocket::m_strErrorMsg.c_str(), errno, strerror(errno));
			return RESULT_CONN_FAILED;
		}
	}

	m_cSock.SetNonBlock();

	return RESULT_OK;
}

int CNMInterface::Close(bool a_blnTimerClearFlag)
{
    m_cSock.Close();

    m_eNmState = SOCK_STATE_CLOSED;
    if(a_blnTimerClearFlag == true){
        if(SockTimeOutNode != NULL){
            Stop(SockTimeOutNode);
            SockTimeOutNode = NULL;
        }
    }

    SockTimeOutNode = Start(m_nConnRetryTime, TMR_EVNT_CONN_TIMEOUT, (VOID*)this);

    return RESULT_OK;
}

CProtocol *CNMInterface::Receive()
{
    CProtocol *cProto = NULL;

    if(lstRcvMsgQ.size() == 0){
        return NULL;
    }

    cProto = lstRcvMsgQ.front();

    lstRcvMsgQ.pop_front();

    return cProto;
}

int CNMInterface::Send(CProtocol &cProto)
{
	if(m_eNmState == SOCK_STATE_CLOSED){
        NMINT_LOG("closed state\n");
		return RESULT_CLOSED_STATE;
	}
    if(m_cSock.SendMesg(cProto) == false){
        NMINT_LOG("message send failed\n");
        NMINT_LOG("- %s\n", m_cSock.CSocket::m_strErrorMsg.c_str());

        Close(true);

		return RESULT_SEND_FAILED;
    }

    return RESULT_OK;
}

int CNMInterface::Handler()
{
	int nRet = 0;
	int nCmdCode = 0;
	string strCmdCode;
	CProtocol cRcvMsg;
	CProtocol *cProto = NULL;

	TimerEvent::Calc();

	TimerEvent::Handler();

	if(m_eNmState == SOCK_STATE_CLOSED){
		if(SockTimeOutNode == NULL){
			ConnReq();
		}
		return RESULT_OK;
	}

	if(m_eNmState == SOCK_STATE_CONN_WAIT){
		ConnReq();
		return RESULT_OK;
	}

	nRet = m_cSock.RecvMesg(&cRcvMsg, -1);
	if(nRet < 0){
		NMINT_LOG("Receive message failed(nRet=%d)\n",nRet);
		Close(true);
		return RESULT_RECV_FAILED;
	}

	if(nRet == 0){
		return RESULT_OK;
	}

	if(m_eNmState == SOCK_STATE_REG_WAIT){
		if(m_cRegProto.GetCommand().compare(cRcvMsg.GetCommand()) != 0){
			NMINT_LOG("Invalid command code(%s)\n",cRcvMsg.GetCommand().c_str());
			Close(true);
			return RESULT_RECV_FAILED;
		}
		nRet = RegRsp(cRcvMsg);
		if(nRet == RESULT_OK){
			m_eNmState = SOCK_STATE_OPEN;
			/* close timer */
			Stop(SockTimeOutNode);
			SockTimeOutNode = NULL;

			return RESULT_OK;
		}
		else {
			NMINT_LOG("Registration failed (nRet=%d)\n",nRet);
			Close(true);

			return RESULT_REG_FAILED;
		}
	}

	m_cSock.GetCommand(strCmdCode);

	nCmdCode = GetCmdCode(strCmdCode);

	cProto = new CProtocol(cRcvMsg);

	lstRcvMsgQ.push_back(cProto);
#if 0
	switch(nCmdCode){
		case CMD_CLI_COMMAND:
			{
				CProtocol *cProto = NULL;

				cProto = new CProtocol(cRcvMsg);

				lstRcvMsgQ.push_back(cProto);

			}
			break;
		default:
			{
				NMINT_LOG("Invalid CmdCode(code=%s)\n", strCmdCode.c_str());
			}
			break;
	};
#endif

	return RESULT_OK;
}

int CNMInterface::RegRspFunc(CProtocol &cProto)
{
	return RESULT_OK;
}

int CNMInterface::RegRsp(CProtocol &cProto)
{
	int nRet = 0;

	nRet = RegRspFunc(cProto);

	return nRet;
}

int CNMInterface::RegReqFunc(CProtocol &cProto)
{
	return RESULT_OPEN;
}

int CNMInterface::RegReq()
{
	int nRet = 0;

	nRet = RegReqFunc(m_cRegProto);
	if(nRet != RESULT_OK){
		if(nRet == RESULT_OPEN){
			return nRet;
		}
		return RESULT_CLOSED_STATE;
	}

	/* send message */
	nRet = Send(m_cRegProto);
	if(nRet != RESULT_OK){
        NMINT_LOG("Reg message send failed\n");
		return RESULT_CLOSED_STATE;
	}

	m_eNmState = SOCK_STATE_REG_WAIT;

	if(SockTimeOutNode != NULL){
		Stop(SockTimeOutNode);
	}

	SockTimeOutNode = Start(30, TMR_EVNT_REG_WAIT, (VOID*)this);

	return RESULT_OK;
}


int CNMInterface::ConnReq()
{
	int nRet = 0;
	int nEvent = 0;

	nRet = TryConnect(m_strConnIp, m_nPort);
	if(nRet != RESULT_OK){
		if(nRet == RESULT_CONN_WAIT){
			m_eNmState = SOCK_STATE_CONN_WAIT;
			if(SockTimeOutNode != NULL){
				nEvent = SockTimeOutNode->GetEvent();
				if(nEvent == TMR_EVNT_CONN_WAIT){
					return RESULT_OK;
				}
				else {
					Stop(SockTimeOutNode);
					SockTimeOutNode = NULL;
				}
			}
			SockTimeOutNode = Start(m_nConnRetryTime, TMR_EVNT_CONN_WAIT, (VOID*)this);
			return RESULT_OK;
		}
		NMINT_LOG("Socket connect failed(nRet=%d\n",nRet);
		Close(true);
		return RESULT_CONN_FAILED;
	}

	m_eNmState = SOCK_STATE_REG_WAIT;

	nRet = RegReq();
	if((nRet != RESULT_OK) && 
			(nRet != RESULT_OPEN)){
		NMINT_LOG("Regstration failed(nRet=%d\n)\n",nRet);
		Close(true);
		return nRet;
	}
	else if(nRet == RESULT_OK){
		return RESULT_OK;
	}

	m_eNmState = SOCK_STATE_OPEN;
	NMINT_LOG("Connecton open\n");

	return RESULT_OK;
}

RT_RESULT CNMInterface::TmrEvtFunc(UINT event, VOID *data)
{
    switch(event){
        case TMR_EVNT_CONN_TIMEOUT:
            {
                int nRet = 0;

                ClearTimerNode();
                nRet = ConnReq();
                if(nRet != RESULT_OK){
                    NMINT_LOG("Connonection reqeust failed\n");
                }
            }
            break;
        case TMR_EVNT_CONN_WAIT:
            {
                NMINT_LOG("Socket close(state=CONN_WAIT)\n");
                ClearTimerNode();
                Close(true);
            }
            break;

		case TMR_EVNT_REG_WAIT:
			{
                NMINT_LOG("Socket close(state=REG_WAIT)\n");
                ClearTimerNode();
                Close(true);
			}
			break;
    };

    return RC_OK;
}
