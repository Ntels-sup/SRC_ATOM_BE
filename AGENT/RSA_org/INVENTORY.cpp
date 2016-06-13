#include "INVENTORY.hpp"

INVENTORY::INVENTORY()
{
	m_pmapRsc = NULL;
	m_pclsLog = NULL;
	m_pclsDB = NULL;
	m_pclsMain = NULL;

}


INVENTORY::~INVENTORY()
{
	RESOURCE_ATTR *pstAttr = NULL;
	map<string, RESOURCE_ATTR *>::iterator it;
	for(it = m_pmapRsc->begin(); it != m_pmapRsc->end() ; it++)
	{
		if(it->second != NULL)
		{
			pstAttr = it->second;
		}
		else
		{
			continue;
		}

		if(pstAttr->pData)
		delete (INVENTORY_VALUE*)pstAttr->pData;
	}

}

int INVENTORY::Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain)
{
	char *pszDBConnInfo[MAX_DB_CONN_INFO_IDX];
	memset(pszDBConnInfo, 0x00, sizeof(pszDBConnInfo));

	if(a_pclsLog == NULL)
		return -1;

	m_pclsLog = a_pclsLog;
	m_pGroupRsc = a_pRsc;
	m_pmapRsc = &(a_pRsc->mapRsc);	
	m_pclsMain = (RSARoot*)a_pclsMain;


	m_pclsDB = m_pclsMain->GetDBConn();
	if(m_pclsDB == NULL)
	{
		m_pclsLog->ERROR("DB Class is NULL");
		return -1;
	}

	m_pclsLog->INFO("Inventory Initialized"); 
	return 0;
}

int INVENTORY::MakeJson(time_t a_tCur)
{
	return 0;
}

int INVENTORY::GetCPUInfo()
{
	int ret = 0;
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];
	char szQuery[DEF_MEM_BUF_1024];

	snprintf(szBuffer, sizeof(szBuffer), "cat /proc/cpuinfo | grep 'model name' | tail -1 | sed -e 's;^.*:;;' | sed -e 's/^ *//g'");
	if( NULL == (fp = popen(szBuffer, "r")) )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}	

	if( NULL == fgets(szBuffer, sizeof(szBuffer), fp) )
	{
		m_pclsLog->ERROR("fgets() error (%s)", szBuffer);
		pclose(fp);
		return -1;
	}	

	szBuffer[strlen(szBuffer)-1] = 0x00;
	m_pclsLog->INFO("CPU Model : %s", szBuffer);

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_CPU, szBuffer);	

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert CPU Info ");
		pclose(fp);
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert CPU Info Failed [%d:%s]", szQuery, ret,  m_pclsDB->GetErrorMsg(ret));
		pclose(fp);
		return -1;
	}

	pclose(fp);
	return 0;
}


#if 0
int INVENTORY::GetDiskInfo()
{
	return 0;
}
#endif

int INVENTORY::GetKernelInfo()
{
	int ret = 0;
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];
	char szQuery[DEF_MEM_BUF_1024];

	snprintf(szBuffer, sizeof(szBuffer), "uname -r");
	if( NULL == (fp = popen(szBuffer, "r")) )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}	

	if( NULL == fgets(szBuffer, sizeof(szBuffer), fp) )
	{
		m_pclsLog->ERROR("fgets() error (%s)", szBuffer);
		pclose(fp);
		return -1;
	}	

	szBuffer[strlen(szBuffer)-1] = 0x00;
	m_pclsLog->INFO("Kernel Version : %s", szBuffer);

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_KERNEL, szBuffer);	

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert Kerenel Info ");
		pclose(fp);
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert Kernel Info Failed [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		pclose(fp);
		return -1;
	}

	pclose(fp);

	return 0;
}

int INVENTORY::GetMemoryInfo()
{
	int ret = 0;
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];
	char szQuery[DEF_MEM_BUF_1024];

	snprintf(szBuffer, sizeof(szBuffer), "cat /proc/meminfo | grep MemTotal | sed -e 's;^.*:;;' | sed -e 's/^ *//g'");
	if( NULL == (fp = popen(szBuffer, "r")) )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}	

	if( NULL == fgets(szBuffer, sizeof(szBuffer), fp) )
	{
		m_pclsLog->ERROR("fgets() error (%s)", szBuffer);
		pclose(fp);
		return -1;
	}	
	szBuffer[strlen(szBuffer)-1] = 0x00;
	m_pclsLog->INFO("Memory : %s", szBuffer);


	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_MEMORY, szBuffer);

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert Memory Info ");
		pclose(fp);
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert MEMORY Info Failed [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		pclose(fp);
		return -1;
	}
	pclose(fp);

	return 0;
}

int INVENTORY::GetDBInfo()
{
	int ret = 0;
	char szQuery[DEF_MEM_BUF_1024];

	char szTemp[DEF_MEM_BUF_128];
	char szBuffer[DEF_MEM_BUF_1024];
	char szVariableName[DEF_MEM_BUF_128];
	char szVariableValue[DEF_MEM_BUF_128];

	FetchMaria fData;
	fData.Set(szVariableName, sizeof(szVariableName));
	fData.Set(szVariableValue, sizeof(szVariableValue));

	sprintf(szBuffer, "show variables where Variable_name in ('innodb_version', 'version_comment', 'version_compile_machine','version_compile_os')");
	ret = m_pclsDB->Query(&fData, szBuffer, strlen(szBuffer));
	if(ret < 0)
	{
		m_pclsLog->ERROR("Fail to Query (%s) [%d:%s]", szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}


	memset(szBuffer, 0x00, sizeof(szBuffer));	
	while(true)
	{	
		if(fData.Fetch() == false)
		{
			break;
		}

		snprintf(szTemp, sizeof(szTemp), "%s ", szVariableValue);
		strcat(szBuffer, szTemp);	
		
	}

	m_pclsLog->INFO("DB Total String : %s", szBuffer);

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_MYSQL, szBuffer);	

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert DB Info ");
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert DB Info Failed [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}

	return 0;
}

