#include "CLicense.hpp"

bool CLicense::
SetAsciiLicense(unsigned char* a_uszLicenseCipher ,int a_nSize ,bool a_bAscii)
{	
	if( a_bAscii ) 
	{		
		a_nSize = AsciiToHex((char *)a_uszLicenseCipher,(char *)m_uszLicenseCipher);
#ifdef LM_DEBUG	
		printf("\nBefore Vaule[%s]\n",a_uszLicenseCipher);
				 
		printf("\nConverting Result[");
		for(i=0; i < a_nSize; i++)
		{
			printf("%02X",(unsigned char)m_uszLicenseCipher[i]);
		}
		printf("]\n");
#endif
	}
	else
		memcpy(m_uszLicenseCipher, a_uszLicenseCipher, a_nSize);
	
	if( -1 == m_clsAES.DecryptAll(m_uszLicenseTxt,m_uszLicenseCipher,a_nSize,(unsigned char*)DFN_LICENSE_KEY))
		return false;
#ifdef LM_DEBUG	
	printf("Decrypt[%s]\n",m_uszLicenseTxt);
#endif
	return Parsing();
}
bool CLicense::
Parsing()
{
	
	char*   pStr = NULL;
	char*  ppStr = NULL;
	
	int 	i	 = 0;
	
	
	
	pStr = strtok_r((char *)m_uszLicenseTxt,DFN_LICENSE_DELIM, &ppStr);
	
	while(pStr)
	{
		
		switch(i)
		{
		case 0:
			strncpy(m_szPkgName,pStr,LEN_LM_PKGNAME);
			break;
			
		case 1:
			strncpy(m_szLicenseSort,pStr,LEN_LM_LICENSE);
			if(	0 == strncmp(m_szLicenseSort,LICENSE_COMMERICIAL,LEN_LM_LICENSE) ||
   				0 == strncmp(m_szLicenseSort,LICENSE_TESTBED,LEN_LM_LICENSE))
   				m_bPerfLicense = true;
   			else if(0 == strncmp(m_szLicenseSort,LICENSE_TRIAL,LEN_LM_LICENSE) )
   				m_bPerfLicense = false;
   			else
   				return false;
			break;
			
		case 2:
			strncpy(m_szHostName,pStr,LEN_LM_HOSTNAME);
			break;
			
		case 3:
			strncpy(m_szMaxValue,pStr,LEN_LM_TEMP);
			m_lMaxValue = atol(m_szMaxValue);
			
			break;
		}
		pStr = strtok_r(NULL,DFN_LICENSE_DELIM, &ppStr);
		i++;
	}
	
	if(i != 4)
		return false;
		
	return true;
}

int CLicense::
Make(char* a_szPkgName,char* a_szLicenseSort,char* a_szHostName,char* a_szMaxValue,unsigned char* a_szCipherResult)
{
	int nCipherSize	=0;
		
	sprintf((char *)m_uszLicenseTxt,"%s%s%s%s%s%s%s",a_szPkgName,DFN_LICENSE_DELIM,a_szLicenseSort,
		DFN_LICENSE_DELIM,a_szHostName,DFN_LICENSE_DELIM,a_szMaxValue);

	nCipherSize = m_clsAES.EncryptAll(m_uszLicenseTxt,a_szCipherResult,(unsigned char*)DFN_LICENSE_KEY);
	
	return nCipherSize;
}
void CLicense::
Display()
{
	
	printf("*  Pakcage: %s                                                                         \n",m_szPkgName);
	printf("*  License: %s                                                                         \n",m_szLicenseSort);
	if(	0 == strncmp(GetLicenseSort(),LICENSE_COMMERICIAL,LEN_LM_LICENSE))
    	printf("*           Commercial license                                                     \n");
    else if( 0 == strncmp(GetLicenseSort(),LICENSE_TESTBED,LEN_LM_LICENSE))
    	printf("*           Testbed license                                                        \n");
    else
    	printf("*           Trial license                                                          \n");
	printf("*  Host   : %s                                                                         \n",m_szHostName);
	
	if( m_bPerfLicense)
		printf("*  Max performance per day: %s                                                     \n",m_szMaxValue);
	else
		printf("*  Expired date: %s                                                                \n",m_szMaxValue);		
	printf("=========================================================================================\n\n");
	
}
int CLicense::
AsciiToHex(char *a_szAscii, char *a_szHex)
{
   int i = 0;
   int j = 0;
   int nOddFlag = 0;    
   int nOutLength = strlen(a_szAscii);
   char cNum =0;    
   
   if(strlen(a_szAscii)%2 > 0 ) nOutLength++; 

	for ( i= 0; i < (int)strlen(a_szAscii) ; i++ ) 
	{   
		if ( ! nOddFlag ) 
			cNum = 0x00;
	
		if ( *(a_szAscii+i) >= 'A' && *(a_szAscii+i) <= 'F' ) 
			cNum = cNum | ( *(a_szAscii+i)-'A'+10);
		else if (*(a_szAscii+i) == ' ')	// ' ' -> F
			cNum = cNum |  15;
		else 
			cNum = cNum |  ( *(a_szAscii+i)-'0'+0x00);
	
		if ( !nOddFlag  ) 
		{
			cNum = cNum << 4;
			nOddFlag = 1;
		} 
		else 
		{
			*(a_szHex+j) = cNum;
			j++;
			nOddFlag = 0;
		}
	}
	
	if( nOddFlag )	// Odd로 끝났을 경우 Even을 'F'로 채워준다.
	{
		cNum = cNum | 15;
		*(a_szHex+j) = cNum;
		j++;
	}
	
	*(a_szHex+j) = '\0';  
	return j;    
}

