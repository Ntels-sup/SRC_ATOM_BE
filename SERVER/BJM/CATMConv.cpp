/*
 ***************************************************************************
 *
 * CATMConv.cpp -  nTels Library -  conversion clss.
 *
 ***************************************************************************
 */

#include "CATMConv.hpp"

/*****************************************************************************
* Function Name : GetBCDHex()
* Description   : char->Hex 
* Parameters    : char _ch : char value
* Return values : Hex value char
*****************************************************************************/
char CATMConv::GetBCDHex(char _ch)
{
	if(_ch >= '0' && _ch <= '9') 
	{
		return (_ch - '0');
    }
    else if(_ch >= 'A' && _ch <= 'F') 
    {
    	return (_ch - 'A' + 10);
    }
    else 
    {
    	return (_ch - 'a' + 10);
    }
}

/*****************************************************************************
* Function Name : GetBCDChar()
* Description   : Hex->Char
* Parameters    : char _hex : Hex value
* Return values : char value  char
*****************************************************************************/
char CATMConv::GetBCDChar(char _hex)
{
    if( _hex > 9)
        return _hex - 10 + 'a';
    else
        return _hex + '0';
}

/*****************************************************************************
* Function Name : Str2BCD()
* Description   : 
	ex) "1259" ==> 0x12 0x59
	    "123"  ==> 0x01 0x23
* Return values : int :
*****************************************************************************/
int CATMConv::Str2BCD(char *_dest, char* _src)
{
    char 	t;
    char 	b;
    int  	len  = strlen(_src);
    int 	i,x;
	
	if( (len % 2) != 0 )
	{
		t = _src[0];
	    _dest[0] = GetBCDHex(t);
	       
        for(i=1,x=1; i<len; i+=2, x++)
	    {
	        t = _src[i];
	        t = GetBCDHex(t);
	        
		    b = _src[i+1];
			b = GetBCDHex(b);
			
	        _dest[x] = t<<4 | b;
	    }
	}
	else
	{
	    for(i=0,x=0; i<len; i+=2, x++)
	    {
	        t = _src[i];
	        t = GetBCDHex(t);
	        
		    b = _src[i+1];
			b = GetBCDHex(b);
			
	        _dest[x] = t<<4 | b;
	    }
	}
	
	_dest[x] = 0x00;
		
    return x;
}

/*****************************************************************************
* Function Name : BCD2Str()
* Description   : BCD값을 string으로 변경하는 함수. _src의 값이 BCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
	ex) 0x12 0x53 ==> "1253"
	    0x03 0x26 ==> "0326"
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
				  int _len		: string value length.
* Return values : int : 변환한 결과의 길이.
*****************************************************************************/
void CATMConv::BCD2Str(char *_dest,char *_src,int _len)
{
    int y = 0;

    for(int i=0; i<_len; i++)
    {
        _dest[y++] = ((_src[i] >> 4) & 0x0f) + '0';
        _dest[y++] = (_src[i] & 0x0f) + '0';
    }

    _dest[y] = 0x00 ;
}

/*****************************************************************************
* Function Name : Str2Int()
* Description   : string을 interger로 변경해주는 함수
* Parameters    : char *_src 	: char value
				  int _len		: string length
* Return values : int : 변경된 interger
*****************************************************************************/
int CATMConv::Str2Int(char *_src,int _len)
{
    char str[32+1];
	char* pEnd;
	int nRet = 0;

	memset(str, 0x00, sizeof(str));

	if(_len > 0)
	{
	    strncpy(str,_src,_len) ;
    	str[_len] = 0;
	}

	nRet = strtol(str, &pEnd, 10);
//	nRet = atoi(str);
    return nRet;
}

/*****************************************************************************
* Function Name : Int2Str()
* Description   : interger를 string로 변경해주는 함수.
* Parameters    : int _num 	: interger
* Return values : char *	: 변경된 string
*****************************************************************************/
char *CATMConv::Int2Str(int _num)
{

#ifdef __HAVE_LTOA
    return ltoa((long)_num) ;
#else
    // linux ( 2.96 on IA64 archicter ) not support ltoa system call 
    // IBM-AIX not support ltoa system call 
    static char result[20] ;
    sprintf(result,"%d",_num) ;

    return result ;
#endif

}

