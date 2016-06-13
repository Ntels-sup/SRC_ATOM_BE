/* vim:ts=4:sw=4
 */
/**
 * \file	CAppQueue.hpp
 * \brief	APP 프로세스 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

// common header
#include "CFileLog.hpp"

// PRA header
#include "CConfigPra.hpp"
#include "CAppQueue.hpp"

extern CFileLog* g_pclsLogPra;


CAppQueue& CAppQueue::Instance(void)
{
	if (m_pclsInstance == NULL) {
		m_pclsInstance = new CAppQueue();
	}

	return *m_pclsInstance;
}

bool CAppQueue::OpenQ(CLQManager* a_pclsQ)
{
	if (a_pclsQ != NULL) {
		m_pclsQ = a_pclsQ;
		return true;
	}
	
	CConfigPra& clsCfg = CConfigPra::Instance();
	m_pclsQ = new CLQManager((char*)clsCfg.m_strPkgName.c_str(), 
							(char*)clsCfg.m_strNodeType.c_str(),
							(char*)clsCfg.m_strProcName.c_str(),
							0, false, false, NULL);

	if	(m_pclsQ->Initialize(DEF_CMD_TYPE_SEND) < 0) {
		g_pclsLogPra->ERROR("CMQ, command queue initalize failed, %s", m_pclsQ->GetErrorMsg());
		return false;
	}
	
	return true;
}

bool CAppQueue::CmdRingClear(const char* a_szProcName)
{
	char szQName[64] = { 0x00, };
	snprintf(szQName, sizeof(szQName), "%s_0", a_szProcName);
//	if (m_pclsQ->InitCommandRing(a_szProcName, 0, DEF_CMD_TYPE_RECV) < 0) {
	if (m_pclsQ->DeleteQueue(szQName) < 0) {
		g_pclsLogPra->ERROR("CMQ, command queue clear failed, %s", m_pclsQ->GetErrorMsg());
		return false;
	}

	return true;
}

bool CAppQueue::SendCmd(CProtocol& a_clsProto, const char* a_szTarget)
{
	if (m_pclsQ == NULL) {
		g_pclsLogPra->ERROR("CMQ, command queue not opend");
		return false;
	}

	vector<char> vecMesg;
	a_clsProto.GenStream(vecMesg);
	
	int nRet = m_pclsQ->SendCommand((char*)a_szTarget, 0, &vecMesg[0], vecMesg.size());
	if (nRet != 0) {
		g_pclsLogPra->ERROR("CMQ, command send failed, to: %s, ret: %d", a_szTarget, nRet);
		a_clsProto.Print(g_pclsLogPra, LV_INFO, true);
		return false;
	}

	return true;
}

int CAppQueue::RecvCmd(CProtocol& a_clsProto)
{
	if (m_pclsQ == NULL) {
		g_pclsLogPra->ERROR("CMQ, command queue not opend");
		return false;
	}

	char* szRecvBuf = NULL;
	int nRead = m_pclsQ->ReceiveCommand(&szRecvBuf);
	if (nRead < 0) {
		if (nRead == -E_Q_NOENT) {
			return 0;
		}
		g_pclsLogPra->ERROR("CMQ, receive command failed, ret: %d\n", nRead);
    	return -1;
	}
	if (a_clsProto.SetAll(szRecvBuf, nRead) == false) {
		g_pclsLogPra->ERROR("CMQ, invalied receive message");
		a_clsProto.Print(g_pclsLogPra, LV_INFO, true);
		m_pclsQ->FreeReadData();
		return -1;
	}
	m_pclsQ->FreeReadData();

	return 1;
}

CAppQueue* CAppQueue::m_pclsInstance = NULL;
CAppQueue::CAppQueue()
{
	m_pclsQ = NULL;
	return;
}

CAppQueue::~CAppQueue()
{
	if (m_pclsQ) {
		delete m_pclsQ;
		m_pclsQ = NULL;
	}
}
