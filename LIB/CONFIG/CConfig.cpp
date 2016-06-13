#include "CConfig.hpp"

CConfig::CConfig()
{
	m_mapConfig.clear();
	m_strFilePath.clear();
}


CConfig::~CConfig()
{
	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;
	std::map< std::string, CONFIG_INFO* >::iterator itAttr;

	for( itGroup = m_mapConfig.begin(); itGroup != m_mapConfig.end(); ++itGroup)
	{
		for( itAttr = itGroup->second.begin(); itAttr != itGroup->second.end(); ++itAttr)
		{
			if( NULL != itAttr->second )
			{
				delete itAttr->second;
			}
		}
		itGroup->second.clear();
	}

	m_mapConfig.clear();
}

int CConfig::Initialize(char *a_szCfgFile, int a_nTimeOut)
{
    int ret = 0;
	char szPathBuffer[PATH_MAX];
	char *pCfgPath = NULL;

	if( NULL == a_szCfgFile )
	{
		pCfgPath = getenv("ATOM_CFG_HOME");
		if( NULL == pCfgPath )
		{
			perror("Failed to Get Config Path\n");
			return -E_CONF_GET_ENV;
		}

		snprintf(szPathBuffer, sizeof(szPathBuffer), "%s", pCfgPath);
	}
	else
	{
		snprintf(szPathBuffer, sizeof(szPathBuffer), "%s", a_szCfgFile);

	}
	m_strFilePath.assign(szPathBuffer);

	if( (ret = ParsingConfig(m_strFilePath.c_str(), a_nTimeOut)) < 0)
		return ret;

	return 0;
}

int CConfig::ParsingConfig(const char *a_szCfgFile, int a_nTimeOut)
{
	char szBuffer[DEF_MEM_BUF_1024];
	FILE *fp = NULL;
	int fd = 0;

	char *pStr;
	char *pGroup;
	char *pAttrName;
	char *pAttrValue;

    int ret = 0;

	time_t tStart;
	time_t tCur;
	struct flock stWriteLock = { F_WRLCK, SEEK_SET, 0, 0 };
	struct flock stUnLock = { F_UNLCK, SEEK_SET, 0, 0 };

	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;	
	std::map<std::string, CONFIG_INFO*>::iterator itAttr;
    std::string strComment;

    CONFIG_INFO *pstConfig;


	fp = fopen(a_szCfgFile, "r+");
	if ( NULL == fp )
	{
		return -E_CONF_FILE_OPEN;
	}

	fd = fileno(fp);

	time(&tStart);
	while( (ret = fcntl(fd, F_SETLK, &stWriteLock)) < 0)
	{
		time(&tCur);
		if( tCur > (tStart + a_nTimeOut) )
		{
			fclose(fp);
			return -E_CONF_LOCK;
		}

		usleep(3000);
	}

    pstConfig = NULL;

	while( NULL != fgets( szBuffer, sizeof(szBuffer), fp ) )
	{

		pStr = DeleteSpace(szBuffer, strlen(szBuffer));

		if( NULL != pStr )
		{
			if( '[' == pStr[0] )
			{
				if(pstConfig)
					pstConfig->vecComment.clear();

				pGroup = InsertGroup(pStr);	
				itGroup = m_mapConfig.find(pGroup);

				if(itGroup == m_mapConfig.end())
				{
					fcntl(fd, F_SETLK, &stUnLock);
					fclose(fp);
					return -E_CONF_PARSING;
				}

				itGroup->second.clear();
			}
			else
			{
				ret = ParsingAttr(pStr, &pAttrName, &pAttrValue);	

                if( DEF_STR_CONFIG == ret )
                {
                    if( NULL == pstConfig )
                    {
                        pstConfig = new CONFIG_INFO;
                        pstConfig->vecComment.clear();                    
                    }

                    pstConfig->strValue.assign(pAttrValue);

    				itGroup->second.insert( std::pair<std::string, CONFIG_INFO*>(pAttrName, pstConfig));
        			pstConfig = NULL;
                }
                else
                {
                    if( NULL == pstConfig )
                    {
                        pstConfig = new CONFIG_INFO;
						pstConfig->vecComment.clear();
                    }

					strComment.assign(&szBuffer[1]);
                    pstConfig->vecComment.push_back(strComment);
                }


			}
		}		
	}

	fcntl(fd, F_SETLK, &stUnLock);
	fclose(fp);

	return 0;
}