/*****************************************************************************
* Function Name : GetTBCDHex()
* Description   : char값을 TBCD로 변경하는 함수. _ch의 값이 TBCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
* Parameters    : char _ch 	: char value
* Return values : int : TBCD value로 변경된 char
*****************************************************************************/
char CATMConv::GetTBCDHex(char _ch)
{
    if ( isdigit(_ch) != 0 )
        return _ch - '0';

    switch(_ch)
    {
        case '*' : return 10;
        case '#' : return 11;
        case 'a' : 
        case 'A' : 
        	return 12;
        case 'b' : 
        case 'B' : 
        	return 13;
        case 'c' : 
        case 'C' : 
        	return 14;
        case 'f' : 
        case 'F' : 
        	return 15; // filler
		default :
			return -1;
    }

    return -1 ;
}

/*****************************************************************************
* Function Name : GetTBCDChar()
* Description   : TBCD값을 TBCD char로 변경하는 함수. _hex의 값이 TBCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
* Parameters    : char _hex 	: TBCD value
* Return values : int : TBCD char로 변경된 char
*****************************************************************************/
char CATMConv::GetTBCDChar(char _hex)
{
    if(_hex >= 0 && _hex <= 9)
        return _hex + '0';

    switch(_hex)
    {
        case 10 : return '*';
        case 11 : return '#';
        case 12 : return 'a';
        case 13 : return 'b';
        case 14 : return 'c';
        case 15 : return 'f'; // filler
		default : return -1 ;
    }
    
    return -1 ;
}

/*****************************************************************************
* Function Name : Str2TBCD()
* Description   : String값을 BCD로 변경하는 함수. _src의 값이 BCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
	_src의 길이가 홀수인 경우 마지막 NIBBLE에 'f'를 채워준다.
	ex) "12ab" ==> 0x12 0xcd
	    "12*"  ==> 0x12 0xaf
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
* Return values : int : 변환한 결과의 길이.
*****************************************************************************/
int CATMConv::Str2TBCD(char *_dest, char *_src)
{
    char t;
    char b;
    int  len  = strlen(_src);
    int  i = 0,x = 0 ;

    for(i = 0, x = 0; i < len; i+=2, x++)
    {
        t = _src[i];
        t = GetTBCDHex(t);

        if( i + 1 == len && len%2 != 0)
        {
            b = 'f';
        }
        else
        {
            b = _src[i + 1];
        }

        b = GetTBCDHex(b);
		
		if( x > 0 )
	        _dest[x] = t << 4 | b;
    }
            
    _dest[x] = 0x00;
    
    return x;
}

/*****************************************************************************
* Function Name : TBCD2Str()
* Description   : TBCD값을 string으로 변경하는 함수. _src의 값이 TBCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
	_mkLen 보다 짧은 데이터는 뒷부분에 ' '를 채워준다.
	ex) 0x12 0x53 ==> "1253"
	    0xA3 0x26 ==> "*326"
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
				  int _len		: string value length.
				  int _mkLen	: 최종 결과로 나갈 길이. 
* Return values : int : 변환한 결과의 길이.
*****************************************************************************/
int CATMConv::TBCD2Str(char *_dest, char *_src, int _len,int _mkLen)
{
    int  y = 0;
    char t;
    char b;
    int  i;

    for(i=0; i<_len; i++)                             	
    {
        b = (_src[i] & 0xf0) >> 4;                      			
        b = GetTBCDChar(b);      
        _dest[y++] = b;                                         
                                                                
        t = (_src[i] & 0x0f);                           
                                                                
        if(t == 0xf)                                            
        {                                                       
            break;                                              
        }                                                       
                                                                
        t = GetTBCDChar(t);                                     
                                                                
        _dest[y++] = t;                                         
    }                                                           

    for(;y<_mkLen;y++)
        _dest[y] = ' ';

    return y;
}

