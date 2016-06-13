#ifndef __CCOLLECTVALUE_HPP__
#define __CCOLLECTVALUE_HPP__
#include <string>
#include <list>

using namespace std;

class CCollectValue{
		private:
				list<string> *m_lstPrimaryKey;
				list<int> *m_lstValue;
		public:
				CCollectValue();
				~CCollectValue();
				void SetPrimaryKey(list<string> *a_lstPrimaryKey) { m_lstPrimaryKey = a_lstPrimaryKey; };
				void SetValue(list<int> *a_lstValue) { m_lstValue = a_lstValue; };
				list<string>* GetPrimaryKeyList() { 
						list<string> *lstTmp = NULL;
						lstTmp = m_lstPrimaryKey;
						m_lstPrimaryKey = NULL;
						return lstTmp;
				}
				list<int>* GetValueList() { 
						list<int> *lstTmp = NULL;
						lstTmp = m_lstValue;
						m_lstValue = NULL;
						return lstTmp;
				}
				list<string>* GetPrimaryKeyListP() { return m_lstPrimaryKey; };
				list<int>* GetValueListP() { return m_lstValue; };

};

#endif
