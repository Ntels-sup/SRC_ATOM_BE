#include <stdio.h>
#include "CCliReq.hpp"

CCliReq::CCliReq()
{
	m_nSessionId = 0;
	m_nCmdCode = 0;
}

CCliReq::CCliReq(unsigned int a_nSessionId)
{
	m_nSessionId = a_nSessionId;
	m_nCmdCode = 0;
}

CCliReq::CCliReq(unsigned int a_nSessionId, int a_nCmdCode, std::string &a_strCmdName)
{
	m_nSessionId = a_nSessionId;
	m_nCmdCode = a_nCmdCode;
	m_strCmdName = a_strCmdName;
}

CCliReq::CCliReq(unsigned int a_nSessionId, int a_nCmdCode, const char *a_szCmdName)
{
	m_nSessionId = a_nSessionId;
	m_nCmdCode = a_nCmdCode;
	m_strCmdName = a_szCmdName;
}

CCliReq::CCliReq(unsigned int a_nSessionId, int a_nCmdCode, std::string &a_strCmdName, std::string &a_strPkgName )
{
	m_nSessionId = a_nSessionId;
	m_nCmdCode = a_nCmdCode;
	m_strCmdName = a_strCmdName;
	m_strPkgName = a_strPkgName;
}

CCliReq::CCliReq(unsigned int a_nSessionId, int a_nCmdCode, const char *a_szCmdName, const char *a_szPkgName )
{
	m_nSessionId = a_nSessionId;
	m_nCmdCode = a_nCmdCode;
	m_strCmdName = a_szCmdName;
	m_strPkgName = a_szPkgName;
}

CCliReq::~CCliReq()
{
	PopAll();
}

CCliArg& CCliReq::operator[] (std::string &a_strArgName)
{
	return *PushArg(a_strArgName);
}

CCliArg* CCliReq::PushArg(std::string &a_strArgName)
{
	std::map<std::string, std::list<CCliArg*>::iterator>::iterator iter;

	iter = m_mapArg.find(a_strArgName);
	if(iter != m_mapArg.end()){
		return *iter->second;
	}

	CCliArg *cArg = NULL;

	cArg = new CCliArg(a_strArgName);
	
	m_lstArg.push_back(cArg);
	m_mapArg.insert(std::map<std::string, std::list<CCliArg*>::iterator>::value_type(a_strArgName, --m_lstArg.end()));

	return cArg;
}

CCliArg& CCliReq::operator[] (const char *a_szArgName)
{
	std::string strArgName;
	std::map<std::string, std::list<CCliArg*>::iterator>::iterator iter;

	strArgName = a_szArgName;

	return (*this)[strArgName];

}

CCliArg& CCliReq::operator[] (unsigned int a_nIndex)
{
	unsigned int i = 0;
	 std::list<CCliArg*>::iterator iter;

	if(m_lstArg.size() <= a_nIndex){
		throw m_strErrString = "Invalid argument index";
	}

	for(i = 0, iter = m_lstArg.begin();i < a_nIndex; i++, iter++){
	}

	return **iter;
}

int CCliReq::IsExist(std::string &a_strArgName)
{
	std::map<std::string, std::list<CCliArg*>::iterator>::iterator iter;

	if(m_mapArg.size() == 0){
		return CCliApi::RESULT_NOK;
	}

	iter = m_mapArg.find(a_strArgName);
	if(iter != m_mapArg.end()){
		return CCliApi::RESULT_OK;

	}

	return CCliApi::RESULT_NOK;
}

int CCliReq::IsExist(const char *a_szArgName)
{
	std::string strArgName;

	strArgName = a_szArgName;

	return IsExist(strArgName);
}

CCliArg* CCliReq::GetArg(unsigned int a_nIndex)
{
	unsigned int i = 0;
	std::list<CCliArg*>::iterator iter;

	if(m_lstArg.size() >= a_nIndex) {
		return NULL;
	}

	for(i=0;i<a_nIndex;i++){
		iter++;
	}

	return *iter;
}

CCliArg* CCliReq::GetFirstArg()
{
	if(m_lstArg.size() == 0){
		return NULL;
	}

	return m_lstArg.front();
}

CCliArg* CCliReq::GetLastArg()
{
	if(m_lstArg.size() == 0){
		return NULL;
	}

	return m_lstArg.back();
}

void CCliReq::PopFirst()
{
	CCliArg *cArg = NULL;

	if(m_lstArg.size() == 0){
		return;
	}

	cArg = m_lstArg.front();

	m_mapArg.erase(m_mapArg.find(cArg->GetName()));

	m_lstArg.pop_front();

	delete cArg;
}

void CCliReq::PopLast()
{
	CCliArg *cArg = NULL;

	if(m_mapArg.size() == 0){
		return;
	}

	cArg = m_lstArg.back();

	m_mapArg.erase(m_mapArg.find(cArg->GetName()));

	m_lstArg.pop_back();

	delete cArg;
}

