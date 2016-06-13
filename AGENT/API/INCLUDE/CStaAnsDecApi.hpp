#ifndef __CSTAANSDECAPI_HPP__
#define __CSTAANSDECAPI_HPP__

#include <time.h>
#include <string>
#include <list>

using namespace std;

class CStaAnsDecApi{
		private:
				string m_strTableName;
				int m_nTableType;
				list<string> *m_lstPrimaryKey;
				list<int> *m_lstValue;
				time_t m_nCollectTime;

		public:
				CStaAnsDecApi();
				~CStaAnsDecApi();
				int GetTableType() { return m_nTableType; };
				string GetTableName(){ return m_strTableName; };
				list<string> *GetPrimaryKey();
				list<int> *GetValue();	
				time_t GetCollectTime() { return m_nCollectTime; };
				int DecodeMessage(char *a_chData, int a_nDataLen);
};

#endif
