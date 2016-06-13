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
 
#include "CFileLog.hpp"

#include "CCmdBase.hpp"

using std::pair;
using std::string;
using std::vector;
using std::map;
 
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
		g_pclsLogPra->ERROR("'PROC_STATUS', invalied type mismatch");
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
	return CCmdBase::RequestParse(a_szJson);
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
	      "service": [
              {
                 "svc_no": 1,
                 "svc_name": "IMS",
	             "worst_status": "RUNNING",
    	         "process_status": [
       	             {"procno": 1, "procname": "APP01", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
       	             {"procno": 2, "procname": "APP02", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
       	             {"procno": 3, "procname": "APP03", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"}
	             ]
	          },
              {
                 "svc_no": 2,
                 "svc_name": "SMS",
	             "worst_status": "RUNNING",
    	         "process_status": [
       	             {"procno": 1, "procname": "APP01", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
       	             {"procno": 2, "procname": "APP02", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
       	             {"procno": 3, "procname": "APP03", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"}
	             ]
	          }
	      ]
       }
    }
 */
string CCmdBase::ResponseGen(void)
{
	rabbit::object docRoot;
	rabbit::object docBody = docRoot["BODY"];
	docBody["worst_status"] = m_strWorstStatus;
	
	rabbit::array docService = docBody["service"];
	
	rabbit::object* rs = new rabbit::object[m_mapResponse.size()];
	//process counting 
	int nCount = 0;
	for (auto miter = m_mapResponse.begin(); miter != m_mapResponse.end(); ++miter) {
		nCount += miter->second.m_vecProcess.size();
	}
	rabbit::object* rp = new rabbit::object[nCount];
	
	int i = 0, j = 0;
	for (auto miter = m_mapResponse.begin(); miter != m_mapResponse.end(); ++miter, ++i) {
		rs[i]["svc_no"] = miter->first;
		rs[i]["svc_name"] = miter->second.m_strSvcName;
		rs[i]["worst_status"] = miter->second.m_strWorstStatus;
		
		rabbit::array docProcess = rs[i]["process_status"];

		auto viter = miter->second.m_vecProcess.begin();
		for (; viter != miter->second.m_vecProcess.end(); ++viter, ++j) {
			rp[j]["procno"]		= viter->m_nProcNo;
			rp[j]["procname"]	= viter->m_strProcName;
			rp[j]["status"]		= viter->m_strStatus;
			rp[j]["startdate"]	= viter->m_strStartDate;
			rp[j]["stopdate"]	= viter->m_strStopDate;
			rp[j]["version"]	= viter->m_strVersion;

			docProcess.push_back(rp[j]);
		}

		docService.push_back(rs[i]);
	}

	string strBody = docRoot.str();

	delete[] rs;
	delete[] rp;

	return strBody;
}

string CCmdBase::ErrorGen(int a_nCode, const char* a_szText)
{
	rabbit::object  docRoot;

	rabbit::object  docBody = docRoot["BODY"];
	docBody["code"] = a_nCode;
	docBody["text"] = a_szText;

	return docRoot.str();
}
