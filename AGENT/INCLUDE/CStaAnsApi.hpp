#ifndef __CSTAANSAPI_HPP__
#define __CSTAANSAPI_HPP__

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <time.h>
#include <list>
#include <string>
#include <map>

#include "CTableData.hpp"

class CStaAnsApi{
		private:
				std::map<std::string, CTableData*> m_mapTableMap;
				std::list<rabbit::object*> *m_lstRabbitObject;
				std::list<rabbit::array*> *m_lstRabbitArray;
				std::string m_strParsingError;
				int m_nErrorLen;
				char m_chError[STAAPI_ERROR_MESSAGE_LEN];

				rabbit::object *GetRabbitObject();
				rabbit::array *GetRabbitArray();
				int EncodeTableData(CTableData *a_cTableData, rabbit::object *a_cRecord);
				int DecodeTableData(rabbit::object *a_cRecord, CTableData **a_cTableData);
		public:
				CStaAnsApi();
				~CStaAnsApi();
				std::string GetError() {
						std::string strError;
						strError.append(m_chError);
						return strError;
				}
				CTableData& operator[] (std::string a_strTableName);
				CTableData* GetTableData(unsigned int a_nIndex);
				CTableData* GetFirstTableData();
				CTableData* GetFirstTableDataP();
				int GetTableCount() { return m_mapTableMap.size(); };
				int EraseTable(std::string a_strTableName);
				int EraseTableAll();
				int EncodeMessage(std::string &a_strData);
				int EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen);
				int DecodeMessage(std::string &a_strData);
				int DecodeMessage(char *a_chData, unsigned int a_nDataLen);
};

#endif
