/*
 ***************************************************************************
 *
 * CATMConv.cpp -  nTels Library -  conversion clss.
 *
 ***************************************************************************
 */
#include "CATMDate.hpp"
#include <string>

using namespace std;

/*****************************************************************************
* Function Name : CATMDate
* Description   : CATMDate constructor.
* Parameters    : None.
* Return value  : None
*****************************************************************************/
CATMDate::CATMDate(void)
{
    time_t now;
    struct tm *dt;
    struct tm  result;

    time(&now);
    dt = localtime_r(&now,&result);

    this->ToJulian(dt->tm_year + 1900, dt->tm_mon + 1, dt->tm_mday);
}

/*****************************************************************************
* Function Name : CATMDate
* Description   : CATMDate constructor.
* Parameters    : None.
*     Input     : struct tm *dt 
* Return value  : None
*****************************************************************************/
CATMDate::CATMDate(struct tm *_dt)
{
    this->ToJulian(_dt->tm_year + 1900, _dt->tm_mon + 1, _dt->tm_mday);
}

/*****************************************************************************
* Function Name : CATMDate
* Description   : CATMDate constructor.
* Parameters    : None.
*     Input     : time_t tm 
* Return value  : None
*****************************************************************************/
CATMDate::CATMDate(time_t _tm)
{
    SetDate(_tm) ;
}
void CATMDate::SetDate(time_t _tm)
{
    struct tm *dt ;
    struct tm  result  ;
    
    dt = localtime_r(&_tm,&result);
    this->ToJulian(dt->tm_year + 1900, dt->tm_mon + 1, dt->tm_mday);
}


/*****************************************************************************
* Function Name : CATMDate
* Description   : CATMDate constructor.
* Parameters    : 
*     Input     : char *_str ( YYYYMMDD format )
* Return value  : None
*****************************************************************************/
CATMDate::CATMDate(char *_str)
{
    long year,month,day ;
    int  i ;

    if ( strlen(_str) == 8 )
    {
        for(i=0;i<8;i++)
        {
            if (_str[i] < '0' || _str[i] > '9' ) 
            {
                NL_DISPLAY_LIB_ERROR("invalid date string") ;

                this->julian_ = NL_INVALID_DATE ;
                return ;
            }
        }

        year  = CATMConv::Str2Int(_str  ,4) ;
        month = CATMConv::Str2Int(_str+4,2) ;
        day   = CATMConv::Str2Int(_str+6,2) ;

        this->ToJulian(year,month,day) ;
    }
    else
    {
        NL_DISPLAY_LIB_ERROR("invalid date string") ;

        this->julian_ = NL_INVALID_DATE ;
    }
}

/*****************************************************************************
* Function Name : IsValid
* Description   : 현재 설정된값이 유효한 Date값인지 확인한다.
* Parameters    : None.
* Return value  :
*                 NL_TRUE  : 유효한 Date 값임.
*                 NL_FALSE : 유효하지 않은 Date값임.
*****************************************************************************/
boolean CATMDate::IsValid(void)
{
    if(this->julian_ == NL_INVALID_DATE )
    {
        return NL_FALSE ;
    }

    return NL_TRUE ;
}

/*****************************************************************************
* Function Name : Get
* Description   : 현재 설정된 Date값을  time_t값으로 변환하여 돌려준다.
* Parameters    : None.
* Return value  :
*                 time_t : 현재설정된 Date값의 time_t 변환값.
*****************************************************************************/
time_t CATMDate::Get(void)
{
	struct tm dt;
	char   str[9];
    int    year,month,day ;

    if ( this->julian_ == NL_INVALID_DATE )
    {
        return -1 ;
    }

	this->FromJulian(str);

	memset(&dt, 0, sizeof(dt));

    year  = CATMConv::Str2Int(str  ,4) ;
	month = CATMConv::Str2Int(str+4,2) ;
	day   = CATMConv::Str2Int(str+6,2) ;
	
	dt.tm_year = year - 1900;
	dt.tm_mon =  month - 1;
	dt.tm_mday = day ;

	dt.tm_isdst = -1;

	return mktime(&dt);
}

/*****************************************************************************
* Function Name : GetYear
* Description   : 현재 설정된 Date값의 년도를 구하여 돌려준다.
* Parameters    : None.
* Return value  :
*                 int : 현재설정된 Date값의 yeae 값.
*****************************************************************************/
int CATMDate::GetYear(void)
{
    char str[9];

    if ( this->julian_ == NL_INVALID_DATE )
    {
        return -1 ;
    }

    this->FromJulian(str) ;

    return  CATMConv::Str2Int(str,4) ;
}

