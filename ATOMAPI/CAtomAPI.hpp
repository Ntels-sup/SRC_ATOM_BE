#ifndef __ATOM_API_HPP__
#define __ATOM_API_HPP__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <map>

#include "CAtomDefine.hpp"

//Global
#include "CFileLog.hpp"
#include "CConfig.hpp"

//Command
//#include "CommonCode.hpp"
//#include "CommandFormat.hpp"

//IPC
#include "CLQManager.hpp"
#include "CProtocol.hpp"

//Stat
#include "STAAPI.h"
#include "CStaAnsApi.hpp"
#include "CStaReqApi.hpp"

//Alarm
#include "CTimeUtil.hpp"
#include "CAppControl.hpp"

//Trace
#include "CTraceAPI.hpp"

enum trace_idx
{
	ATOM_DEF_IDX_TRACE_IMSI = 0,
	ATOM_DEF_IDX_TRACE_MSISDN,
	ATOM_DEF_IDX_TRACE_CALLER_IP,
	ATOM_DEF_IDX_TRACE_CALLEE_IP,
	ATOM_DEF_MAX_TRACE_IDX
};

typedef struct _trace_info
{
	std::vector<std::string> vecData;
	int narrTraceReqNo[ATOM_DEF_MAX_TRACE_IDX];
}TRACE_INFO;

class ATOM_API
{
	private :
		static bool m_bRun;

		//Array of CProtocol For Command
		int m_nCmdIdx;
		std::vector<CProtocol*> m_vecCommand;

		char m_szErrorMsg[DEF_MEM_BUF_1024 * DEF_MEM_BUF_1024];
		char m_szPkgName[DEF_MEM_BUF_64];
		char m_szNodeType[DEF_MEM_BUF_64];
		char m_szProcName[DEF_MEM_BUF_64];
		int m_nNodeNo;
		int m_nProcNo;
		CLQManager *m_pclsIPC;		

		char *m_pData;
		CProtocol *m_pclsReqProtocol;
		CAppControl *m_pclsAppCtl;

		CFileLog *m_pclsLog;
		CConfig *m_pclsConfig;

		CTraceAPI *m_pclsTrace;
		TRACE_INFO m_stTraceInfo;

		//Signal Handler for Stop Signaler (SIGTERM)
		static void StopSigHandler(int a_nSigNo);

		//For ALA/VNA 
		void AddProcDateAndDstYn(std::vector<char> &a_vecData);
		int SendNotifyToLocalAPP(int a_nCommand, int a_nDstProc, std::vector<char> &a_vecData);

		//Process Error Msg
		void SetErrorMsg(int a_nErrCode, const char *a_pszFmt, ...);

		//Command
		CProtocol *BlockCommand(int a_nCmd, int a_nTime);
		int ProcessCommand(CProtocol *a_pclsReqProtocol);
		int SendCommand(CProtocol *a_pclsProtocol);
		int SendCommand(CProtocol *a_pclsProtocol, char *a_szProcName, int a_nInstanceID = 0);

	public:

		ATOM_API();
		~ATOM_API();
		int Init(int a_nArgc, char **a_parrArgs, int a_nLogLevel = LV_INFO, int a_nInstanceID = 0, int a_nCmdType = DEF_CMD_TYPE_RECV);

		//Global
		bool IsRun() { return this->m_bRun; }
		CFileLog *GetLog() { return m_pclsLog; }
		CConfig *GetConfig() { return m_pclsConfig; }
		char *GetPkgName() { return m_szPkgName; }
		char *GetNodeType() { return m_szPkgName; }
		char *GetProcName() { return m_szPkgName; }
		int GetNodeNo() { return m_nNodeNo; }
		int GetProcNo() { return m_nProcNo; }


		//IPC Data Get/Set
		int ReadWait(int a_nTime = 0);
		int GetData(char **a_pData);
		int SendData(int a_nIdx = 0);		
		int SendData(char *a_pData, int a_nLength, int a_nIdx = 0);		
		void ReadComplete();
		void FreeReadData();
		int GetQueueIndex(int a_nType, const char *a_szProc);
		int GetQueueCount(int a_nType);

		//IPC Command Get/Set
		int GetCommand();
		int SendResponse(bool a_bRet, const char *a_szResult);
		int SendCommandResult(const char *a_pData, int a_nLength);

		//RSA
		int GetIPAddress(const char *a_szIfName, int a_nVersion, std::string &a_strResult);
		int GetPeerIPAddress(const char *a_szPkgName, const char *a_szNodeType, std::map<std::string, std::string> &a_mapResult);
		int GetNodeIPAddress(const char *a_szPkgName, const char *a_szNodeType, std::map<std::string, std::string> &a_mapResult);

		//Trace
		bool TraceIsOn();
		int SendTraceData(const char *a_szData);
		int SetTraceArgument(int a_nIdx, const char *a_szData);
		bool CheckTrace();

		//Stat
		int SendStat(char *a_szTable, vector<string> &a_vecPri, vector<int> &a_vecData, bool a_bHist = false);

		//ALA, VNA
		int PingSuccess(int a_nPeerNo, const char *a_szMyIP, const char *a_szPeerIP);
		int PingFail(int a_nPeerNo, const char *a_szMyIP, const char *a_szPeerIP);
		int ConnectReport(const char *a_szMyIP, const char *a_szPeerIP, const char *a_szService);
		int DisconnectReport(const char *a_szMyIP, const char *a_szPeerIP, const char *a_szService);
		int Subscribe(const char *a_szCode, int a_nServerityID, char *a_szPkgName);
		int RegisterProvider(int a_nCommandID, int a_nDstProc);
		int RegisterTps();

		int SendTrap(
						const char * a_szCode,
						const char *a_szTarget,
						int 		a_nValue,
						const char *a_szComplement,
						const char *a_szText
					);
		int SendTrap(
						const char * a_szCode,
						const char *a_szTarget,
						double 		a_dValue,
						const char *a_szComplement,
						const char *a_szText
					);
		int SendTrap(
						const char * a_szCode,
						const char *a_szTarget,
						const char *a_szValue,
						const char *a_szComplement,
						const char *a_szText
					);
		int SendTps(const char *a_szTpsTitle, const int a_nValue);

		char *GetErrorMsg() { return m_szErrorMsg; }
};


#endif
