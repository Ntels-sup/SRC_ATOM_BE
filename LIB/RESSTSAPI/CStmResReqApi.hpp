#ifndef __CSTMRESREQAPI_HPP__
#define __CSTMRESREQAPI_HPP__

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <string>
#include <vector>
#include <list>
#include "CStmResApi.hpp"

class CStmResReqApi : public CStmResApi{
		private:
				std::list<std::string> m_lstRscGrpId;
				std::list<rabbit::object*> *m_lstRabbitObject;

				rabbit::object *GetRabbitObject();

		public:
				CStmResReqApi();
				~CStmResReqApi();
				int Init(int a_nSessionId, time_t a_nCollectTime);
				int GetSessionId() { return m_nSessionId; };
				time_t GetCollectTime() { return m_nCollectTime; };
				int RscGrpIdSize();
				int InsertRscGrpId(char *a_szRscGrpId);
				int InsertRscGrpId(std::string a_szRscGrpId);
				std::string GetFirstRscGrpId();
				int DecodeMessage(std::vector<char>& a_vecPayload);
				int DecodeMessage(std::string &a_strPayload);
				int EncodeMessage(std::vector<char>& a_vecPayload);
				int EncodeMessage(std::string &a_strPayload);
};

#endif
