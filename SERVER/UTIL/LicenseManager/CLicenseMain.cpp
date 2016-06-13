#include "CLicenseMain.hpp"

CLicenseMain::CLicenseMain()
{
	memset(m_szPkgName		,0x00,sizeof(m_szPkgName		));
	memset(m_szHostName		,0x00,sizeof(m_szHostName		));
	memset(m_szLicenseSort	,0x00,sizeof(m_szLicenseSort	));
	memset(m_szMaxValue		,0x00,sizeof(m_szMaxValue		));
	memset(m_szLicenseFile	,0x00,sizeof(m_szLicenseFile	));
	memset(m_uszLicenseTxt	,0x00,sizeof(m_uszLicenseTxt	));

	m_bDisplay 		= false;

}

CLicenseMain::~CLicenseMain()
{
	
}
bool CLicenseMain::
ParsingArgument(int a_nArgc, char** a_szArgv)
{
	int nNextOption = 0;
	
	const char *const szShortOptions = "hdp:l:o:m:f:t:";
	const struct option stLongOptions[] = 
	{
		{ "package",           	1,  NULL,   'p' },
		{ "license",       		1,  NULL,   'l' },
		{ "host", 		        1,  NULL,   'o' },
		{ "help", 		        0,  NULL,   'h' },
		{ "max", 		        1,  NULL,   'm' },
		{ "display", 		    0,  NULL,   'd' },
		{ "file", 		        1,  NULL,   'f' },
		{ "text", 		        1,  NULL,   't' },
		{ NULL, 	            0,  NULL,    0  }
	};	
	
	do
	{
		nNextOption = getopt_long(a_nArgc, a_szArgv,szShortOptions,stLongOptions,NULL);
#ifdef LM_DEBUG
		printf("nNextOption [%d]\n",nNextOption);
#endif
		switch(nNextOption)
		{

		case 'p':
			strncpy(m_szPkgName,optarg,LEN_LM_PKGNAME);
			break;
		case 'l':
			strncpy(m_szLicenseSort,optarg,LEN_LM_LICENSE);
			break;
		case 'o':
			strncpy(m_szHostName,optarg,LEN_LM_HOSTNAME);
			break;
		case 'h':
			Usage(a_szArgv[0]);
			exit(0);	
		case 'm':
			strncpy(m_szMaxValue,optarg,LEN_LM_TEMP);
			break;
		case 'd':
			m_bDisplay = true;
			break;
		case 'f':
			strncpy(m_szLicenseFile,optarg,LEN_LM_FILE_PATH);	
			break;
		case 't':
			strncpy((char *)m_uszLicenseTxt,optarg,MAX_LICENSE);	
			break;
		case -1:		// Option end
			break;
		default:
			return false;		
		}
	} while( -1 != nNextOption);
	
	if(true == m_bDisplay )
	{
		if( 0x00 == m_szLicenseFile[0] && 0x00 == m_uszLicenseTxt[0] )
		{
			printf("Display option require license file or license text\n");
			return false;
		}
		return true;
	}	
	
	if(0x00 != m_uszLicenseTxt[0] )
	{
		printf("Make option not require license text\n");
		return false;
	}
	
	if( 0 == atol(m_szMaxValue))
   	{
   		printf("The license require Max value\n");
   		return false;
   	}
   	if(0x00 == m_szPkgName[0] ||
	   0x00 == m_szHostName[0] ||
	   0x00 == m_szLicenseSort[0] )
	{
		printf("-- Option missing \n");
		return false;
	}
	
	if( 0 == memcmp(m_szLicenseSort, LICENSE_TRIAL, strlen(LICENSE_TRIAL)))
	{
		if( strlen(m_szMaxValue) != strlen(LICENSE_DATE_FMT))
		{
			printf(" -- Date format length invalid [%s]\n", m_szMaxValue);
			return false;
		}
		if(false == m_clsLicense.DateCheck(m_szMaxValue,(char *)LICENSE_DATE_FMT))
		{
			printf("-- Trial License date format invalid [%s]\n",m_szMaxValue);
			return false;
		}
	}
		
	return true;
}

