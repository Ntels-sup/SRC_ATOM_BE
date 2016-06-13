/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdBase.hpp
 * \brief	실시간 메세지 공통 interface
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDBASE_HPP_
#define CCMDBASE_HPP_

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>

using namespace rapidjson;

class CCmdBase
{
public:
	// Error
	int			m_nErrorCode;
	std::string m_strErrorText;

	// Request
	bool		m_bIsAll;
	std::list<std::pair<int, std::string> > m_lstTarget; // first: procno, second: procname
	
	// Response
	struct ST_PROCESS {
		int			m_nProcNo;
		std::string	m_strProcName;
		std::string	m_strStatus;
		std::string	m_strStartDate;
		std::string	m_strStopDate;
		std::string	m_strVersion;
	};
	struct ST_RESPONSE {
		std::string	m_strSvcName;
		std::string	m_strWorstStatus;		// 서비스 내의 프로세스중 worst 상태
		std::vector<ST_PROCESS>	m_vecProcess;
	};
	
	std::string		m_strWorstStatus;		// 전체 프로세스 중 worst 상태
	std::map<int, ST_RESPONSE> m_mapResponse;	// key: service no

	
public:
	CCmdBase();
	~CCmdBase() {};

	virtual bool		RequestParse(const char* a_szJson);
	virtual bool		ResponseParse(const char* a_szJson) { return true; }
	virtual bool		NotifyParse(const char* a_szJson);
	bool				ErrorParse(const char* a_szJson);

	virtual std::string	RequestGen(void) { return std::string(); }
	virtual std::string	NotifyGen(void) { return std::string(); }
	virtual std::string ResponseGen(void);
	std::string			ErrorGen(int a_nCode, const char* a_szText);
};

#endif // CCMDBASE_HPP_
