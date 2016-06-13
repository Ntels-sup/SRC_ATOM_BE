#ifndef __CNMINTERFACE_HPP__
#define __CNMINTERFACE_HPP__

#include "TimerEvent.hpp"

#include "CFileLog.hpp"
#include "CProtocol.hpp"
#include "CMesgExchSocket.hpp"

#define MAX_BUFFER_LEN 2096
#define TMR_EVNT_CONN_TIMEOUT 1
#define TMR_EVNT_CONN_WAIT 2
#define TMR_EVNT_REG_WAIT 3

class CNMInterface : public TimerEvent{
	private :
		CMesgExchSocket m_cSock;
		TimerNode *SockTimeOutNode;
		list<CProtocol*> lstRcvMsgQ;
		string m_strConnIp;
		int m_nPort;
		int m_nConnRetryTime;
		int m_eNmState;
		CProtocol m_cRegProto;
		CFileLog *m_cLog;

		static int GetCmdCode(std::string &a_strCmdCode){
			if(a_strCmdCode.size() < 10){
				return 0;
			}

			return atoi(&a_strCmdCode.c_str()[4]);
		};

		RT_RESULT TmrEvtFunc(UINT event, VOID *data);
		CMesgExchSocket *GetExchSocket();
		void ClearTimerNode() { SockTimeOutNode = NULL; };
		int Close(bool a_blnTimerClearFlag = true);
		int ConnReq();
		int TryConnect(string &a_strIp, int a_nPort);

	public :
		static const int SOCK_STATE_CLOSED = 1;
		static const int SOCK_STATE_CONN_WAIT = 2;
		static const int SOCK_STATE_REG_WAIT = 3;
		static const int SOCK_STATE_OPEN = 4; 

		static const int RESULT_OK = 1;
		static const int RESULT_CLOSED_STATE = 2;
		static const int RESULT_CONN_WAIT = 3;
		static const int RESULT_CONN_FAILED = 4;
		static const int RESULT_SEND_FAILED = 5;
		static const int RESULT_RECV_FAILED = 6;
		static const int RESULT_OPEN = 7;
		static const int RESULT_REG_FAILED = 8;

		char errBuffer[MAX_BUFFER_LEN];

		CNMInterface();
		~CNMInterface();
		int RegReq();
		int RegRsp(CProtocol &cProto);
		virtual int RegReqFunc(CProtocol &cProto);
		virtual int RegRspFunc(CProtocol &cProto);
		void SetLog(CFileLog *a_cLog);
		int SetTransportAddr(const char *a_strConnIp, unsigned int a_nPort, unsigned int a_nConnRetryTime);
		CProtocol *Receive();
		int Send(CProtocol &cProto);
		int Handler();
};

#endif
