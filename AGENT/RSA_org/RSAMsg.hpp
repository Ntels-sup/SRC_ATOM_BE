#ifndef __DEF_RSA_MSG__
#define __DEF_RSA_MSG__ 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "CommonCode.hpp"
#include "CommandFormat.hpp"

#include "CModule.hpp"
#include "CModuleIPC.hpp"

#include "CFileLog.hpp"
#include "CConfig.hpp"

#include "RSAGlobal.hpp"
#include "RSABase.hpp"

#include "CMesgExchSocket.hpp"

#include "EventAPI.hpp"

#include "CCliRsp.hpp"

class RSAMsg
{
	private:
		std::string m_strFull;
		std::string m_strSummary;
		std::string m_strStat;

		CFileLog *m_pclsLog;
		CConfig *m_pclsConfig;

		CMesgExchSocket *m_pclsRootSock;
		CModule::ST_MODULE_OPTIONS* m_pstModuleOption;
        EventAPI *m_pclsEvent;

		std::map< string, DST_INFO * > m_mapMonitor;
		std::map< string, DST_INFO * > m_mapPerf;
		std::map< string, DST_INFO * > m_mapSummary;

	public:
		RSAMsg(
					CFileLog *a_pclsLog, 
					CConfig *a_pclsConfig, 
                    EventAPI *a_pclsEvent,
					CModule::ST_MODULE_OPTIONS* a_pstModuleOption
				);
		~RSAMsg();
		int Initialize();
		int ProcessCmd(CProtocol* a_pclsProto, time_t a_tCur, RESOURCE *a_pstRsc, int a_nGroupCnt);

		int ParsingStopCommand(CProtocol* a_pclsProto, int a_nCmd);
		int ParsingReportCommand(CProtocol* a_pclsProto, time_t a_tCur, int a_nCmd);

		int RegistMonitoringDestination(DST_INFO *a_pstDstInfo);
		int RegistSummaryDestination(DST_INFO *a_pstDstInfo);
		int RegistPerformanceDestination(DST_INFO *a_pstDstInfo);

		int ConnectRoot();
		int SendInitMsgToRoot();
		int SendRequestToRoot(RESOURCE *a_pstRsc, int a_nCmd, time_t a_tCur);
		int RecvMesgFromRoot(RESOURCE *a_pstRsc);
        int SendTrapMsg(RESOURCE *a_pstRsc, const char* a_strMsg);

		int SendMonitoringMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload);
		int SendSummaryMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload);
		int SendPerformanceMsg(DST_INFO *a_pstDstInfo, const char* a_szPayload);

		int MakeFullJson(struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt, bool a_bVNFM);
		int MakeSummaryJson(struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int MakeStatJson(int a_nSessionID);

		int ProcessStatCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int ProcessCLICommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int ProcessNicIpCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int ProcessPeerIpCommand(CProtocol *a_pclsProto, RESOURCE *a_pstRsc, int a_nGroupCnt);

		int SendResponseMsg(time_t a_tCur, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int SendMonitoringInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int SendSummaryInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt);
		int SendPerformanceInfo(time_t a_tCur, struct tm* a_pstCur, RESOURCE *a_pstRsc, int a_nGroupCnt);
};

#endif