/*****************************************************************************
* Function Name : TBCD2Str()
* Description   : TBCD값을 string으로 변경하는 함수. _src의 값이 TBCD범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
	ex) 0x12 0x53 ==> "1253"
	    0xA3 0x26 ==> "*326"
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
				  int _len		: string value length.
* Return values : int : 변환한 결과의 길이.
*****************************************************************************/
int CATMConv::TBCD2Str(char *_dest, char *_src, int _len)
{
    int  y = 0;
    char t;
    char b;

    for(int i=0; i<_len; i++)
    {
        b = (_src[i] & 0xf0) >> 4;
        
        b = GetTBCDChar(b);
        _dest[y++] = b;

        t = (_src[i] & 0x0f);

        if(t == 0xf)
        { 
            break;
        }

        t = GetTBCDChar(t);
        _dest[y++] = t;
    }
	
    _dest[y] = 0x00;
	
    return y;
}

void CATMConv::ChangeByteOrder(byte *_data, size_t _size)
{
    byte buffer[16] ;

    memcpy(buffer,_data,_size) ;

    for(int i=0;i<(int)_size;i++)
    {
	 *(_data+i) = buffer[_size-i-1] ;
    }
}

/*****************************************************************************
* Function Name : Str2Hex()
* Description   : String값을 Hex로 변경하는 함수. _src의 값이 hex범위에 있는
	정상적인 값이라고 가정하기 때문에 비정상적인 값이 들어오더라도 관계하지 않는다.
	ex) "12ab" ==> 0x12 0xab
	    "12D"  ==> 0x12 0xd0
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
* Return values : int : 변환한 결과의 길이.
*****************************************************************************/
int CATMConv::Str2Hex(char *_dest, char *_src)						
{                                                       
	char t;                                             
	char b;                                             
	int  len  = strlen(_src);                           
	int		x,i;                                        
	                                                    
	for(i=0, x=0; i<len; i+=2, x++)                     
	{    
		b 			= 0x00;
		                                               
		t = _src[i];                                    
		t = GetBCDHex(t);  
		
		if( i+1 <= len-1 )
		{                
			b = _src[i+1];                                  
			b = GetBCDHex(b);      
			_dest[x] = t<<4 | b; 
		}         
		else
		{   
			if( i == 0 )
			{
				_dest[x] = t; 
			}
			else
			{
				_dest[x] = t<<4 | b;  
			}
		}
	}                                  
	
	_dest[x] = 0x00;
	                                                
	return x;                                           
}                                                       

/*****************************************************************************
* Function Name : Hex2Str()
* Description   : Hex값을 string으로 변경하는 함수.
	ex) 0x12   ==> "12"
	    0xabc0 ==> "abc0"
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
				  int _len		: string value length.
* Return values : void
*****************************************************************************/                                                     
void CATMConv::Hex2Str(char *_dest, char *_src, int _len)		
{                                               
    int y = 0;                                  
    unsigned char n1, n2;                       
                                                
    for(int i=0; i<_len; i++)                   
    {                                           
    	n1 = ((_src[i] >> 4) & 0x0f);           
    	n2 = (_src[i] & 0x0f);                  
    	_dest[y++] = GetBCDChar(n1);               
    	_dest[y++] = GetBCDChar(n2);               
    }                                           
    
    _dest[y] = 0x00;
}

/*****************************************************************************
* Function Name : Hex2Str()
* Description   : Hex값을 string으로 변경하는 함수.
	ex) 0x12   ==> "12"
	    0xabc0 ==> "abc0"
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
				  int _len		: string value length.
				  int _mkLen	: 최종 결과로 나갈 길이. 
* Return values : void
*****************************************************************************/  
void CATMConv::Hex2Str(char *_dest, char *_src, int _len, int _mkLen)
{
    int y = 0;
    int k ;
    unsigned char n1, n2;

    for(int i=0; i<_len; i++)
    {
    	n1 = ((_src[i] >> 4) & 0x0f);
    	n2 = (_src[i] & 0x0f);
    	_dest[y++] = GetBCDChar(n1);
    	_dest[y++] = GetBCDChar(n2);
    }
    
    k = _mkLen - _len ;

    for(int j=0;j<k;j++)
    	_dest[y++] = ' ' ;
}

