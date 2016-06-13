#ifndef __CCLIRSP_HPP__
#define __CCLIRSP_HPP__

#include <string>
#include <vector>

#include "CCliApi.hpp"

class CCliRsp : public CCliApi{
	private:
		unsigned int m_nSessionId;
		int m_nSeqType;
		unsigned int m_nResultCode;
		std::string m_strResult;
		std::string m_strText;
		unsigned int m_nMaxBufferLen;
		char *m_szBuffer;

		void Init(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode);

	public:
		static const int SEQ_TYPE_CONTINUE = 1;
		static const int SEQ_TYPE_END      = 2;

		CCliRsp();
		CCliRsp(unsigned int a_nSessionId);
		CCliRsp(unsigned int a_nSessionId, int a_nSeqType);
		CCliRsp(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode);
		~CCliRsp();
		void SetSeqType(int a_nSeqType);
		int GetSeqType() { return m_nSeqType; };
		void SetSessionId(unsigned int a_nSessionId) { m_nSessionId = a_nSessionId; };
		unsigned int GetSessionId() { return m_nSessionId; };
		void GetResultCode(unsigned int &a_nResultCode, std::string &a_strResult){
			a_nResultCode = m_nResultCode; 
			a_strResult = m_strResult; 
		}
		void SetResultCode(unsigned int a_nResultCode, std::string &a_strResult) {
			m_nResultCode = a_nResultCode; 
			m_strResult = a_strResult; 
		};
		void SetResultCode(unsigned int a_nResultCode, const char *a_szResult) {
			m_nResultCode = a_nResultCode; 
			m_strResult = a_szResult; 
		};
		void ResetHeader(unsigned int a_nSessionId, int a_nSeqType, unsigned int a_nResultCode);
		void SetText(std::string &a_strText);
		void SetText(const char *a_szText);
		void SetTextAppend(std::string &a_strText);
		void SetTextAppend(const char *a_szText);
		void NPrintf(unsigned int a_nMaxLength, const char *fmt,...);
		std::string& GetText() { return m_strText; };
		void ClearAll();
		void ClearData();
		int EncodeMessage(std::vector<char>& a_vecData);
		int EncodeMessage(std::string &a_strData);
		int DecodeMessage(std::vector<char>& a_vecData);
		int DecodeMessage(std::string &a_strData);
};

#endif
