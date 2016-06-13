#ifndef __CCLIREGREQAPI_HPP__
#define __CCLIREGREQAPI_HPP__
#include <vector>
#include <string>
#include "CCliRegApi.hpp"

class CCliRegReqApi : public CCliRegApi {
		private:
				std::string m_strUserId;
		public:
				CCliRegReqApi();
				~CCliRegReqApi();
				int Init(std::string &a_strUserId);
				int Init(char *a_szuserId);
				std::string GetUserId() { return m_strUserId; };
				int DecodeMessage(std::vector<char>& a_vecPayload);
				int DecodeMessage(std::string &a_strPayload);
				int EncodeMessage(std::vector<char>& a_vecPayload);	
				int EncodeMessage(std::string &a_strPayload);
};

#endif

