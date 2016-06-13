#include <list>
#include <string>

using namespace std;

class CStaAnsEncApi{
		private:
				string m_strTableName;
				int m_nTableType;
				time_t m_nCollectTime;
				list<string> m_lstPrimaryKey;
				list<int> m_lstValue;

		public:
				CStaAnsEncApi();
				~CStaAnsEncApi();
                int SetTableInfo(string a_strTableName, int a_nTableType, time_t a_nCollectTime);
				int InsertPrimaryKey(string a_strPrimaryKey);
				int InsertValue(int a_nValue);
				int EncAnswerMessage(char *a_chData, int a_nMaxLen, int *a_nDataLen);
};