bool CLicense::DateCheck(char *a_szDay, char *a_szFormat)
{
	int		i, nLen;
	int		Y, M, D, h, m, s, c;  
	char	sTemp[10];
	char 	szDateTime[LEN_LM_DATE+1];
	
	
	memset(szDateTime, 0x00, sizeof(szDateTime));
	;
	Y=M=D=h=m=s=c=0;
	
	nLen = strlen(a_szFormat);
	
	
	for(i=0; i<nLen; i++)
	{
		switch(a_szFormat[i])
		{
			case 'Y':
				szDateTime[Y++]    = a_szDay[i];
				break;
			case 'M':
				szDateTime[4+M++]  = a_szDay[i];
				break;
			case 'D':
				szDateTime[6+D++]  = a_szDay[i];
				break;
			case 'h':
				szDateTime[8+h++]  = a_szDay[i];
				break;
			case 'm':
				szDateTime[10+m++]  = a_szDay[i];
				break;
			case 's':
				szDateTime[12+s++] = a_szDay[i];
				break;
			case 'c':
				szDateTime[14+c++] = a_szDay[i];
				break;
		}
	}	
	
	// 년도체크 
	if(Y)
	{
		sprintf(sTemp, "%.4s", szDateTime);	
		i = atoi(sTemp);
		if     (Y==4 && i>9999) return false;
		else if(Y==3 && i>999 ) return false; 
		else if(Y==2 && i>99  ) return false;
		else if(i==0  ) 		return false;
	}
	
	
	// 월체크 
	if(M)
	{
		sprintf(sTemp, "%.2s", (char *)&szDateTime[4]);	
		i = atoi(sTemp);
		if((i>12 || i==0)) return false;
	}
	
	// 일체크 
	if(D)
	{	
		sprintf(sTemp, "%.2s", (char *)&szDateTime[6]);	
		i = atoi(sTemp);
		if((i>31 || i==0)) return false;
	}
	
	// 시체크 
	if(h)
	{
		sprintf(sTemp, "%.2s", (char *)&szDateTime[8]);	
		i = atoi(sTemp);
		if((i>23 || i<0)) return false;
	}
	
	// 분체크
	if(m)
	{ 
		sprintf(sTemp, "%.2s", (char *)&szDateTime[10]);	
		i = atoi(sTemp);
		if((i>59 || i<0)) return false;
	}
	
	// 초체크
	if(s)
	{ 
		sprintf(sTemp, "%.2s", (char *)&szDateTime[12]);	
		i = atoi(sTemp);
		if((i>59 || i<0)) return false;
	}
	
	return true;	
}

bool CLicense::
IsExpired()
{
	struct 	timeb	stTimeb;
	struct 	tm   	stTm;
	char	szTemp[LEN_LM_DATE];
	
	ftime(&stTimeb);
	memcpy((char *)&stTm,  (char *)localtime_r( &stTimeb.time, &stTm ), sizeof(struct tm)); 
	sprintf(szTemp,"%04d%02d%02d", stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday);
	
	if( strncmp(szTemp,m_szMaxValue,strlen(m_szMaxValue)) <= 0)
		return false;
	
	return true;
}