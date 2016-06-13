/* vim:ts=4:sw=4
 */
/**
 * \file	CAppControl.hpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CAppControl.hpp"

using std::string;


bool CAppControl::Init(int a_nArgument, char* a_szArgument[])
{
	m_strErrorMsg.clear();
	
	if (a_nArgument < 6) {
		m_strErrorMsg = "invalied arguemnt count";
		return false;
	}
	
	m_strPkgName = a_szArgument[1];
	m_strSvcName = a_szArgument[2];
	m_strNodeType = a_szArgument[3];
	m_strNodeName = a_szArgument[4];
	m_strProcName = a_szArgument[5];
	
	return true;
}

const char* CAppControl::GetPkgName(string* a_strPkgName)
{
	if (a_strPkgName == NULL) {
		return m_strPkgName.c_str();
	}
	
	*a_strPkgName = m_strPkgName;
	return a_strPkgName->c_str();
}

const char* CAppControl::GetSvcName(string* a_strSvcName)
{
	if (a_strSvcName == NULL) {
		return m_strSvcName.c_str();
	}
	
	*a_strSvcName = m_strSvcName;
	return a_strSvcName->c_str();
}

const char* CAppControl::GetNodeType(string* a_strNodeType)
{
	if (a_strNodeType == NULL) {
		return m_strNodeType.c_str();
	}
	
	*a_strNodeType = m_strNodeType;
	return a_strNodeType->c_str();
}

const char* CAppControl::GetNodeName(string* a_strNodeName)
{
	if (a_strNodeName == NULL) {
		return m_strNodeName.c_str();
	}
	
	*a_strNodeName = m_strNodeName;
	return a_strNodeName->c_str();
}

const char* CAppControl::GetProcName(string* a_strProcName)
{
	if (a_strProcName == NULL) {
		return m_strProcName.c_str();
	}
	
	*a_strProcName = m_strProcName;
	return a_strProcName->c_str();
}

int CAppControl::GetNodeNo(void)
{
	int nNodeNo = 0;
	
	if (getenv("ATOM_NODENO") != NULL) {
		nNodeNo = atoi(getenv("ATOM_NODENO"));
	}

	return nNodeNo;
}

int CAppControl::GetProcNo(void)
{
	int nProcNo = 0;
	
	if (getenv("ATOM_PROCNO") != NULL) {
		nProcNo = atoi(getenv("ATOM_PROCNO"));
	}

	return nProcNo;
}

int CAppControl::Request(string& a_strBody, int *a_pnOpt)
{
	int nCtl = ATOM_CMD_APP_UNKNOWN;

	if (a_strBody.empty()) {
		m_strErrorMsg = "empty body message";
		return nCtl;
	}

	string strAction;
	int nOption = 0;

    try {
		rabbit::document docRoot;

		docRoot.parse(a_strBody);

		strAction = docRoot["BODY"]["action"].as_string();
		if (docRoot["BODY"].has("option")) {
			nOption = docRoot["BODY"]["option"].as_int();
		}
	
	} catch(rabbit::type_mismatch& e) {
		m_strErrorMsg = "message, invalied type mismatch";
		return nCtl;
	} catch(rabbit::parse_error& e) {
		m_strErrorMsg = "message, parsing failed";
		return nCtl;
	} catch(...) {
		m_strErrorMsg = "message, parsing failed";
		return nCtl;
	}

	if (strAction.compare("INIT") == 0) {
		nCtl = ATOM_CMD_APP_INIT;
	} else if (strAction.compare("SUSPEND") == 0) {
		nCtl = ATOM_CMD_APP_SUSPEND;
	} else if (strAction.compare("RESUME") == 0) {
		nCtl = ATOM_CMD_APP_RESUME;
	} else if (strAction.compare("LOGLEVEL") == 0) {
		nCtl = ATOM_CMD_APP_LOGLEVEL;
		*a_pnOpt = nOption;
#if 0
		switch (nOption) {
			// TODO
			// Log Class와 level을 맞춰야 할 것 같은데...
			default:
				break;
		}
#endif
	} else {
		nCtl = ATOM_CMD_APP_UNKNOWN;
		m_strErrorMsg = "invalied action type";
	}
	
	return nCtl;
}

std::string	CAppControl::Response(bool a_bIsSuccess, const char* a_szText)
{
	rabbit::object  docRoot;

	rabbit::object  docBody = docRoot["BODY"];
	if (a_bIsSuccess) {
		docBody["code"] = 0;
	} else {
		docBody["code"] = 1;
	}

	docBody["text"] = a_szText;

	return docRoot.str();
}