char *CConfig::InsertGroup(char *a_szBuffer)
{
	static char szGroupBuffer[DEF_MEM_BUF_1024];
	char *p = a_szBuffer;
	int nIdx = 0;
	std::map<std::string, CONFIG_INFO*> mapTemp;

	while(*p)
	{
		if( (*p != '[') && (*p != ']') )
		{
			szGroupBuffer[nIdx++] = *p;
		}

		p++;
	}
		
	szGroupBuffer[nIdx] = 0x00;

	mapTemp.clear();
	m_mapConfig.insert(std::pair<std::string, std::map<std::string, CONFIG_INFO*> >(szGroupBuffer, mapTemp));

	return szGroupBuffer;
}

int CConfig::ParsingAttr(char *a_szBuffer, char **a_pszAttrName, char **a_pszAttrValue)
{
	char *p = NULL;

    if( '#' == a_szBuffer[0] )
	{
		return DEF_STR_COMMENT;
	}

	p = strtok ( a_szBuffer, "=" );
	if(p)
		*a_pszAttrName = p;

	p = strtok ( NULL, "=" );
	if(p)
		*a_pszAttrValue = p;

	return DEF_STR_CONFIG;
}

char *CConfig::DeleteSpace(char *a_szBuffer, int a_nLen)
{
	static char szSpaceBuffer[DEF_MEM_BUF_1024];
	char *p = a_szBuffer;
	int nIdx = 0;

	for(int i = 0; i < a_nLen ; i++, p++)
	{
		if( '\n' == *p || 0x00 == *p )
			break;

		if( 0x20 == *p)
			continue;

		szSpaceBuffer[nIdx++] = *p;
	}

	szSpaceBuffer[nIdx] = 0x00;
	
	if( strlen(szSpaceBuffer) )
		return szSpaceBuffer;
	else
		return NULL;
	return 0;
}

const char *CConfig::GetGlobalConfigValue(const char *a_szName)
{
	return GetConfigValue("GLOBAL", a_szName);
}

const char *CConfig::GetConfigValue(const char *a_szGroup, const char *a_szName)
{
	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;
	std::map< std::string, CONFIG_INFO* >::iterator itAttr;

	itGroup = m_mapConfig.find(a_szGroup);
	if(itGroup != m_mapConfig.end())
	{
		itAttr = itGroup->second.find(a_szName);
		if(itAttr != itGroup->second.end())
			return itAttr->second->strValue.c_str();
	}

	return NULL;
}

int CConfig::InsertConfigValue(const char *a_szGroup, const char *a_szName, const char *a_szValue)
{
	std::pair<std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator, bool> retGroup;
	std::pair<std::map< std::string, CONFIG_INFO* >::iterator, bool> retAttr;
	std::map<std::string, CONFIG_INFO*> mapTemp;
	CONFIG_INFO* pstConfig = NULL;

	mapTemp.clear();
	retGroup = m_mapConfig.insert(std::pair<std::string, std::map<std::string, CONFIG_INFO*> >(a_szGroup, mapTemp));

	pstConfig = new CONFIG_INFO;
	pstConfig->vecComment.clear();
	pstConfig->strValue.assign(a_szValue);

	if( true == retGroup.second )
	{

		retAttr = retGroup.first->second.insert(std::pair<std::string, CONFIG_INFO*>(a_szName, pstConfig));
	}
	else
	{
		retAttr = retGroup.first->second.insert(std::pair<std::string, CONFIG_INFO*>(a_szName, pstConfig));
		if( false == retAttr.second )
		{
			delete pstConfig;
			return -E_CONF_INSERT;
		}
	}

	return 0;
}

