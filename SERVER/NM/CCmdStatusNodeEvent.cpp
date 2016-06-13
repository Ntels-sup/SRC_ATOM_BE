#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"
#include "CCmdStatusNodeEvent.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLog;

/* CMD_STATUS_NODE_EVENT
 * - Notify
   {
		"BODY": {
			"message": "node status change",
			"pkg_name": "OFCS",
			"node_no"": 1,
			"node_name": " ",
			"node_version": "OFCS1.2",
			"node_type": "AP",
			"node_status": "ACTIVE",
			"prc_date": "2016-04-04 09:14:53.92",
			"dst_yn": "N"
		}
	}
 */

string CCmdStatusNodeEvent::NotifyGen(void)
{
	rabbit::object docRoot;

	rabbit::object docBody = docRoot["BODY"];
	docBody["message"]		= m_strMessage;
	docBody["pkg_name"]		= m_strPkgName;
	docBody["node_no"]		= m_nNodeNo;
	docBody["node_name"]	= m_strNodeName;
	docBody["node_version"]	= m_strNodeVersion;
	docBody["node_type"]	= m_strNodeType;
	docBody["node_status"]	= m_strNodeStatus;

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