/*****************************************************************************
* Function Name : GetYear
* Description   : 현재 설정된 Date값의 월을 구하여 돌려준다.
* Parameters    : None.
* Return value  :
*                 int : 현재설정된 Date값의 month 값.
*****************************************************************************/
int CATMDate::GetMonth(void)
{
    char str[9];

    if ( this->julian_ == NL_INVALID_DATE )
    {
        return -1 ;
    }

    this->FromJulian(str) ;

    return  CATMConv::Str2Int(str+4,2) ;
}

/*****************************************************************************
* Function Name : GetDay
* Description   : 현재 설정된 Date값의 일을 구하여 돌려준다.
* Parameters    : None.
* Return value  :
*                 int : 현재설정된 Date값의 day 값.
*****************************************************************************/
int CATMDate::GetDay(void)
{
    char str[9];

    if ( this->julian_ == NL_INVALID_DATE )
    {
        return -1 ;
    }

    this->FromJulian(str) ;

    return  CATMConv::Str2Int(str+6,2) ;
}

/*****************************************************************************
* Function Name : GetDayOfWeek
* Description   : 현재 설정된 Date값의 요일을 구하여 돌려준다.
* Parameters    : None.
* Return value  :
*                 int : 현재설정된 Date값의 요일 값. (0~6) 
*****************************************************************************/
int CATMDate::GetDayOfWeek(void)
{
    if ( this->julian_ == NL_INVALID_DATE )
    {
        return -1 ;
    }

    return (unsigned)((this->julian_ + 1l) % 7l) ;
}

/*****************************************************************************
* Function Name : operator()
* Description   : .
* Parameters    : None.
* Return value  :
*                 char * : 현재설정된 Date값 (YYYYMMDD)
*****************************************************************************/
char *CATMDate::operator()()
{
    static char buf[9];

    this->FromJulian(buf);

    return buf;
}

/*****************************************************************************
* Function Name : operator++
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate & CATMDate::operator++()
{
    ++this->julian_;

    return *this;
}

CATMDate & CATMDate::operator++(int)
{
    ++this->julian_;

    return *this;
}

/*****************************************************************************
* Function Name : operator--
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate & CATMDate::operator--()
{
    --this->julian_;

    return *this;
}

CATMDate & CATMDate::operator--(int)
{
    --this->julian_;

    return *this;
}

/*****************************************************************************
* Function Name : operator+=
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate & CATMDate::operator+=(long _val)
{
    this->julian_ += _val;

    return *this;
}

/*****************************************************************************
* Function Name : operator-=
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate & CATMDate::operator-=(long _val)
{
    this->julian_ -= _val;

    return *this;
}

/*****************************************************************************
* Function Name : operator==
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator==(CATMDate &d)
{
    return this->julian_ == d.julian_;
}

/*****************************************************************************
* Function Name : operator!=
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator!=(CATMDate &d)
{
    return this->julian_ != d.julian_;
}

/*****************************************************************************
* Function Name : operator<
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator<(CATMDate &d)
{
    return this->julian_ < d.julian_;
}

/*****************************************************************************
* Function Name : operator<=
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator<=(CATMDate &d)
{
    return this->julian_ <= d.julian_;
}

/*****************************************************************************
* Function Name : operator>
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator>(CATMDate &d)
{
    return this->julian_ > d.julian_;
}

/*****************************************************************************
* Function Name : operator>=
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
int CATMDate::operator>=(CATMDate &d)
{
    return this->julian_ >= d.julian_;
}


/*****************************************************************************
* Function Name : operator+
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate operator+(CATMDate &date, long val)
{
    CATMDate d = date;
    d.julian_ += val;

    return d;
}

/*****************************************************************************
* Function Name : operator+
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate operator+(long val, CATMDate &date)
{
    CATMDate d = date;
    d.julian_ += val;

    return d;
}

/*****************************************************************************
* Function Name : operator-
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate operator-(CATMDate &date, long val)
{
    CATMDate d = date;
    d.julian_ -= val;
    return d;
}

/*****************************************************************************
* Function Name : operator-
* Description   : .
* Parameters    : None.
* Return value  :
*****************************************************************************/
CATMDate operator-(long val, CATMDate &date)
{
    CATMDate d = date;
    d.julian_ -= val;
    return d;
}
/*****************************************************************************
* Function Name : ToJulian
* Description   : 주어진 year,month,day값을 julian 값으로 설정한다.
* Parameters    : 
*     Input     : int year 
*                 int month
*                 int day
* Return value  : None
*****************************************************************************/
void CATMDate::ToJulian(long year, long month, long day)
{
    this->julian_ = NL_INVALID_DATE ;

    if(month < 1 || month > 12)
    {
        return;
    }

    if(day < 1 || day > 31)
    {		
        return;
    }
    if ( year == 0 )
    {		
        return;
    }
   

    if(year < 0)
    {
        year++;
    }

    this->julian_ = day - 32075l +
        1461l * (year + 4800l + ( month - 14l) / 12l) / 4l +
        367l * (month - 2l - (month - 14l) / 12l * 12l) / 12l -
	3l * ((year + 4900l + (month - 14l) / 12l) / 100l) / 4l;
}