void CCliReq::Pop(std::string &a_strArgName)
{
	CCliArg *cArg = NULL;
	std::map<std::string, std::list<CCliArg*>::iterator>::iterator iter;

	if(m_mapArg.size() == 0){
		return;
	}

	iter = m_mapArg.find(a_strArgName);
	if(iter == m_mapArg.end()){
		return;
	}

	cArg = *iter->second;
	m_lstArg.erase(iter->second);

	m_mapArg.erase(iter);

	delete cArg;
}

void CCliReq::Pop(const char *a_szArgName)
{
	std::map<std::string, CCliArg*>::iterator iter;
	std::string strArgName;

	strArgName = a_szArgName;

	Pop(strArgName);
}

void CCliReq::Pop(unsigned int a_nIndex)
{
	unsigned int i = 0;
	std::list<CCliArg*>::iterator iter;

	if(m_lstArg.size() >= a_nIndex) {
		return;
	}

	for(i=0;i<a_nIndex;i++){
		iter++;
	}

	m_mapArg.erase(m_mapArg.find((*iter)->GetName()));

	delete *iter;

	m_lstArg.erase(iter);
}

void CCliReq::PopAll()
{
	std::list<CCliArg*>::iterator iter;

	if(m_lstArg.size() == 0){
		return;
	}

	while(m_lstArg.size() != 0){
		delete m_lstArg.front();
		m_lstArg.pop_front();
	}

	m_mapArg.clear();
}

int CCliReq::EncodeMessage(std::vector<char>& a_vecData)
{
	int nRet = RESULT_OK;
	std::string strData;

	nRet = EncodeMessage(strData);
	if(nRet != RESULT_OK){
		return nRet;
	}

	a_vecData.clear();
	a_vecData.insert(a_vecData.end(), strData.begin(), strData.end());

	return RESULT_OK;
}


int CCliReq::EncodeMessage(std::string &a_strData)
{
	std::list<CCliArg*>::iterator iter;
	CCliArg *cArg = NULL;
	int nRet = 0;

	try {
		rabbit::object cRoot;
		rabbit::object cBody = cRoot["BODY"];
		rabbit::array *cArgArray = NULL;
		rabbit::object *cRecord = NULL;

		cBody["session_id"] = m_nSessionId;
		if(m_strCmdLine.size() != 0){
			cBody["cmd_line"] = m_strCmdLine;
		}

		if(m_strPkgName.size() != 0){
			cBody["package"] = m_strPkgName;
		}
		cBody["command_code"] = m_nCmdCode;
		cBody["command"] = m_strCmdName;

		cArgArray = GetRabbitArray();

		if(m_lstArg.size() != 0){
			*cArgArray = cBody["argument_array"];

			for(iter = m_lstArg.begin();iter != m_lstArg.end(); iter++){
				cArg = *iter;
				cRecord = GetRabbitObject();
				nRet = cArg->GetEncodeMessage(*cRecord);
				if(nRet != CCliApi::RESULT_OK){
					return nRet;
				}

				cArgArray->push_back(*cRecord);
			}
		}

		a_strData = cRoot.str();

	} catch(rabbit::type_mismatch   e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(rabbit::parse_error e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(...) {
		m_strErrString.append("Unknown Error");
		return CCliApi::RESULT_PARSING_ERROR;
	}

	return CCliApi::RESULT_OK;
}

int CCliReq::DecodeMessage(std::vector<char>& a_vecData)
{
	int nRet = 0;
	std::string  strPayload;

	strPayload = &a_vecData[0];

	nRet = DecodeMessage(strPayload);

	return nRet;
}


int CCliReq::DecodeMessage(std::string &a_strData)
{
	int nRet = 0;
	unsigned int i = 0;
	std::string strTableName;
	std::string strTableType;
	std::string strRscId;

	try {
		rabbit::document doc;
		rabbit::object cBody;
		rabbit::array cArgArray;
		doc.parse(a_strData);

		cBody = doc["BODY"];

		m_nSessionId = cBody["session_id"].as_uint();

		if(cBody.has("cmd_line")  != 0){
			m_strCmdLine = cBody["cmd_line"].as_string();
		}

		if(cBody.has("package") != 0){
			m_strPkgName = cBody["package"].as_string();
		}
		m_nCmdCode = cBody["command_code"].as_uint();
		m_strCmdName = cBody["command"].as_string();

		if(cBody.has("argument_array")){
			cArgArray = cBody["argument_array"];

			for(i=0;i<cArgArray.size();i++){
				rabbit::object cObject;
				std::string argName;
				CCliArg *cArg = NULL;

				cObject = cArgArray[i];

				argName = cObject["name"].as_string();

				cArg = PushArg(argName);

				nRet = cArg->GetDecodeMessage(cObject);
				if(nRet != RESULT_OK){
					return nRet;
				}
			}
		}
	} catch(rabbit::type_mismatch   e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(rabbit::parse_error e) {
		m_strErrString.append(e.what());
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(std::string e) {
		m_strErrString.append(e);
		return CCliApi::RESULT_PARSING_ERROR;
	} catch(...) {
		m_strErrString.append("Unknown Error");
		return CCliApi::RESULT_PARSING_ERROR;
	}

	return CCliApi::RESULT_OK;
}

