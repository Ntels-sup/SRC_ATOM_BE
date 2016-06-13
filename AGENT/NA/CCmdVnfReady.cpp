#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"

#include "CCmdVnfReady.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;


/* CMD_VNF_READY
 * - Request
   {"BODY" : {
        "pkg_name" : "vOFCS",
        "node_type" : "LB",
        "ip" : "192.168.1.154",
        "prc_date" : "2016-04-30 16:00:45",
        "dst_yn" : "Y"
       }
   }
 *
 * - Response 
   {"BODY" : {
        "result" : "OK",
        "reason" : " ",
        "prc_date" : "2016-04-30 16:00:45",
        "dst_yn" : "Y"
        }
    }
 */

bool CCmdVnfReady::RequestParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strPkgName	= docRoot["BODY"]["pkg_name"].as_string();
		m_strNodeType	= docRoot["BODY"]["node_type"].as_string();
		m_strIp			= docRoot["BODY"]["ip"].as_string();
		m_strPrcDate	= docRoot["BODY"]["prc_date"].as_string();
		m_strDstYN		= docRoot["BODY"]["dst_yn"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLog->ERROR("CMD 'VNF_READY', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLog->ERROR("CMD 'VNF_READY', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLog->ERROR("CMD 'VNF_READY', unknown error");
		return false;
	}

	return true;
}

string CCmdVnfReady::ResponseGen(void)
{
	rabbit::object  docRoot;

	rabbit::object  docBody = docRoot["BODY"];
	if (m_bResult) {
		docBody["result"]	= "OK";
	} else {
		docBody["result"]	= "ERROR";
	}
	docBody["reason"]	= " ";
	
	struct timeval stTval;
	gettimeofday(&stTval, NULL);
	struct tm stTm;	
	localtime_r(&stTval.tv_sec, &stTm);
	char szTime[40] = {0x00,};
	snprintf(szTime, sizeof(szTime), "%04d-%02d-%02d %02d:%02d:%02d.%ld", 
									stTm.tm_year + 1900, stTm.tm_mon+1, stTm.tm_mday,
									stTm.tm_hour, stTm.tm_min, stTm.tm_sec,
									stTval.tv_usec / 10000);
	docBody["prc_date"]	= szTime;
	if (stTm.tm_isdst > 0) {
		docBody["dst_yn"]	= "Y";
	} else {
		docBody["dst_yn"]	= "N";
	}
	
	return docRoot.str();
}
