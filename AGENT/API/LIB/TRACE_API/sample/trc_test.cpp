#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>

#include "CTraceAPI.hpp"
#include "CFileLog.hpp"
#include "CConfig.hpp"
#include "CProtocol.hpp"

CFileLog *g_pcLog = NULL;

using namespace std;

int main()
{
	int ret = 0;
	string test;
	string senddata;

	g_pcLog = new CFileLog(&ret);
	if(ret < 0)
	{
		if(g_pcLog)
			delete g_pcLog;

		return 0;
	}

	CProtocol *p_clsReqProtocol = NULL;
	CProtocol *p_clsResProtocol = NULL;
	// initialize Trace API
	CTraceAPI *p_clsTraceApi = NULL;

	int nTraceCount = 0, nRunMode = 0, nSearchMode = 0;  // Trace Count, 
														 // RunMode : normal(1)/debug(0), 
														 // SearchMode : IMSI(1), MSISDN(2), Calling IP(3), Called IP(4)
	char strKeyword[1024];		// Search 문자열

	memset(strKeyword, 0x00, 1024);
	
	// Trace 하는 대상 프로세스 등록 
	// SetTraceProcess(string a_strPkgName, string a_strNode_type, string a_strProc_name, int a_nProcNo, string a_cmd, int a_node_no, string a_node_name) //패키지 이름, 노드 타입, 프로세스 이름, 프로세스 번호, 내부 trace 실행 명령, 노드 번호, 노드명
	test = p_clsTraceApi->SetTraceProcess("vOFCS", "AP", "Script_NA", 53, "TRC_DIA -i 11111111", 12, "vOFCS");

	p_clsTraceApi->SendMessage(test);

	while(1)
	{
		// Request 메시지 추출 Trace Off는 1, Trace ON은 1
		p_clsTraceApi->GetTraceRequest(p_clsReqProtocol, p_clsResProtocol);	

		if(p_clsTraceApi->IsOn() == true)		//Trace 갯수가 존재시
		{
			sprintf(strKeyword, "%s", p_clsTraceApi->GetKeyWord(1));  // 입력된 문자열
			printf("GetCount()[%d], GetRunMode()[%d], GetSearchMode[%d], strKeyword[%s]\n", nTraceCount, nRunMode, nSearchMode, strKeyword);

			// CheckIMSI
			if(p_clsTraceApi->CheckIMSI("1234561231") == true)
			{
				printf("IMSI find true");
				// 검색 결과 전송 - SendTraceData(등록 Trace 번호, 검색 성공여부(true/false, 전송할 문자열, 실패시 공백)
				senddata = p_clsTraceApi->SendTraceData(1, true, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
				p_clsTraceApi->SendMessage(senddata);
			}
			else
			{
				printf("IMSI find false");
			}

			if(p_clsTraceApi->CheckMSISDN("8210101001010") == true)
			{
				printf("MSISDN find true");
			// 검색 결과 전송 - SendTraceData(등록 Trace 번호, 검색 성공여부(true/false, 전송할 문자열, 실패시 공백)
				senddata = p_clsTraceApi->SendTraceData(1, true, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
				p_clsTraceApi->SendMessage(senddata);
			}
			else
			{
				printf("MSISDN find false");
			}
			if(p_clsTraceApi->CheckCallingIP("123.111.100.100") == true)
			{
				printf("MSISDN find true");
			// 검색 결과 전송 - SendTraceData(등록 Trace 번호, 검색 성공여부(true/false, 전송할 문자열, 실패시 공백)
				senddata = p_clsTraceApi->SendTraceData(1, true, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
				p_clsTraceApi->SendMessage(senddata);
			}
			else
			{
				printf("MSISDN find false");
			}
			if(p_clsTraceApi->CheckCalledIP("192,168.100.100") == true)
			{
				printf("MSISDN find true");
			// 검색 결과 전송 - SendTraceData(등록 Trace 번호, 검색 성공여부(true/false, 전송할 문자열, 실패시 공백)
				senddata = p_clsTraceApi->SendTraceData(1, true, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
				p_clsTraceApi->SendMessage(senddata);
			}
			else
			{
				printf("MSISDN find false");
			}
		}

	}

	return 0;
}
