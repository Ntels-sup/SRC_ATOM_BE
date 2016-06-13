#ifndef __CCLACONFIG_HPP__
#define __CCLACONFIG_HPP__

#include "MariaDB.hpp"
#include "CCliRoute.hpp"

class CClaConfig{
	private:
		DB *m_cDb;
	public:
		CClaConfig(DB *a_cDb, int &nRet);
		~CClaConfig();
		int DBLoadCmdDest(CCliRoute *cCliRoute);
};

#endif
