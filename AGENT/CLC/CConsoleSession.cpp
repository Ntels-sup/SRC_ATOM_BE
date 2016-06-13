#include <unistd.h>
#include "CConsoleSession.hpp"

CConsoleSession::CConsoleSession(unsigned int a_nType, time_t a_sndTime)
{
    m_nSessionId = 0;
	m_nResultCode = 0;
	m_nType = a_nType;
	m_sndTime = a_sndTime;
}

CConsoleSession::CConsoleSession(unsigned int a_nType, unsigned int a_nSessionId, time_t a_sndTime)
{
	m_nType = a_nType;
    m_nSessionId = a_nSessionId;
	m_sndTime = a_sndTime;
}

CConsoleSession::~CConsoleSession()
{
}

void CConsoleSession::SetCmd(unsigned int a_nCmdCode, string a_strCmd, string a_strCmdDesc)
{
	m_nCmdCode = a_nCmdCode; 
	m_strCmd = a_strCmd;
	m_strCmdDesc = a_strCmdDesc;
}

void CConsoleSession::GetCmdP(unsigned int *a_nCmdCode, string **a_strCmd, string **a_strCmdDesc)
{
	*a_nCmdCode = m_nCmdCode; 
	*a_strCmd = &m_strCmd;
	*a_strCmdDesc = &m_strCmdDesc;
}

void CConsoleSession::SetRequest(string &a_strRequest)
{
	m_strRequest = a_strRequest;
}

void CConsoleSession::SetResponse(string &a_strResponse)
{
	m_strResponse = a_strResponse;
}
