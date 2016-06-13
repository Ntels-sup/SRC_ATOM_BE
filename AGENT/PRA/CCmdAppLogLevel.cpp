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

#include "CCmdAppLogLevel.hpp"

using std::pair;
using std::string;
 
extern CFileLog* g_pclsLogPra;


/* CMD_APP_LOGLEVEL
 * - Notify
	{
	   "BODY": {
	     "all": false,
	     "process" : [
	           {"procno": 1, "procname": "APP01"},
	           {"procno": 2, "procname": "APP02"},
	           {"procno": 3, "procname": "APP03"}
	     ],
	     "loglevel": 3
	   }
	}
 */
bool CCmdAppLogLevel::NotifyParse(const char* a_szJson)
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

		m_nLogLevel = docRoot["BODY"]["loglevel"].as_int();

	} catch(rabbit::type_mismatch e) {
		g_pclsLogPra->ERROR("'APP_LOGLEVEL',, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error e) {
		g_pclsLogPra->ERROR("'APP_LOGLEVEL', parsing failed");
		return false;
	} catch(...) {
		g_pclsLogPra->ERROR("'APP_LOGLEVEL', parsing failed");
		return false;
	}
	
	return true;
}