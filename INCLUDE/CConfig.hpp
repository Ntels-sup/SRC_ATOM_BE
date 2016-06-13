#ifndef __CCONFIG_HPP__
#define __CCONFIG_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#define DEF_MEM_BUF_128 128
#define DEF_MEM_BUF_256 256
#define DEF_MEM_BUF_512 512
#define DEF_MEM_BUF_1024 1024

#define E_CONF_GET_ENV      5001
#define E_CONF_FILE_OPEN    5002
#define E_CONF_PARSING      5003
#define E_CONF_INSERT       5004
#define E_CONF_UPDATE       5005
#define E_CONF_LOCK      5006

#define DEF_STR_COMMENT 0
#define DEF_STR_CONFIG  1

typedef struct _config_info
{
    std::vector<std::string> vecComment;
    std::string strValue;
}CONFIG_INFO;

class CConfig
{
	private:
        std::string m_strFilePath;
		std::map< std::string, std::map<std::string, CONFIG_INFO*> > m_mapConfig;
		
		int ParsingConfig(const char *a_szCfgFile, int a_nTimeOut);
		char *InsertGroup(char *a_szBuffer);
		int ParsingAttr(char *a_szBuffer, char **a_pszAttrName, char **a_pszAttrValue);
		char *DeleteSpace(char *a_szBuffer, int a_nLen);

	public:
		CConfig();
		~CConfig();
		int Initialize(char *a_szCfgFile = NULL, int a_nTimeOut = 3);
		const char* GetGlobalConfigValue(const char *a_szName);
		const char* GetConfigValue(const char *a_szGroup, const char *a_szName);
        int InsertConfigValue(const char *a_szGroup, const char *a_szName, const char *a_szValue);
        int UpdateConfigValue(const char *a_szGroup, const char *a_szName, const char *a_szValue);
        int ReWriteConfig(int a_nTimeOut = 1);
        void DumpConfig();		
};

#endif

