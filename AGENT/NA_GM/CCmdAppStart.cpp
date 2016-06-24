#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdAppStart.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_APP_START, CMD_APP_STOP, CMD_APP_KILL, CMD_APP_RESTART °øÅë
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
 *
 * - Response 
	{
	  "BODY": {
	      "worst_status": "STOPPED",
	      "process_status": [
	          {"procno": 1, "procname": "APP01", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
	          {"procno": 2, "procname": "APP02", "status": "STOPPED", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"},
	          {"procno": 3, "procname": "APP03", "status": "RUNNING", "startdate": "2016-03-04 13:00:00", "stopdate": "2016-03-04 14:00:00", "version": "3.2.1"}
	      ]
	  }
	}
 */

string CCmdAppStart::RequestGen(void)
{
	rabbit::object  docRoot;
	rabbit::object  docBody = docRoot["BODY"];
	docBody["all"]	= m_bAll;

	return docRoot.str();
}

bool CCmdAppStart::ResponseParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strWorstStatus = docRoot["BODY"]["worst_status"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMDAPPSTART, invalied type mismatch");
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMDAPPSTART, parsing failed");
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMDAPPSTART, parsing failed");
		return false;
	}
	
	return true;
}
