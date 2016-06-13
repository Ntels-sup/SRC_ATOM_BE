/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdAppInit.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDAPPINIT_HPP_
#define CCMDAPPINIT_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdAppInit : public CCmdBase
{
public:
	// Request
	
	// Response
	struct ST_RESPONSE {
		int			m_nProcNo;
		std::string	m_strProcName;
		bool		m_bSuccess;
	};
	std::list<ST_RESPONSE>	m_lstResponse;

public:
	CCmdAppInit() {}
	~CCmdAppInit() {}
	
	std::string	ResponseGen(void);
	
private:

};

#endif // CCMDAPPINIT_HPP_
