#ifndef __CSTMANSAPI_HPP__
#define __CSTMANSAPI_HPP__

#include <string>
#include <list>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CStmApi.hpp"
#include "CCollectValue.hpp"

using namespace std;

class CStmAnsApi : public CStmApi{
		private:
				unsigned int m_nSessionId;
				std::string m_strPackageName;
				int m_nTableType;
				std::string m_strTableName;
				//std::string m_strParsingError;
				list<CCollectValue*> *m_lstCollectValue;
				list<rabbit::object*> *m_lstRabbitObject;
				list<rabbit::array*> *m_lstRabbitArray;
				int m_nResultCode;

				rabbit::object *GetRabbitObject();
				rabbit::array *GetRabbitArray();
				void ClearCollectValue();
				CStmApi::eResult MakeCollectValue(rabbit::array *a_cRecordArray);
				CStmApi::eResult EncodeCollectValue(CCollectValue *a_cValue, rabbit::object *a_cRecord);
		public:
				CStmAnsApi();
				~CStmAnsApi();
				CStmApi::eResult Init(int a_nSessionId, string &a_strPackageName, string &a_strTableName, int a_nTableType);
				void SetResultCode(int a_nResultCode) { m_nResultCode = a_nResultCode; };
				int GetResultCode() { return m_nResultCode; };
				int GetSessionId() { return m_nSessionId; };
				std::string GetPackageName() { return m_strPackageName; };
				string GetTableName() { return m_strTableName; };
				int GetTableType() { return m_nTableType; };
				list<CCollectValue*> *GetCollectValue() {
						list<CCollectValue*> *tmp = NULL;
						tmp = m_lstCollectValue;
						m_lstCollectValue = NULL;
						return tmp;
				};
				list<CCollectValue*> *GetCollectValueP() { return m_lstCollectValue; };
				CStmApi::eResult SetCollectValue(list<CCollectValue*> *collectValue);
				CStmApi::eResult DecodeMessage(char *a_chData, int a_nDataLen);
				CStmApi::eResult EncodeMessage(char *a_chData, unsigned int a_nMaxLen, unsigned int *a_nDataLen);
				CStmApi::eResult EncodeErrorMessage(int a_nSessionId, int a_nErrorCode, char *a_chData, 
								unsigned int a_nMaxLen, unsigned int *a_nDataLen);
};

#endif
