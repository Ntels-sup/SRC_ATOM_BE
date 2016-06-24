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

using namespace rapidjson;

/* ERROR
 *
	{"BODY": {
		"code": 0,
		"text": "ok"
		}
	}
 */

class CCmdBase
{
public:
	int			m_nErrorCode;
	std::string m_strErrorText;
	
public:
	CCmdBase() { m_nErrorCode = 0; }
	~CCmdBase() {};

	virtual std::string	RequestGen() { return std::string(); }
	virtual std::string	NotifyGen() { return std::string(); }
	virtual std::string ResponseGen() { return std::string(); }
	std::string			ErrorGen(int a_nCode, const char* a_szText);
	
	virtual bool		RequestParse(const char* a_szJson) { return true; }
	virtual bool		ResponseParse(const char* a_szJson) { return true; }
	virtual bool		NotifyParse(const char* a_szJson) { return true; }
	bool				ErrorParse(const char* a_szJson);
};

#endif // CCMDBASE_HPP_