/*****************************************************************************
* Function Name : ToJulian
* Description   : 현재 설정된  julian 값을 'YYYYMMDD'문자열로 변환한다.
* Parameters    : 
*     Outpur    : char *buffer : 변환결과 buffer
*                                반드시 size가 9 여야만 한다.
* Return value  : None
*****************************************************************************/
void CATMDate::FromJulian(char *buffer)
{
	double i, j, k, l, n;

	l = julian_ + 68569.0;
	n = int( 4 * l / 146097.0);
	l = l - int( (146097.0 * n + 3)/ 4 );
	i = int( 4000.0 * (l+1)/1461001.0);
	l = l - int(1461.0*i/4.0) + 31.0;
	j = int( 80 * l/2447.0);
	k = l - int( 2447.0 * j / 80.0);
	l = int(j/11);
	j = j+2-12*l;
	i = 100*(n - 49) + i + l;

    sprintf(buffer,"%04d%02d%02d",(int)i,(int)j,(int)k) ;
}

/*****************************************************************************
* Function Name : AscDate
* Description   : 현재 설정된 date값을 Asc String으로 변환한다.
*                 picture 에 맞게 format을 다양하게 지정할 수 있다.
* Parameters    : 
*     Input     : char *picture : asc string format
* Return value  :
*                 char *: 유효한 Date 값임.
*****************************************************************************/
const char *CATMDate::AscDate(const char *picture)
{
    char        dateStr[9] ;
    char       *CC,*YY,*MM,*DD ;
    char       *dest ;
    char        value ;
    int         picCnt[4] ;
    int         picLen = strlen(picture) ;
    int         i;

    if ( picture == NULL || picLen > (int)(sizeof(this->dateStr_))-1 )
    {
        NL_DISPLAY_LIB_ERROR("picture size too long") ;
        return NULL ;
    }
    if ( this->IsValid() == NL_FALSE )
    {
        NL_DISPLAY_LIB_ERROR("invalid method") ;
        return NULL ;
    }

    this->FromJulian(dateStr) ;

    dest = this->dateStr_ ;

    CC = dateStr +  0  ;
    YY = dateStr +  2 ;
    MM = dateStr +  4 ;
    DD = dateStr +  6 ;
    
    for(i=0;i<4;i++) 
        picCnt[i] = 0 ;

    for(i=0;i<picLen;i++)
    {
        switch(picture[i])
        {
            case 'C' :            // Century
                if ( picCnt[0] == 2 )
                    goto picture_error ;
                value = *CC++ ;
                picCnt[0]++ ;
                break ;
            case 'Y' :
                if ( picCnt[1] == 2 )
                    goto picture_error ;
                value = *YY++ ;   // Year
                picCnt[1]++ ;
                break ;
            case 'M' :
                if ( picCnt[2] == 2 )
                    goto picture_error ;
                value = *MM++ ;   // Month
                picCnt[2]++ ;
                break ;
            case 'D' :
                if ( picCnt[3] == 2 )
                    goto picture_error ;
                value = *DD++ ;   // Day
                picCnt[3]++ ;
                break ;
            default :
                value = picture[i] ;
                break ;
        }
        *dest++ = value ;
    }
    *dest = 0x00 ;

    return this->dateStr_ ;

picture_error :

    return NULL ;
}

