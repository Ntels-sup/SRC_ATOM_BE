/* vim:ts=4:sw=4
 */

/**
 * \file	CDBInOut.hpp
 * \brief	
 *
 * $Author: junls&ntels.com $
 * $Date: $
 * $Id: $
 */

#include <string>
#include <vector>

#include "MariaDB.hpp"


class CDBInOut
{
public:
	int			m_nNodeNo;
	std::string	m_strNodeName;
	int			m_nProcNo;
	
	struct ST_GUINODE {
		std::string	m_strNodeName;
		int			m_nNodeGrpId;
		int			m_nImageNo;
	};
	
public:
	CDBInOut();
	~CDBInOut();
	
	int		FindNodeNo(std::string& a_strUuid);
	int		FindNodeNo(std::string& a_strPkgName, std::string& a_strNodeType);
	int		FindProcNo(std::string& a_strPkgName, std::string& a_strNodeType,
												std::string& a_strProcName);
	int		NodeCreate(std::string& a_strPkgName, std::string& a_strNodeType,
						std::string& a_strUuid, std::string& a_strIp);
	bool	NodeUse(std::string& a_strUuid, char a_cUsed = 'Y');
	int		NodeName(std::string& a_strPkgName, std::string& a_strNodeType,
									std::vector<ST_GUINODE>& a_vecNodeName);

private:
	bool	DBOpen();
	void	DBClose();
	
	DB*		m_pclsDB;
};