/*****************************************************************************
* Function Name : Str2GMTTime()
* Description   : 시간을 가리키는 string을 GMT 기준의 time_t형으로 변환해준다.
	ex) "20050317152032" ==> 2063862676
* Parameters    : char *_ptime 	: 시간을 가리키는 스트링.(YYYYMMDDhhmmss)
* Return values : time_t : 변환된 time_t
*****************************************************************************/
time_t CATMConv::Str2GMTTime(char *_ptime)
{
	int			nLen = 0;
	struct tm 	convtime ;
	
	nLen = strlen( _ptime );
	
	if( nLen != 14 )
	{
		return 0;
	}
	
	convtime.tm_year = Str2Int(_ptime+ 0,4)-1900 ;
	convtime.tm_mon  = Str2Int(_ptime+ 4,2)-1 ;
	convtime.tm_mday = Str2Int(_ptime+ 6,2) ;
	convtime.tm_hour = Str2Int(_ptime+ 8,2) ;
	convtime.tm_min  = Str2Int(_ptime+ 10,2) ;
	convtime.tm_sec  = Str2Int(_ptime+ 12,2) ;
	convtime.tm_isdst = 0 ;

	return timegm(&convtime);
}

/*****************************************************************************
* Function Name : Str2Time()
* Description   : 시간을 가리키는 string을 localtime 기준으로 time_t형으로 변환해준다.
	ex) "20050317152032" ==> 2063862676
* Parameters    : char *_ptime 	: 시간을 가리키는 스트링.(YYYYMMDDhhmmss)
* Return values : time_t : 변환된 time_t
*****************************************************************************/
time_t CATMConv::Str2Time(char *_ptime)
{
	int			nLen = 0;
	struct tm 	convtime ;
	
	nLen = strlen( _ptime );
	
	if( nLen != 14 )
	{
		return 0;
	}
	
	convtime.tm_year = Str2Int(_ptime+ 0,4)-1900 ;
	convtime.tm_mon  = Str2Int(_ptime+ 4,2)-1 ;
	convtime.tm_mday = Str2Int(_ptime+ 6,2) ;
	convtime.tm_hour = Str2Int(_ptime+ 8,2) ;
	convtime.tm_min  = Str2Int(_ptime+ 10,2) ;
	convtime.tm_sec  = Str2Int(_ptime+ 12,2) ;
	convtime.tm_isdst = -1 ;

	return mktime(&convtime);
}

/*****************************************************************************
* Function Name : Time2GMTStr()
* Description   : 시간을 가리키는 time_t형을 GMT 시간 스트링(YYYYMMDDhhmmss)으로 
	변환한다.
	ex) 2063862676 ==> "20050317152032" 
* Parameters    : char *_des 	: 변환된 시간이 저장될 buffer(YYYYMMDDhhmmss)
				  time_t _tTime	: 변환할 시간.
* Return values : void 
*****************************************************************************/
void CATMConv::Time2GMTStr(char *_dest,time_t _tTime)
{
	struct tm*		tmStruct;
    struct tm		t_result ;
	
    tmStruct = gmtime_r(&_tTime, &t_result);
            
	int nYear   = tmStruct->tm_year+1900;
	int nMonth  = tmStruct->tm_mon+1;
	int nDay    = tmStruct->tm_mday;
	int nHour	= tmStruct->tm_hour;
	int nMin	= tmStruct->tm_min;
	int nSec	= tmStruct->tm_sec;
	
	sprintf(_dest,"%04d%02d%02d%02d%02d%02d", nYear, nMonth, nDay, 
		nHour, nMin, nSec);
}