int INVENTORY::GetOSInfo()
{
	int ret = 0;
	FILE *fp;
	char szBuffer[DEF_MEM_BUF_1024];
	char szQuery[DEF_MEM_BUF_1024];

	snprintf(szBuffer, sizeof(szBuffer), "cat /etc/issue | head -1");
	if( NULL == (fp = popen(szBuffer, "r")) )
	{
		m_pclsLog->ERROR("popen() error (%s)", szBuffer);
		return -1;
	}	

	if( NULL == fgets(szBuffer, sizeof(szBuffer), fp) )
	{
		m_pclsLog->ERROR("fgets() error (%s)", szBuffer);
		pclose(fp);
		return -1;
	}	
	szBuffer[strlen(szBuffer)-1] = 0x00;

	m_pclsLog->DEBUG("OS Version : %s", szBuffer);

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_OS, szBuffer);	

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert OS Info ");
		pclose(fp);
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert OS Info Failed [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		pclose(fp);
		return -1;
	}
	pclose(fp);

	return 0;
}

int INVENTORY::GetPythonInfo()
{
	int ret = 0;
	char szQuery[DEF_MEM_BUF_1024];

	m_pclsLog->DEBUG("Python Version : %s", PY_VERSION);

	memset(szQuery, 0x00, sizeof(szQuery));
	snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_PYTHON, PY_VERSION);	

	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Insert Python Info ");
		return -1;
	}

	if( (ret = m_pclsDB->Execute(szQuery, strlen(szQuery))) < 0)
	{
		m_pclsLog->ERROR("(%s) Insert Python Info Failed [%d:%s]", szQuery, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}

	return 0;
}

#if 0
int INVENTORY::GetIPInfo()
{
	FILE *fp;
	char szQuery[DEF_MEM_BUF_1024];
	char szBuffer[DEF_MEM_BUF_1024];

	struct ifreq ifr;
	struct if_nameindex* pidx;
	struct if_nameindex* head;
	int sockfd;

	struct sockaddr_in * ipaddr = NULL;

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		m_pclsLog->ERROR( "socket() error");
		return -1;
	}

	head = if_nameindex();
	for (pidx = head; (pidx != NULL) && (pidx->if_name != NULL); pidx++)
	{
		strncpy(ifr.ifr_name, pidx->if_name, IFNAMSIZ);

		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
		{
			m_pclsLog->ERROR( "ioctl(SIOCGIFFLAGS) error");
			continue;
		}

		if (ifr.ifr_flags & IFF_LOOPBACK)
		{
			m_pclsLog->INFO( "iface(%s) = loopback", pidx->if_name);
			continue;
		}

		if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
		{
			m_pclsLog->ERROR("ioctl(SIOCGIFADDR) error");
			continue;
		}

		ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
		
		m_pclsLog->INFO("NIC : %s, IP Address : %s", pidx->if_name, inet_ntoa(ipaddr->sin_addr));
		memset(szQuery, 0x00, sizeof(szQuery));
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO TAT_NODE_INFO VALUES('%d', '%d', sysdate(), '%s')", 
										m_pclsMain->GetNodeID(), DEF_COMMON_CCD_IP, inet_ntoa(ipaddr->sin_addr));	

		if(m_pclsDB->Execute(szQuery, strlen(szQuery)) < 0)
		{
			m_pclsLog->ERROR("(%s) Insert IP Info Failed [%d:%s]", szQuery, m_pclsDB->GetError(), m_pclsDB->GetErrorMsg());
			delete m_pclsDB;
			m_pclsDB = NULL;
			return -1;
		}
		
	}

	return 0;
}
#endif
int INVENTORY::Run()
{
	int ret = 0;
	char szBuffer[DEF_MEM_BUF_1024];
	
	if( NULL == m_pclsDB )
	{
		m_pclsLog->ERROR("Failed to Run Inventory ");
		return -1;
	}

	snprintf(szBuffer, sizeof(szBuffer), "DELETE FROM TAT_NODE_INFO WHERE NODE_NO='%d'", 	m_pclsMain->GetNodeID());
	if( (ret = m_pclsDB->Execute(szBuffer, strlen(szBuffer))) < 0)
	{
		m_pclsLog->ERROR("Query (%s) Failed [%d:%s]", szBuffer, ret, m_pclsDB->GetErrorMsg(ret));
		return -1;
	}

	if(GetDBInfo() < 0)
		return -1;
	if(GetCPUInfo() < 0)
		return -1;
	if(GetKernelInfo() < 0)
		return -1;
	if(GetMemoryInfo() < 0)
		return -1;
	if(GetOSInfo() < 0)
		return -1;
	if(GetPythonInfo() < 0)
		return -1;
//	if(GetIPInfo() < 0)
//		return -1;

	return 0;
}

extern "C"
{
	RSABase *PLUG0015_INVENTORY()
	{
		return new INVENTORY;
	}

}
