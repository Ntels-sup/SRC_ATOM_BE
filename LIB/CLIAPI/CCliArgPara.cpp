#include "CCliArgPara.hpp"

CCliArgPara::CCliArgPara()
{
    m_nParaType = PARA_TYPE_NONE;
    m_nPara = 0;
}

CCliArgPara::CCliArgPara(std::string &a_strPara)
{
    SetString(a_strPara);
}

CCliArgPara::CCliArgPara(const char *a_szPara)
{
    SetString(a_szPara);
}

CCliArgPara::CCliArgPara(int &a_nPara)
{
    SetNumber(a_nPara);
}

CCliArgPara::CCliArgPara(unsigned int &a_nPara)
{
    SetNumber(a_nPara);
}

CCliArgPara::~CCliArgPara()
{
    m_nParaType = PARA_TYPE_NONE;
    m_nPara = 0;
}

void CCliArgPara::SetString(std::string &a_strPara)
{
    m_nParaType = PARA_TYPE_STRING;
    m_strPara = a_strPara;
}

void CCliArgPara::SetString(const char* a_szPara)
{
    m_nParaType = PARA_TYPE_STRING;
    m_strPara = a_szPara;
}

void CCliArgPara::SetNumber(int a_nPara)
{
    m_nParaType = PARA_TYPE_NUMBER;
    m_nPara = (unsigned int)a_nPara;
}

void CCliArgPara::SetNumber(unsigned int a_nPara)
{
    m_nParaType = PARA_TYPE_NUMBER;
    m_nPara = a_nPara;
}

std::string& CCliArgPara::GetString()
{
	if(m_nParaType != PARA_TYPE_STRING){
		throw m_strErrString = "Invalid Parameter Type ";
	}
    return m_strPara;
}

int CCliArgPara::GetNumber()
{
	if(m_nParaType != PARA_TYPE_NUMBER){
		throw m_strErrString = "Invalid Parameter Type ";
	}

    return m_nPara;
}

bool CCliArgPara::IsNumber()
{
	if(m_nParaType == PARA_TYPE_NUMBER){
		return true;
	}

	return false;
}

bool CCliArgPara::IsString()
{
	if(m_nParaType == PARA_TYPE_STRING){
		return true;
	}

	return false;
}

int CCliArgPara::GetEncodeMessage(rabbit::object &a_cRecord)
{
	std::list<CCliArgPara*>::iterator iter;

	try {
		if(m_nParaType == PARA_TYPE_NUMBER){
			a_cRecord["parameter"] = m_nPara;
		}
		else {
			a_cRecord["parameter"] = m_strPara;
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

	return RESULT_OK;
}

