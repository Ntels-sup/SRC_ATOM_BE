#ifndef __CRECOLUMNORDER_HPP__
#define __CRECOLUMNORDER_HPP__

#include <list>
#include <time.h>
#include <map>
#include <string>

#include "CResTableData.hpp"
#include "CStmResApi.hpp"

class CResGroup : public CStmResApi {
		private:
				std::map<std::string, CResTableData*> m_mapResTableDataMap;
				std::string m_strRscId;

		public:
				CResGroup(std::string &a_strRscId); 
				~CResGroup(); 
				int EraseResTableDataAll();
				std::string GetRscId() { return m_strRscId; };
				unsigned int GetResTableDataCount() { return m_mapResTableDataMap.size(); };
				CResTableData& operator[] (std::string a_strColumnOrder);
				CResTableData* GetResTableData(unsigned int a_nIndex);
				CResTableData* GetFirstResTableData();
				CResTableData* GetFirstResTableDataP();
				int GetEncodeMessage(rabbit::object &a_cRecord);
				int GetDecodeMessage(rabbit::object &a_cRecord);
};

#endif