/*****************************************************************************
* Function Name : Time2Str()
* Description   : 시간을 가리키는 time_t형을 시간 스트링(YYYYMMDDhhmmss)으로 
	변환한다.
	ex) 2063862676 ==> "20050317152032" 
* Parameters    : char *_des 	: 변환된 시간이 저장될 buffer(YYYYMMDDhhmmss)
				  time_t _tTime	: 변환할 시간.
* Return values : void 
*****************************************************************************/
void CATMConv::Time2Str(char *_dest,time_t _tTime)
{
	struct tm*		tmStruct;
    struct tm		t_result ;

    tmStruct = localtime_r(&_tTime, &t_result);
            
	int nYear   = tmStruct->tm_year+1900;
	int nMonth  = tmStruct->tm_mon+1;
	int nDay    = tmStruct->tm_mday;
	int nHour	= tmStruct->tm_hour;
	int nMin	= tmStruct->tm_min;
	int nSec	= tmStruct->tm_sec;
	
	sprintf(_dest,"%04d%02d%02d%02d%02d%02d", nYear, nMonth, nDay, 
		nHour, nMin, nSec);
}

/*****************************************************************************
* Function Name : Str2Ipv4()
* Description   : String값을 IPv4 Type으로 변경하는 함수. 
	ex) "192.168.1.225" ==> 0xc0a801ff
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터
				  char *_src	: 변환할 string value
* Return values : true or false;
*****************************************************************************/
bool CATMConv::Str2Ipv4(char *_dest, char *_src)						
{                                                       
    uint32_t unIp ;
	
	unIp =  inet_addr(_src);
	
	if( unIp == 4294967295U )
	{
		return false;
	}
	else
	{
		memcpy( _dest, (char*)&unIp, sizeof(unsigned int) );
	}
	
	_dest[sizeof(unsigned int)] = 0x00;
	
	return true;
}

/*****************************************************************************
* Function Name : Ipv42Str()
* Description   : IPv4 Type을 string형 IP로 변환해 주는 함수.
	ex) 0xc0a801ff ==> "192.168.1.225"
* Parameters    : char *_dest 			: 변환한 결과가 저장될 포인터
				  unsigned int _unIp	: 변환할 unsigned int Type의 IP
* Return values : void
*****************************************************************************/
void CATMConv::Ipv42Str(char *_dest, unsigned int _unIp)						
{                                                       
	struct in_addr in_val;
	
	in_val.s_addr  = _unIp;
	
	strncpy( _dest, inet_ntoa( in_val ), strlen(inet_ntoa( in_val) ) );
}

