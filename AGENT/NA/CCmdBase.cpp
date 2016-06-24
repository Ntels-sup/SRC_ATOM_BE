/* vim:ts=4:sw=4
 */
/**
 * \file	NAMain.cpp
 * \brief	Node Agent 메인 function
 *			Agent thread 및 function을 초기화 하고 실행
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include "CFileLog.hpp"

#include "CCmdBase.hpp"
 
extern CFileLog* g_pclsLog;

std::string CCmdBase::ErrorGen(int a_nCode, const char* a_szText)
{
	rabbit::object  root;

	rabbit::object  body = root["BODY"];
	body["code"] = a_nCode;
	body["text"] = a_szText;

	return root.str();
}
 	
bool CCmdBase::ErrorParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document doc;
		doc.parse(a_szJson);

		m_nErrorCode = doc["BODY"]["code"].as_int();
		m_strErrorText = doc["BODY"]["text"].as_string();	
		
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMDERROR,, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMDERROR, parsing failed");
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMDERROR, parsing failed");
		return false;
	}

	return true;	
}
