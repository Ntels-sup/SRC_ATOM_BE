#ifndef  _CLICENSE_MAIN_HPP
#define  _CLICENSE_MAIN_HPP

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <vector>
#include <errno.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <signal.h>
#include <cstring>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::string;
using namespace std;

#include <getopt.h>
#include "CLicense.hpp"

const char* const 	LICENSE_DATE_FMT ="YYYYMMDD";

class CLicenseMain {
	private :
	char	m_szPkgName			[LEN_LM_PKGNAME+1	];
	char	m_szHostName		[LEN_LM_HOSTNAME+1	];
	char 	m_szLicenseSort		[LEN_LM_LICENSE+1	];
	char	m_szMaxValue		[LEN_LM_TEMP +1		];
	char	m_szLicenseFile		[LEN_LM_FILE_PATH+1	];
	
	unsigned char	m_uszLicenseTxt	[MAX_LICENSE+1	];

	
	bool	m_bDisplay;
	
	CLicense	m_clsLicense;
	
	public:
		CLicenseMain();
		~CLicenseMain();

		bool    Run();
		bool    ParsingArgument(int , char** );
		void    Usage(char *);
};

#endif