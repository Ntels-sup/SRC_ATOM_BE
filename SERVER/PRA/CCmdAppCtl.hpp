/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppCtl.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDAPPCTL_HPP
#define CCMDAPPCTL_HPP

#include <string>

#include "CCmdBase.hpp"


class CCmdAppCtl : public CCmdBase
{
public:
	// Request
	std::string	m_strAction;
	int			m_nOption;
		
	// Response

public:
	CCmdAppCtl() {
		m_nOption = 0;
		return;
	}
	~CCmdAppCtl() {}
	
	std::string	RequestGen(void);

private:

};

#endif // CCMDAPPCTL_HPP