/*****************************************************************************
* Function Name : MiddleNibbleToStr()
* Description   : 앞과 뒤의 nibble에 걸쳐있는 데이터를 정상적으로 옮겨준다.
	ex) 0xA1234B ==> 0x1234
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::MiddleNibbleToStr(char *_dest, char *_src, int _nLen )						
{         
	int		i;
	int		y = 0;
	char	cFirst;
	char	cSecond;
	
	for(i=0; i<_nLen-1; i++)
    {
    	cFirst = (_src[i] & 0x0f ) << 4;
    	cSecond = (_src[i+1] & 0xf0 ) >> 4;
    	
    	_dest[y] |= cFirst;
    	_dest[y++] |= cSecond;
    }
    
    _dest[y] = 0x00;
}

/*****************************************************************************
* Function Name : LastNibbleToStr()
* Description   : 맨 처음 바이트의 2번째 nibble부터 유효한 데이터를 입력받아
	유효하지 않은 첫번째 nibble에 0을 넣어준다.
	ex) 0xA12345 ==> 0x012345
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::LastNibbleToStr(char *_dest, char *_src, int _nLen )						
{         
	char	cFirst;
	
	cFirst = _src[0] & 0x0f;
	
	_dest[0] = cFirst;
	
	memcpy( &_dest[1], &_src[1], _nLen-1 );
    _dest[_nLen] = 0x00;
}

/*****************************************************************************
* Function Name : FirstNibbleToStr()
* Description   : 맨 마지막 바이트의 첫번째 nibble까지 유효한 데이터를 입력받아
	유효하지 않은 마지막 nibble에 0을 넣어준다.
	ex) 0x12345A ==> 0x123450
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::FirstNibbleToStr(char *_dest, char *_src, int _nLen )						
{         
	char	cLast;
	
	memcpy( _dest, _src, _nLen-1 );
	
	cLast = _src[_nLen-1] & 0xf0;
	
	_dest[_nLen-1] = cLast;
	
    _dest[_nLen] = 0x00;
}

/*****************************************************************************
* Function Name : FirstNibbleToNum()
* Description   : 맨 마지막 바이트의 첫번째 nibble까지 유효한 데이터를 입력받아
	우측으로 4bit shift시켜준다.
	ex) 0x12345A ==> 0x012345
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::FirstNibbleToNum(char *_dest, char *_src, int _nLen )						
{         
	int		i;
	int		y = 0;
	char	cFirst;
	char	cSecond;
	
	for(i=0; i<_nLen; i++)
    {
    	cFirst = (_src[i] & 0xf0) >> 4;
    	
    	if( i != _nLen )
    	{
    		cSecond = (_src[i] & 0x0f) << 4;
    	}
    	
    	_dest[y++] |= cFirst;
    	
    	if( i != _nLen )
    	{
    		_dest[y] |= cSecond;
    	}
    }
    
    _dest[y] = 0x00;
}

/*****************************************************************************
* Function Name : ResultForFirstNibble()
* Description   : 맨 처음 2번째 nibble부터 유효한 데이터를 입력받아, 좌측으로
	4bit shift 시켜준다. 마지막 byte의 first nibble까지만 채워짐.
	ex) 0xA12345 ==> 0x123450
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::ResultForFirstNibble(char *_dest, char *_src, int _nLen )	
{
	int		i;
	int		y = 0;
	char	cFirst;
	char	cSecond;
	
	for(i=0; i<_nLen; i++)
    {
    	cFirst = (_src[i] & 0x0f ) << 4;
    	
    	if( i != _nLen-1 )
    	{
	    	cSecond = (_src[i+1] & 0xf0 ) >> 4;
    	}
    	else
    	{
    		cSecond = 0x00;
    	}
    	
    	_dest[y] = 0x00;
    	_dest[y] |= cFirst;
    	_dest[y++] |= cSecond;
    }
    
     _dest[y] = 0x00;
	
}

/*****************************************************************************
* Function Name : ResultForLastNibble()
* Description   : _dest의 맨 처음 nibble은 건드리지 않고 그 다음 nibble부터 
	_src의 값을 채운다.
	ex) 0xA12345 ==> 0xB12345
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::ResultForLastNibble(char *_dest, char *_src, int _nLen )	
{
	int		i;
	int		y = 0;
	char	cFirst;
	char	cSecond;
	
	for(i=0; i<_nLen; i++)
    {
    	cFirst = (_src[i] & 0x0f );
    	
    	_dest[y] = (_dest[y] & 0xf0);
    	_dest[y] |= cFirst;
    	
	    cSecond = (_src[i+1] & 0xf0 );
    	
    	_dest[++y] |= cSecond;
    }
    
     _dest[y] = 0x00;
}

/*****************************************************************************
* Function Name : ResultForMiddleNibble()
* Description   : _dest의 맨 처음 nibble은 건드리지 않고 그 다음 nibble부터 
	값을 채운다.
	ex) 0x1234 ==> 0xB1234A
* Parameters    : char *_dest 	: 변환한 결과가 저장될 포인터.
				  char *_src	: 변환할 Nibble string.
				  int _nLen		: 변환활 string length.
* Return values : void
*****************************************************************************/
void CATMConv::ResultForMiddleNibble(char *_dest, char *_src, int _nLen )	
{
	int		i;
	int		y = 0;
	char	cFirst;
	char	cSecond;
	
	for(i=0; i<_nLen-1; i++)
    {
    	cFirst = (_src[i] & 0xf0 ) >> 4;
    	
    	_dest[y] = (_dest[y] & 0xf0);
    	_dest[y] |= cFirst;
    	
	    cSecond = (_src[i] & 0x0f ) << 4;
    	
    	y++;
    	_dest[y] = (_dest[y] & 0x0f);
    	_dest[y] |= cSecond;
    }
    
     _dest[++y] = 0x00;
}



