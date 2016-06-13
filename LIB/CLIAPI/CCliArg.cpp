#include <stdio.h>
#include "CCliArg.hpp"

CCliArg::CCliArg()
{
}

CCliArg::CCliArg(std::string &a_strName)
{
    m_strName = a_strName;
}

CCliArg::CCliArg(char *a_szName)
{
    m_strName = a_szName;
}

CCliArg::~CCliArg()
{
    PopAll();
}

CCliArgPara& CCliArg::operator[] (unsigned int a_nIndex)
{
    unsigned int i = 0;
    std::list<CCliArgPara*>::iterator iter;

    if(a_nIndex>= m_lstArgPara.size()){
		 throw m_strErrString = "Invalid arguemnt parameter index";
    }

    for(i=0, iter = m_lstArgPara.begin(); i<a_nIndex; iter++, i++){
    }

    return **iter;
}

void CCliArg::PopFirst()
{
    CCliArgPara *cArgPara = NULL;

    if(m_lstArgPara.size() == 0){
        return;
    }

    cArgPara = m_lstArgPara.front();
    m_lstArgPara.pop_front();

    delete cArgPara;
}

void CCliArg::PopLast()
{
    CCliArgPara *cArgPara = NULL;

    if(m_lstArgPara.size() == 0){
        return;
    }

    cArgPara = m_lstArgPara.back();
    m_lstArgPara.pop_back();

    delete cArgPara;
}

void CCliArg::Pop(unsigned int a_nIndex)
{
	unsigned int i = 0;
	CCliArgPara *cArgPara = NULL;
	std::list<CCliArgPara*>::iterator iter;

	if(a_nIndex >= m_lstArgPara.size()){
		return;
	}

	for(i=0, iter = m_lstArgPara.begin(); i<a_nIndex; iter++, i++){
	}

	cArgPara = *iter;

	if(cArgPara != NULL){
		delete cArgPara;
	}
}

void CCliArg::PopAll()
{
    CCliArgPara *argPara = NULL;

   while(1){
       if(m_lstArgPara.size() == 0){
           break;
       }

       argPara = m_lstArgPara.front();
       m_lstArgPara.pop_front();

       delete argPara;
   }
}

int CCliArg::GetEncodeMessage(rabbit::object &a_cRecord)
{
	std::list<CCliArgPara*>::iterator iter;
	CCliArgPara *cArgPara = NULL;
	int nRet = 0;

	try {
		rabbit::array *cArgParaArray = NULL;
		rabbit::object *cRecord = NULL;

		a_cRecord["name"] = m_strName;

		cArgParaArray = GetRabbitArray();

		*cArgParaArray = a_cRecord["parameter_array"];

		for(iter = m_lstArgPara.begin();iter != m_lstArgPara.end(); iter++){
			cArgPara = *iter;
			cRecord = GetRabbitObject();
			nRet = cArgPara->GetEncodeMessage(*cRecord);
			if(nRet != CCliApi::RESULT_OK){
				return nRet;
			}

			cArgParaArray->push_back(*cRecord);
		}
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

int CCliArg::GetDecodeMessage(rabbit::object &a_cRecord)
{
	unsigned int i = 0;
	std::string strTableName;
	std::string strTableType;
	std::string strRscId;

	try {
		rabbit::array cArgParaArray;

		if(m_strName.size() == 0){
			m_strName = a_cRecord["name"].as_string();
		}

		cArgParaArray = a_cRecord["parameter_array"];

		for(i=0;i<cArgParaArray.size();i++){
			rabbit::object cObject;
			std::string strPara;
			int nPara = 0;
			CCliArgPara cArgPara;

			cObject = cArgParaArray[i];

			if(cObject["parameter"].is_string() != 0){
				strPara = cObject["parameter"].as_string();
				PushPara(strPara);
			}
			else {
				nPara = cObject["parameter"].as_uint();
				PushPara(nPara);
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
