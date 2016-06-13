#ifndef __CSTARESREQAPI_HPP__
#define __CSTARESREQAPI_HPP__

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <string>
#include <vector>
#include <list>
#include "CStaResApi.hpp"

class CStaResReqApi : public CStaResApi{
		private:
				std::list<std::string> lstRscGrpId;
				std::list<rabbit::object*> *m_lstRabbitObject;

				rabbit::object *GetRabbitObject();

		public:
				CStaResReqApi();
				~CStaResReqApi();
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
