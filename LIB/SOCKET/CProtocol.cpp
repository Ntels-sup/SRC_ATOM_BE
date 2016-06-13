/* vim:ts=4:sw=4
 */
/**
 * \file	CRawMessage.hpp
 * \brief	실시간 message를 목적지 주소에 따른 경로 배분, 전달
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <arpa/inet.h>

#include "CProtocol.hpp"

using std::string;
using std::vector;
using std::pair;


CProtocol::CProtocol()
{
	m_stHead.m_cVer = 0x01;
	memset(m_stHead.m_szCmd, 0x00, sizeof(m_stHead.m_szCmd));
	m_stHead.m_cFlag = 0x00;
	m_stHead.m_nSrcNode = 0;
	m_stHead.m_nSrcProc = 0;
	m_stHead.m_nDstNode = 0;
	m_stHead.m_nDstProc = 0;
	m_stHead.m_nSeq = 0;
	m_stHead.m_nLength = 0;

	srand(time(NULL));
	m_nSequence = rand();
	
	return;
}

CProtocol::CProtocol(const CProtocol& a_clsProto)
{
	m_stHead = a_clsProto.m_stHead;
	m_vecBody = a_clsProto.m_vecBody;
	m_nSequence = a_clsProto.m_nSequence;
	return;
}

void CProtocol::SetCommand(const char* a_szCmd)
{
	strncpy(m_stHead.m_szCmd, a_szCmd, sizeof(m_stHead.m_szCmd));
	return;
}

void CProtocol::SetCommand(int a_nCmd)
{
	char buff[sizeof(m_stHead.m_szCmd) + 1];
	snprintf(buff, sizeof(buff), "%010d", a_nCmd);

	strncpy(m_stHead.m_szCmd, buff, sizeof(m_stHead.m_szCmd));
	return;
}
 
void CProtocol::SetFlagRequest(void)
{
	m_stHead.m_cFlag |= FLAG_REQUEST;
	return;
}

void CProtocol::SetFlagResponse(void)
{
	m_stHead.m_cFlag |= FLAG_RESPONSE;	
	return;
}

void CProtocol::SetFlagNotify(void)
{
	m_stHead.m_cFlag |= FLAG_NOTIFY;	
	return;
}

void CProtocol::SetFlagRetransmit(void)
{
	m_stHead.m_cFlag |= FLAG_RETRNS;	
	return;
}

void CProtocol::SetFlagBroadcast(void)
{
	m_stHead.m_cFlag |= FLAG_BROAD;
	return;
}

void CProtocol::SetFlagError(void)
{
	m_stHead.m_cFlag |= FLAG_ERROR;	
	return;
}
	
void CProtocol::SetSource(int a_nNodeNo, int a_nProcNo)
{
	m_stHead.m_nSrcNode = a_nNodeNo;
	m_stHead.m_nSrcProc = a_nProcNo;
	return;
}

void CProtocol::SetSource(pair<int, int> a_parSrc)
{
	m_stHead.m_nSrcNode = a_parSrc.first;
	m_stHead.m_nSrcProc = a_parSrc.second;
	return;
}

void CProtocol::SetDestination(int a_nNodeNo, int a_nProcNo)
{
	m_stHead.m_nDstNode = a_nNodeNo;
	m_stHead.m_nDstProc = a_nProcNo;
	return;
}

void CProtocol::SetDestination(std::pair<int, int> a_parDst)
{
	m_stHead.m_nDstNode = a_parDst.first;
	m_stHead.m_nDstProc = a_parDst.second;
	return;	
}

int CProtocol::SetSequence(int a_nSequence)
{
	if (a_nSequence < 0) {
		m_stHead.m_nSeq = m_nSequence++;
	} else {
		m_stHead.m_nSeq = a_nSequence;
	}
	
	return m_stHead.m_nSeq;
}

void CProtocol::SetPayload(const vector<char>& a_vecPay)
{
	m_stHead.m_nLength = sizeof(ST_HEAD) + a_vecPay.size();
	m_vecBody = a_vecPay;
	return;
}

void CProtocol::SetPayload(const string& a_strPay)
{
	m_stHead.m_nLength = sizeof(ST_HEAD) + a_strPay.size();
	m_vecBody.clear();
	m_vecBody.insert(m_vecBody.end(), a_strPay.begin(), a_strPay.end()); 
	return;
}

void CProtocol::SetPayload(const char* a_szPay, int a_nLen)
{
	m_stHead.m_nLength = sizeof(ST_HEAD) + a_nLen;
	m_vecBody.clear();
	if (a_szPay != NULL) {
		m_vecBody.insert(m_vecBody.end(), a_szPay, a_szPay + a_nLen);
	}
	return;	
}

void CProtocol::SetResponse(const CProtocol& a_clsProto)
{
	m_stHead.m_cVer		= a_clsProto.m_stHead.m_cVer;
	memcpy(m_stHead.m_szCmd, a_clsProto.m_stHead.m_szCmd, sizeof(m_stHead.m_szCmd));
	m_stHead.m_nSeq		= a_clsProto.m_stHead.m_nSeq;
	m_stHead.m_cFlag	= FLAG_RESPONSE;
	m_stHead.m_nSrcNode	= a_clsProto.m_stHead.m_nDstNode;
	m_stHead.m_nSrcProc	= a_clsProto.m_stHead.m_nDstProc;
	m_stHead.m_nDstNode	= a_clsProto.m_stHead.m_nSrcNode;
	m_stHead.m_nDstProc	= a_clsProto.m_stHead.m_nSrcProc;
	m_stHead.m_nSeq		= a_clsProto.m_stHead.m_nSeq;
	m_stHead.m_nLength	= sizeof(ST_HEAD);

	return;
}

bool CProtocol::SetAll(const char* a_pMsg, int a_nMsgSize)
{
	if (a_nMsgSize < (signed)sizeof(ST_HEAD)) {
		m_strErrorMsg = "invalied header size";
		return false;
	}

	// member 변수 초기화
	Clear();
	memcpy(&m_stHead, a_pMsg, sizeof(m_stHead));
	
	// header 값 유효성 확인
	if (m_stHead.m_cVer != 0x01) {
		m_strErrorMsg = "invalied header version number";
		Clear();
		return false;
	}

	int flagcnt = 0;
	if (m_stHead.m_cFlag & FLAG_REQUEST) flagcnt++;
	if (m_stHead.m_cFlag & FLAG_RESPONSE) flagcnt++;
	if (m_stHead.m_cFlag & FLAG_NOTIFY) flagcnt++;
	if (flagcnt > 1) {
		m_strErrorMsg = "invalied set header flag";
		Clear();
		return false;
	}
	
	m_stHead.m_nSrcNode = ntohl(m_stHead.m_nSrcNode);
	m_stHead.m_nSrcProc = ntohl(m_stHead.m_nSrcProc);
	m_stHead.m_nDstNode = ntohl(m_stHead.m_nDstNode);
	m_stHead.m_nDstProc = ntohl(m_stHead.m_nDstProc);
	m_stHead.m_nSeq = ntohl(m_stHead.m_nSeq);
	m_stHead.m_nLength = ntohl(m_stHead.m_nLength);
	
	if (m_stHead.m_nLength != a_nMsgSize) {
		m_strErrorMsg = "invalied set length";
		Clear();
		return false;
	}

	const char *pPayload = a_pMsg + sizeof(ST_HEAD);
	// body set, raw message 저장
	if (a_nMsgSize > (signed)sizeof(ST_HEAD)) {
		m_vecBody.clear();
		m_vecBody.insert(m_vecBody.end(), 
						pPayload, pPayload + (a_nMsgSize - sizeof(ST_HEAD)));
	}

	return true;
}

char CProtocol::GetVersion(void)
{
	return m_stHead.m_cVer;
}

void CProtocol::GetCommand(string& a_strCmd)
{
	a_strCmd.assign(m_stHead.m_szCmd, sizeof(m_stHead.m_szCmd));
	return;
}

string CProtocol::GetCommand(void)
{
	string strCmd(m_stHead.m_szCmd, sizeof(m_stHead.m_szCmd));
	return strCmd;
}

void CProtocol::GetSource(int& a_nNodeNo, int& a_nProcNo)
{
	a_nNodeNo = m_stHead.m_nSrcNode;
	a_nProcNo = m_stHead.m_nSrcProc;
	return;
}

pair<int, int> CProtocol::GetSource(void)
{
	pair<int, int> parSrc;
	parSrc.first = m_stHead.m_nSrcNode;
	parSrc.second = m_stHead.m_nSrcProc;

	return parSrc;
}

void CProtocol::GetDestination(int& a_nNodeNo, int& a_nProcNo)
{
	a_nNodeNo = m_stHead.m_nDstNode;
	a_nProcNo = m_stHead.m_nDstProc;
	return;
}

pair<int, int> CProtocol::GetDestination(void)
{
	pair<int, int> parDst;
	parDst.first = m_stHead.m_nDstNode;
	parDst.second = m_stHead.m_nDstProc;

	return parDst;
}

char CProtocol::GetFlag(void)
{
	return m_stHead.m_cFlag;
}

int CProtocol::GetPayload(vector<char>& a_vecPayload)
{
	a_vecPayload = m_vecBody;
	return a_vecPayload.size();
}

string CProtocol::GetPayload(void)
{
	string strBody(m_vecBody.begin(), m_vecBody.end());
	return strBody;
}

void CProtocol::GenStream(vector<char>& a_vecM)
{
	ST_HEAD h = m_stHead;
	h.m_nSrcNode = htonl(h.m_nSrcNode);
	h.m_nSrcProc = htonl(h.m_nSrcProc);
	h.m_nDstNode = htonl(h.m_nDstNode);
	h.m_nDstProc = htonl(h.m_nDstProc);
	h.m_nSeq	 = htonl(h.m_nSeq);
	if (h.m_nLength == 0) {
		h.m_nLength = sizeof(m_stHead) + m_vecBody.size();
	}
	h.m_nLength  = htonl(h.m_nLength);
	
	a_vecM.clear();
	a_vecM.insert(a_vecM.end(), (char*)&h, (char*)&h + sizeof(h));
	a_vecM.insert(a_vecM.end(), m_vecBody.begin(), m_vecBody.end());

	return;
}

void CProtocol::Print(CFileLog* a_pclsLog, int a_nLevel, bool a_bPayPrint)
{
	string strTemp;
	
	a_pclsLog->LogMsg(a_nLevel, "MESSAGE ===============================");
	a_pclsLog->LogMsg(a_nLevel, "Version     : [%d]", m_stHead.m_cVer);

	strTemp.assign(m_stHead.m_szCmd, sizeof(m_stHead.m_szCmd));
	a_pclsLog->LogMsg(a_nLevel, "Command     : [%s]", strTemp.c_str());

	strTemp.clear();
	if (m_stHead.m_cFlag & FLAG_REQUEST)  { strTemp.append("Request "); }
	if (m_stHead.m_cFlag & FLAG_RESPONSE) { strTemp.append("Response "); }
	if (m_stHead.m_cFlag & FLAG_NOTIFY)   { strTemp.append("Notify "); }
	if (m_stHead.m_cFlag & FLAG_RETRNS)   { strTemp.append("Retrns "); }
	if (m_stHead.m_cFlag & FLAG_BROAD)    { strTemp.append("Broad "); }
	if (m_stHead.m_cFlag & FLAG_ERROR)    { strTemp.append("Error "); }
	a_pclsLog->LogMsg(a_nLevel, "Flag        : [0x%x] %s", m_stHead.m_cFlag, strTemp.c_str());

	a_pclsLog->LogMsg(a_nLevel, "Source      : [%d : %d]", m_stHead.m_nSrcNode, m_stHead.m_nSrcProc);
	a_pclsLog->LogMsg(a_nLevel, "Destination : [%d : %d]", m_stHead.m_nDstNode, m_stHead.m_nDstProc);
	a_pclsLog->LogMsg(a_nLevel, "Sequence    : [%d]", m_stHead.m_nSeq);
	a_pclsLog->LogMsg(a_nLevel, "Legnth      : [%d]", m_stHead.m_nLength);
	
	if (a_bPayPrint) {
		strTemp.assign(m_vecBody.begin(), m_vecBody.end());
		a_pclsLog->LogMsg(a_nLevel, "Body        : %s", strTemp.c_str());
	}
		
	return;
}



