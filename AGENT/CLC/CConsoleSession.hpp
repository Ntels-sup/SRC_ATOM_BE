#ifndef __CCONSOLESESSION_HPP__
#define __CCONSOLESESSION_HPP__
#include <time.h>
#include <string>

using namespace std;

class CConsoleSession{
	private:
		time_t m_sndTime;
		unsigned int m_nType;
		unsigned int m_nSessionId;
		unsigned int m_nResultCode;
		unsigned int m_nCmdCode;
		string m_strUserId;
		string m_strCmd;
		string m_strCmdDesc;
		string m_strResult;
		string m_strRequest;
		string m_strResponse;

	public:
		CConsoleSession(unsigned int a_nType, time_t a_sndTime);
		CConsoleSession(unsigned int a_nType, unsigned int a_nSessionId, time_t a_sndTime);
		~CConsoleSession();
		time_t GetTime() { return m_sndTime; };
		string GetUserId() { return m_strUserId; };
		unsigned int GetType() { return m_nType; };
		void SetUserId(string a_strUserId) { m_strUserId = a_strUserId; };
		void SetCmd(unsigned int a_nCmdCode, string a_strCmd, string a_strCmdDesc);
		void GetCmdP(unsigned int *a_nCmdCode, string **a_strCmd, string **a_strCmdDesc);
		unsigned int GetSessionId() { return m_nSessionId; };
		void SetResultCode(unsigned int a_nResultCode, string &a_strResult) { 
			m_nResultCode = a_nResultCode; 
			m_strResult = a_strResult; 
		};
		void SetResultCode(unsigned int a_nResultCode, const char *a_szResult) { 
			m_nResultCode = a_nResultCode; 
			m_strResult = a_szResult; 
		};
		void GetResultCode(unsigned int *a_nResultCode, string *a_strResult) { 
			*a_nResultCode = m_nResultCode; 
			*a_strResult = m_strResult; 
		};
		void SetRequest(string &a_strRequest);
		void SetResponse(string &a_strRequest);
		string* GetRequestP() { return &m_strRequest; };
		string* GetResponseP() { return &m_strResponse; };
};

#endif
