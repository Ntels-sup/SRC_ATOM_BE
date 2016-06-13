#ifndef __CSTARESRESPAPI_HPP__
#define __CSTARESRESPAPI_HPP__

#include <list>
#include <string>
#include <map>

//#include "rapidjson/document.h"
//#include "rapidjson/writer.h"
//#include "rapidjson/stringbuffer.h"
//#include "rabbit.hpp"

#include "CStmResApi.hpp"
#include "CResGroup.hpp"
#include "CResTableData.hpp"

class CStmResRspApi : public CStmResApi {
		private:
				int m_nResultCode;
				std::map<std::string, CResGroup*> m_mapResGroupMap;

				int EncodeResGroup(CResGroup *a_cResGroup, rabbit::object *a_cRecord);
				int DecodeResGroup(rabbit::object *a_cRecord, CResGroup **a_cResGroup);
		public:
				CStmResRspApi();
				~CStmResRspApi();
				void Init(int a_nSessionId, int a_nResultCode) {
						m_nSessionId = a_nSessionId;  
						m_nResultCode = a_nResultCode;
				};
				CResGroup& operator[] (std::string a_strRscId);
				CResGroup* GetResGroup(unsigned int a_nIndex);
				CResGroup* GetFirstResGroup();
				CResGroup* GetFirstResGroupP();
				int GetSessionId() { return m_nSessionId; };
				int GetResultCode() { return m_nResultCode; };
				int GetResGroupCount() { return m_mapResGroupMap.size(); };
				int EraseResGroup(std::string a_strRscId);
				int EraseResGroupAll();
				int EncodeMessage(std::string &a_strData);
				int DecodeMessage(std::string &a_strData);
};

#endif
