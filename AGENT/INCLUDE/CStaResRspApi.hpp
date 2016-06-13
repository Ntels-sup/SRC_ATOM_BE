#ifndef __CSTARESRESPAPI_HPP__
#define __CSTARESRESPAPI_HPP__

#include <list>
#include <string>
#include <map>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CStaResApi.hpp"
#include "CResTableData.hpp"

class CStaResRspApi : public CStaResApi {
		private:
				std::map<std::string, CResTableData*> m_mapResTableMap;
				std::list<rabbit::object*> *m_lstRabbitObject;
				std::list<rabbit::array*> *m_lstRabbitArray;
				rabbit::object *GetRabbitObject();
				rabbit::array *GetRabbitArray();

		public:
				CStaResRspApi();
				~CStaResRspApi();
				CResTableData& operator[] (std::string a_strRscGrpId);
				CResTableData* GetResTableData(unsigned int a_nIndex);
				CResTableData* GetFirstResTableData();
				CResTableData* GetFirstResTableDataP();
				int GetTableCount() { return m_mapResTableMap.size(); };
				int EraseResTable(std::string a_strTableName);
				int EraseResTableAll();
				int InsertRscGrpId(char *a_szRscGrpId);
				int InsertRscGrpId(std::string a_szRscGrpId);
				std::string GetFirstRscGrpId();
				int EncodeResTableData(CResTableData *a_cResTableData, rabbit::object *a_cRecord);
				int EncodeMessage(std::string &a_strData);
				int DecodeTableData(rabbit::object *a_cRecord, CResTableData **a_cResTableData);
				int DecodeMessage(std::string &a_strData);
};

#endif
