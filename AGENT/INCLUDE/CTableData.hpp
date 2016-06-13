#ifndef __CTABLEDATA_HPP__
#define __CTABLEDATA_HPP__

#include <time.h>
#include <list>
#include <string>

class CTableData{
		private:
				std::string m_strTableName;
				int m_nTableType;
				time_t m_nCollectTime;
				std::list<std::string> *m_lstPrimaryKey;
				std::list<int> *m_lstValue;
				int m_nErrorLen;
				char m_chError[STAAPI_ERROR_MESSAGE_LEN];

		public:
				CTableData(std::string a_strTableName);
				~CTableData();
				char* GetError() { return m_chError; };
				int SetTableInfo(time_t a_nCollectTime, int a_nTableType=STAAPI_TABLE_TYPE_STS);
				int InsertPrimaryKey(std::string a_strPrimaryKey);
				int InsertValue(int a_nValue);
				int GetTableType() { return m_nTableType; };
				std::string GetTableName(){ return m_strTableName; };
				std::list<std::string> *GetPrimaryKey();
				std::list<int> *GetValue();	
				std::list<std::string> *GetPrimaryKeyP() { return m_lstPrimaryKey; };
				std::list<int> *GetValueP() { return m_lstValue; };	
				time_t GetCollectTime() { return m_nCollectTime; };
};

#endif