int CConfig::UpdateConfigValue(const char *a_szGroup, const char *a_szName, const char *a_szValue)
{
	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;
	std::map< std::string, CONFIG_INFO* >::iterator itAttr;
	CONFIG_INFO* pstConfig = NULL;

	itGroup = m_mapConfig.find(a_szGroup);

	if( itGroup == m_mapConfig.end() )
		return -E_CONF_UPDATE;

	itAttr = itGroup->second.find(a_szName);

	if( itAttr == itGroup->second.end() )
		return -E_CONF_UPDATE;

	pstConfig = itAttr->second;

	pstConfig->strValue.assign(a_szValue);

	return 0;

}

int CConfig::ReWriteConfig(int a_nTimeOut)
{
	int ret = 0;
	time_t tStart;
	time_t tCur;
	struct flock stWriteLock = { F_WRLCK, SEEK_SET, 0, 0 };
	struct flock stUnLock = { F_UNLCK, SEEK_SET, 0, 0 };

	FILE *fp = NULL;
	int fd = 0;

	fp = fopen(m_strFilePath.c_str(), "r+");
	if ( NULL == fp )
	{
		return -E_CONF_FILE_OPEN;
	}

	fd = fileno(fp);

	time(&tStart);
	while( (ret = fcntl(fd, F_SETLK, &stWriteLock)) < 0)
	{
		time(&tCur);
		if( tCur > (tStart + a_nTimeOut) )
		{
			fclose(fp);
			return -E_CONF_LOCK;
		}

		usleep(3000);
	}

	truncate(m_strFilePath.c_str(), 0);

	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;
	std::map< std::string, CONFIG_INFO* >::iterator itAttr;
	
	for( itGroup = m_mapConfig.begin(); itGroup != m_mapConfig.end(); ++itGroup)
	{
		fprintf(fp, "[%s]\n", itGroup->first.c_str());
		for( itAttr = itGroup->second.begin(); itAttr != itGroup->second.end(); ++itAttr)
		{
			for(uint16_t i = 0; i < itAttr->second->vecComment.size(); i++)
			{
				fprintf(fp, "#%s", itAttr->second->vecComment[i].c_str());
			}
			
			fprintf(fp, "%s = %s\n", itAttr->first.c_str(), itAttr->second->strValue.c_str());
		}
		fprintf(fp, "\n\n");
	}
	
	char szBuff[DEF_MEM_BUF_128];
	struct tm *pstCur;
	pstCur = localtime ( &tStart );
	strftime(szBuff, sizeof(szBuff), "%Y-%m-%d %H:%M:%S", pstCur);

	fprintf(fp, "#Rewrite Date %s\n", szBuff);		
	fcntl(fd, F_SETLK, &stUnLock);

	fclose(fp);
	return 0;	
}


void CConfig::DumpConfig()
{
	std::map< std::string, std::map<std::string, CONFIG_INFO*> >::iterator itGroup;
	std::map< std::string, CONFIG_INFO* >::iterator itAttr;
	
	for( itGroup = m_mapConfig.begin(); itGroup != m_mapConfig.end(); ++itGroup)
	{
		printf("Group [ %-8s] ======================\n", itGroup->first.c_str());
		for( itAttr = itGroup->second.begin(); itAttr != itGroup->second.end(); ++itAttr)
		{
			printf("    Attr : %s\n", itAttr->first.c_str());
			for(uint16_t i = 0; i < itAttr->second->vecComment.size(); i++)
			{
				if( 0 == i )
					printf("    Comment : %s", itAttr->second->vecComment[i].c_str());
				else
					printf("              %s", itAttr->second->vecComment[i].c_str());
			}
			
			printf("    Value : %s\n", itAttr->second->strValue.c_str());
			printf(" --------------------------------------\n");
		}
		printf("\n");
	}
}
