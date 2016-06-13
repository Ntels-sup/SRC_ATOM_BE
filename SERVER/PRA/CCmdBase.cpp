/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdBase.cpp
 * \brief	
 *			
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <map>

#include "CFileLog.hpp"

#include "CCmdBase.hpp"

using std::pair;
using std::string;
 
extern CFileLog* g_pclsLogPra;


CCmdBase::CCmdBase()
{
	m_nErrorCode = 0;
	m_bIsAll = false;
	return;
}

/* CMD_PROC_START, STOP, INIT, SUSPEND, RESUME
 * - Request
	{
	   "BODY": {
	     "all": false,
	     "process" : [
	           {"procno": 1, "procname": "APP01"},
	           {"procno": 2, "procname": "APP02"},
	           {"procno": 3, "procname": "APP03"}
	     ]
	   }
	}
 */
bool CCmdBase::RequestParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_bIsAll = docRoot["BODY"]["all"].as_bool();
		if (m_bIsAll) {
			return true;
		}
		rabbit::array docProcess = docRoot["BODY"]["process"];
		
		pair<int, string> t;
		rabbit::value v;

		for (size_t i=0; i < docProcess.size(); i++) {
			v = docProcess.at(i);
			t.first = v["procno"].as_int();
			t.second = v["procname"].as_string();
			
			m_lstTarget.push_back(t);
		}

	} catch(rabbit::type_mismatch e) {
		g_pclsLogPra->ERROR("'PROC_STATUS',, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error e) {
		g_pclsLogPra->ERROR("'PROC_STATUS', parsing failed");
		return false;
	} catch(...) {
		g_pclsLogPra->ERROR("'PROC_STATUS', parsing failed");
		return false;
	}
	
	return true;
}

bool CCmdBase::NotifyParse(const char* a_szJson)
{
	return RequestParse(a_szJson);
}

/* ERROR
 *
	{"BODY": {
		"code": 0,
		"text": "ok"
		}
	}
 */

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
		
	} catch(rabbit::type_mismatch   e) {
		g_pclsLogPra->ERROR("CMDERROR,, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error e) {
		g_pclsLogPra->ERROR("CMDERROR, parsing failed");
		return false;
	} catch(...) {
		g_pclsLogPra->ERROR("CMDERROR, parsing failed");
		return false;
	}

	return true;	
}
 	
/* CMD_PROC_START, STOP, INIT, SUSPEND, RESUME
 * - Response 
	{
	  "BODY": {
	      "worst_status": "RUNNING",
	      "process_status": [
	          {"procno": 1, "procname": "APP01", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
	          {"procno": 2, "procname": "APP02", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
	          {"procno": 3, "procname": "APP03", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"}
	      ]
	   }
    }
 */
string CCmdBase::ResponseGen(void)
{
	rabbit::object docRoot;
	rabbit::object docBody = docRoot["BODY"];
	docBody["worst_status"] = m_strWorstStatus;
	
	rabbit::array docProcess = docBody["process_status"];
	rabbit::object o[m_lstResponse.size()];
	int i = 0;
	for (auto iter = m_lstResponse.begin(); iter != m_lstResponse.end(); ++iter, i++) {
	    o[i]["procno"]		= iter->m_nProcNo;
	    o[i]["procname"]	= iter->m_strProcName;
	    o[i]["status"]		= iter->m_strStatus;
	    o[i]["startdate"]	= iter->m_strStartDate;
	    o[i]["stopdate"]	= iter->m_strStopDate;
	    o[i]["version"]	= iter->m_strVersion;

		docProcess.push_back(o[i]);
	}

	return docRoot.str();
}

string CCmdBase::ErrorGen(int a_nCode, const char* a_szText)
{
	rabbit::object  docRoot;

	rabbit::object  docBody = docRoot["BODY"];
	docBody["code"] = a_nCode;
	docBody["text"] = a_szText;

	return docRoot.str();
}
