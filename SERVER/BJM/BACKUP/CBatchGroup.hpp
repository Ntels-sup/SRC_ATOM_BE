#ifndef __CBATCHGROUP_HPP__
#define __CBATCHGROUP_HPP__

#include "BJM_Define.hpp"
#include <list>
#include <string>
#include <map>

#define BJ_BATCH_GROUP_LAST "999999"

class CBatchGroup
{

public:

    CBatchGroup();
    ~CBatchGroup();

    bool    				Init(DB * a_pDB);
	int						LoadGroupInfo(ST_BatchGroup  *a_batchGroups);
	char* 					GetGroupName(char * a_strJobName);
	int						GroupUpdate();
	int						GroupInsert();

private:
	DB * 					m_pDB;

};

#endif // __CBATCHGROUP_HPP__
