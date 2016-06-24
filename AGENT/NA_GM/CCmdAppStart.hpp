/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppStart.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDAPPSTART_HPP_
#define CCMDAPPSTART_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdAppStart : public CCmdBase
{
public:
	// Request
	bool		m_bAll;

	// Response
	std::string	m_strWorstStatus;

public:
	CCmdAppStart() {
		m_bAll = true;
		return;
	}
	~CCmdAppStart() {}
	
	std::string	RequestGen(void);
	bool		ResponseParse(const char* a_szJson);

private:

};

#endif // CCMDAPPSTART_HPP_
