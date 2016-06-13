/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdBatchStart.cpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstdio>
#include <iostream>

#include "CFileLog.hpp"

#include "CCmdBatchStart.hpp"

using std::string;

// global valiable
extern CFileLog* g_pclsLogPra;


/* CMD_BAT_START
 * - Request
	{
	   "BODY" : {
	        "group_name" : "groupNm",
	        "job_name" : "JobNm",
	        "pkg_name" : "ATOM",
	        "proc_no" : 36,
	        "prc_date" : "20160420121230",
	        "exec_bin" : "ATOM_BJM",
	        "exec_arg" : "arg1 arg2 arg3",
	        "exec_env" : "arg1 arg2 arg3"
	   }
	}
 *
 * - Response 
	{
	   "BODY" : {
	        "end_date" : "20160420141230",
	        "exit_cd" : 9999,
	        "status" : "normal/pending/failure",
	        "success" : true/false
	   }
	}
 */

bool CCmdBatchStart::RequestParse(const char* a_szJson)
{
	if (a_szJson == NULL) {
		return false;
	}

    try {
		rabbit::document docRoot;
		docRoot.parse(a_szJson);

		m_strGroupName	= docRoot["BODY"]["group_name"].as_string();
		m_strJobName	= docRoot["BODY"]["job_name"].as_string();
		m_strPkgName	= docRoot["BODY"]["pkg_name"].as_string();
		m_nProcNo		= docRoot["BODY"]["proc_no"].as_int();
		m_strPrcDate	= docRoot["BODY"]["prc_date"].as_string();
		m_strExecBin	= docRoot["BODY"]["exec_bin"].as_string();
		m_strExecArg	= docRoot["BODY"]["exec_arg"].as_string();
		m_strExecEnv	= docRoot["BODY"]["exec_env"].as_string();
	
	} catch(rabbit::type_mismatch& e) {
		g_pclsLogPra->ERROR("CMD 'BATCH_START', %s", e.what());
		return false;
	} catch(rabbit::parse_error& e) {
		g_pclsLogPra->ERROR("CMD 'BATCH_START', %s", e.what());
		return false;
	} catch(...) {
		g_pclsLogPra->ERROR("CMD 'BATCH_START', unknown error");
		return false;
	}

	return true;
}

string CCmdBatchStart::ResponseGen(void)
{
	rabbit::object  docRoot;
	rabbit::object  docBody = docRoot["BODY"];

	struct timeval stTval;
	gettimeofday(&stTval, NULL);
	struct tm stTm;	
	localtime_r(&stTval.tv_sec, &stTm);
	char szTime[40] = {0x00,};
	snprintf(szTime, sizeof(szTime), "%04d%02d%02d%02d%02d%02d", 
									stTm.tm_year + 1900, stTm.tm_mon+1, stTm.tm_mday,
									stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
	docBody["end_date"]	= szTime;
	docBody["exit_cd"]	= m_nExitCd;
	docBody["status"]	= m_strStatus;
	
	return docRoot.str();
}
