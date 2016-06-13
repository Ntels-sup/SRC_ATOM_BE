#ifndef  _CLICENSE_HPP
#define  _CLICENSE_HPP
#include <time.h>
#include <sys/timeb.h>
#include <getopt.h>
#include <stdlib.h>

#include "CAESCrypt.hpp"



const unsigned char* const 	DFN_LICENSE_KEY 	=(unsigned char *)"NTELS-FENICE-^^*";
const char* const 			DFN_LICENSE_DELIM	=",";

const int 			MAX_LICENSE 		= 1024;
const int 			LEN_LM_TEMP    		= 100;

const int 			LEN_LM_PKGNAME		= 5;
const int 			LEN_LM_HOSTNAME		= 50;
const int 			LEN_LM_LICENSE		= 3;
const int 			LEN_LM_FILE_PATH	= 200;
const int			LEN_LM_DATE			= 20;	//'YYYYMMDDhhmmssccc'

const char* const 	LICENSE_COMMERICIAL	="CML";
const char* const 	LICENSE_TESTBED		="TBL";
const char* const 	LICENSE_TRIAL		="TRL";

class CLicense {
	private :
		char			m_szPkgName			[LEN_LM_PKGNAME+1	];
		char			m_szHostName		[LEN_LM_HOSTNAME+1	];
		char 			m_szLicenseSort		[LEN_LM_LICENSE+1	];
		char			m_szMaxValue		[LEN_LM_TEMP +1		];
		unsigned char	m_uszLicenseTxt		[MAX_LICENSE+1		];
		unsigned char	m_uszLicenseCipher	[MAX_LICENSE+1		];
		unsigned long	m_lMaxValue;
		
		bool	m_bPerfLicense;
		bool	m_bLicenseOver;
		
		CAESCrypt m_clsAES;
  
	public:
		CLicense()
		{
			memset(m_szPkgName		,0x00,sizeof(m_szPkgName		));
			memset(m_szHostName		,0x00,sizeof(m_szHostName		));
			memset(m_szLicenseSort	,0x00,sizeof(m_szLicenseSort	));
			memset(m_szMaxValue		,0x00,sizeof(m_szMaxValue		));
			memset(m_uszLicenseTxt	,0x00,sizeof(m_uszLicenseTxt	));
			memset(m_uszLicenseCipher,0x00,sizeof(m_uszLicenseCipher));

			m_lMaxValue	   = 0;
	
			m_bPerfLicense = false;
			m_bLicenseOver = false;
		}
		~CLicense() { }

		bool	SetAsciiLicense(unsigned char*,int ,bool);	// bool: true -> ASCII, false -> BINARY
		
		// Before Get~ call, first SetAsciiLicense call
		char* 	GetPkgName() 		{ return m_szPkgName;	 }
		char*	GetHostName()		{ return m_szHostName;	 }
		char*	GetLicenseSort()	{ return m_szLicenseSort;}
		char* 	GetLimitValue()		{ return m_szMaxValue;	 }
		bool	IsPerfLicense()  	{ return m_bPerfLicense; }
		bool 	IsPerfOver(unsigned long a_lNowValue)
		{
			return m_lMaxValue < a_lNowValue;
	
		}
		bool	IsExpired();
		
		
		
		int 	Make(char* ,char* ,char* ,char *,unsigned char*);
		void	Display();
		int 	AsciiToHex(char* , char*);
		bool	Parsing();
		bool	DateCheck(char* , char*);
		
};

#endif