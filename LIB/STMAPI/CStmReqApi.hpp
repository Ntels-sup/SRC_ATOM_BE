#ifndef __CSTMREQAPI_HPP__
#define __CSTMREQAPI_HPP__

#include "CStmApi.hpp"

class CStmReqApi : public CStmApi{
		private:
				int m_nSessionId;
				bool m_blnActFlag;
				time_t m_startTime;
				int m_nCollectTime;
				std::string m_strTableName;
				//std::string m_strParsingError;
		public:
				CStmReqApi();
				~CStmReqApi();
				//int Init(int a_nSessionId, time_t a_startTime, int a_nCollectTime);
				CStmApi::eResult Init(int a_nSessionId, time_t a_startTime, int a_nCollectTime);
				void SetCollectTime(time_t a_startTime, int a_nCollectTime){
						m_startTime = a_startTime;
						m_nCollectTime = a_nCollectTime;
				}
				void SetTableName(std::string a_strTableName) {
						m_strTableName.append(a_strTableName);
				}
				int GetSessionId() { return m_nSessionId; };
				time_t GetStartTime() { return m_startTime; };
				int GetCollectTime() { return m_nCollectTime; };
				std::string GetTableName() { return m_strTableName; };
				CStmApi::eResult DecodeMessage(char *a_chData, int a_nDataLen);
				CStmApi::eResult EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen);
				CStmApi::eResult EncodeMessage(std::string &strData);
};

#endif
