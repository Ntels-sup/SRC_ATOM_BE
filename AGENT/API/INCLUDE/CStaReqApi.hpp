#ifndef __CSTA_REQAPI_HPP__
#define __CSTA_REQAPI_HPP__

#include<string>
#include <time.h>

class CStaReqApi{
		private:
				time_t m_collectTime;
				std::string m_strParsingError;
		public:

				CStaReqApi();
				~CStaReqApi();
				void SetTime(time_t a_collectTime) { m_collectTime= a_collectTime; };
				time_t GetTime();
				std::string GetParsingError() { return m_strParsingError; };
				int EncodeMessage(std::string &a_chData);
				int EncodeMessage(char *a_chData, unsigned int a_nMaxData, unsigned int *a_nDataLen);
				int DecodeMessage(std::string &a_chData);
				int DecodeMessage(char *a_chData, unsigned int a_nDataLen);
};

#endif
