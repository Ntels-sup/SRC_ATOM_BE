/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppLogLevel.hpp
 * \brief	APP 로그레벨 변경 메세지
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDAPPLOGLEVEL_HPP_
#define CCMDAPPLOGLEVEL_HPP_

#include <iostream>
#include <string>
#include <list>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CCmdBase.hpp"

using namespace rapidjson;


class CCmdAppLogLevel : public CCmdBase
{
public:
	// Request
	bool		m_bIsAll;
	std::list<std::pair<int, std::string> > m_lstTarget; // first: procno, second: procname
	int			m_nLogLevel;
	
public:
	CCmdAppLogLevel();
	~CCmdAppLogLevel() {};

	bool		NotifyParse(const char* a_szJson);
};

#endif // CCMDAPPLOGLEVEL_HPP_