bool CLicenseMain::
Run()
{
	
	int nFd		=0;
	int nSize   =0;
	int i		=0;
	char strTemp[10];
	unsigned char	uszLicenseCipher	[MAX_LICENSE+1	];
	
	memset(uszLicenseCipher,0x00,sizeof(uszLicenseCipher	));
	
	if(true == m_bDisplay)
	{	
		if( 0x00 !=m_szLicenseFile[0] )
		{
			nFd = open(m_szLicenseFile,O_RDONLY);
			
			if(-1 == nFd)
			{
				printf("License file open failed [%s]\n",m_szLicenseFile);
				return false;
			}
			nSize = (int)read(nFd,uszLicenseCipher,sizeof(uszLicenseCipher));
			if( -1 == nSize || 16 >= nSize)
			{
				printf("Licesne file read failed file[%s] size[%d]\n",m_szLicenseFile,nSize);
				return false;
			}
			close(nFd);
			
			if( false == m_clsLicense.SetAsciiLicense(uszLicenseCipher,nSize,false))
			{
				printf("License decrypt failed file[%s], size [%d]\n", m_szLicenseFile, nSize);
				return false;
			}
		}
		else
		{
			for(i=0; i< (int)strlen((char *)m_uszLicenseTxt); i++)
			{
				// Lower case convert to upper case
				if(m_uszLicenseTxt[i] >='a' && m_uszLicenseTxt[i] <= 'f')
					m_uszLicenseTxt[i] = (unsigned char)(m_uszLicenseTxt[i] - 32) ;
			}
			if( false == m_clsLicense.SetAsciiLicense(m_uszLicenseTxt,(int)strlen((char *)m_uszLicenseTxt),true))
			{
				printf("License decrypt failed src[%s], size [%d]\n", m_uszLicenseTxt, (int)strlen((char *)m_uszLicenseTxt));
				return false;
			}
		}
		m_clsLicense.Display();
		
		return true;
	}

	int nRet = m_clsLicense.Make(m_szPkgName, m_szLicenseSort, m_szHostName, m_szMaxValue,uszLicenseCipher);
	
	if(-1 == nRet)
	{
		printf("Encrypt failed Package name[%s], License sort[%s], Host name [%s], Max value[%s]\n",m_szPkgName,m_szLicenseSort, m_szHostName, m_szMaxValue);
		return false;
	}
	if(0x00 != m_szLicenseFile[0])
	{
		nFd = open(m_szLicenseFile,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	
		if(-1 == nFd)
		{
			printf("License file write failed [%s]\n",m_szLicenseFile);
			return false;
		}
		
		int nWriteSize = write(nFd,uszLicenseCipher,nRet);
		if( -1 == nWriteSize || nWriteSize != nRet)
		{
			printf("Licesne file write failed file[%s] will write size[%d] wrriten size[%d]\n",m_szLicenseFile,nRet, nWriteSize);
			return false;
		}
		close(nFd);
	}
	
	for(i=0; i < nRet; i++)
	{
		sprintf(strTemp,"%02X",(unsigned char)uszLicenseCipher[i]);
		strcat((char *)m_uszLicenseTxt,strTemp);
	}
	printf("\n\n*  License: %s                                                                          =\n",m_uszLicenseTxt);	
	printf("=========================================================================================\n\n");
	
	return true;
}

void CLicenseMain::
Usage(char* a_szPrcName)
{
	printf("NAME																  \n");
	printf("  %s - This utility manage licenses for ntels package   \n\n",a_szPrcName);
	printf("SYNOPSIS                                                              \n");
	printf("  %s [OPTIONS]                                          \n\n",a_szPrcName);
	printf("DESCRIPTION				     										  \n");
	printf("  %s manage license file for ntels package                \n\n",a_szPrcName);
	printf("OPTIONS                                                               \n");
	printf("  -p, --package                                                       \n");
	printf("    Package name.                                                     \n");
	printf("    [OFCS|OCS|PCRF]                                                   \n");
	printf("  -l, --license                                                       \n");
	printf("    [CML|TBL|TRL]                                                     \n");
	printf("    CML: Commercial license                                           \n");
	printf("    TBL: Testbed license                                              \n");
	printf("    TRL: Trial license                                                \n");
	printf("  -o, --host                                                          \n");
	printf("    Hostname will be deployed package                                 \n"); 
	printf("  -h, --help                                                          \n");
	printf("    Print the usage and exit                                          \n");
	printf("  -m, --max                                                           \n");
	printf("    Expired date or Max performance per day                           \n");
	printf("  -d, --display                                                       \n");       
	printf("    Display license information                                       \n"); 
	printf("  -f, --file                                                          \n");
	printf("    License file name                                                 \n");
	printf("  -t, --text                                                          \n");
	printf("    License ASCII text                                              \n\n");
	printf("EXAMPLES                                                              \n");
	printf("  %s -p OFCS -l CML -o littdove_host -m 2000000000 -f saved-license.txt \n", a_szPrcName);
	printf("    Make ofcs commerical license, license will be save the saved-license.txt file \n");
	printf("  %s --display --file saved-license.txt                  \n", a_szPrcName);
	printf("    View saved-license.txt license information                        \n");  
	printf("  %s --display --text \"74FF13C739B7BA6EA1E8B33D60B8B99F4E7A88C928101E3B3345B7B2384EE463AD70FA401B5AC41FD2A07FA9362E916A\"\n", a_szPrcName);
	printf("    View saved-license.txt license information                        \n");
	printf("\n");
	
	
}
void sigCapture( int a_nSigNum )
{
	printf("SIGNAL :: Process End!!! : %d\n",a_nSigNum);
	exit(-1);
}

int main(int argc, char* argv[])
{
	static struct  sigaction act, act2;
	act.sa_handler  =   SIG_IGN;   
	act2.sa_handler =   sigCapture  ;

	sigaction(SIGPIPE,  &act , NULL);
	sigaction(SIGUSR1,  &act2, NULL);
	sigaction(SIGALRM , &act2, NULL);

	printf("=========================================================================================\n\n");
	printf("                         ATOM License Management Utility V1.00                             \n");
	printf("             Copyright (c) 2016, nTels Corporation.  All rights reserved                 \n\n");
	printf("=========================================================================================\n\n");

	CLicenseMain clsMain;

	if (argc < 2)
	{
		clsMain.Usage(argv[0]);
		return -1;
	}

	if( false == clsMain.ParsingArgument(argc, argv))
	{
		printf("Argument parsing failed\n");
		clsMain.Usage(argv[0]);
		return -1;
	}
	if( false == clsMain.Run())
	{
		return -1;
	}
	
	return 0;	
	
}