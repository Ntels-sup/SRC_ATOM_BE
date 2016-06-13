/*!
 * \file CATMTime.cpp
 * \brief CTime Class Source File
 * \details 
 * \author 
 * \date 2016.03.30
 */

#include "CATMTime.hpp"

time_t CATMTime::MakeTime(char *_picture,char *value)
{
    struct tm   tm ;
    int         century,year,mon, day, hour,min,sec ;
    int         picCnt[8] ;
    int         picLen = strlen(_picture) ;
    int         i ;
    char       *ptr ;
    char        picture[32] ;

    if ( picLen != (int)strlen(value))
    {
        goto picture_error ;
    }

    strncpy(picture, _picture, 32) ;
    ptr = strstr(picture,"YYYY") ;
    if ( ptr != NULL )
    {
        strncpy(ptr,"CC",2) ;
    }

    century = year = mon = day = hour =min = sec = 0 ;

    for(i=0;i<8;i++) 
        picCnt[i] = 0 ;

    for(i=0;i<picLen;i++)
    {
        switch(picture[i])
        {
            case 'C' :            // Century
                if ( picCnt[0] == 2 )
                    goto picture_error ;
                century += (value[i] - 0x30) * ((picCnt[0] == 0) ? 10 : 1 ) ;
                picCnt[0]++ ;
                break ;
            case 'Y' :
                if ( picCnt[1] == 2 )
                    goto picture_error ;
                year += (value[i] - 0x30) * ((picCnt[1] == 0) ? 10 : 1 ) ;
                picCnt[1]++ ;
                break ;
            case 'M' :
                if ( picCnt[2] == 2 )
                    goto picture_error ;
                mon += (value[i] - 0x30) * ((picCnt[2] == 0) ? 10 : 1 ) ;
                picCnt[2]++ ;
                break ;
            case 'D' :
                if ( picCnt[3] == 2 )
                    goto picture_error ;
                day += (value[i] - 0x30) * ((picCnt[3] == 0) ? 10 : 1 ) ;
                picCnt[3]++ ;
                break ;
            case 'h' :
                if ( picCnt[4] == 2 )
                    goto picture_error ;
                hour += (value[i] - 0x30) * ((picCnt[4] == 0) ? 10 : 1 ) ;
                picCnt[4]++ ;
                break ;
            case 'm' :
                if ( picCnt[5] == 2 )
                    goto picture_error ;
                min += (value[i] - 0x30) * ((picCnt[5] == 0) ? 10 : 1 ) ;
                picCnt[5]++ ;
                break ;
            case 's' :
                if ( picCnt[6] == 2 )
                    goto picture_error ;
                sec += (value[i] - 0x30) * ((picCnt[6] == 0) ? 10 : 1 ) ;
                picCnt[6]++ ;
                break ;
            default :
                break ;
        }
    }

    tm.tm_year = (century * 100 + year) - 1900 ;
    tm.tm_mon  = mon - 1 ;
    tm.tm_mday = day ;
    tm.tm_hour = hour ;
    tm.tm_min  = min ;
    tm.tm_sec =  sec ;
    tm.tm_isdst =  -1;

    return mktime(&tm) ;

picture_error :

    return NULL ;
}

/*!
 * Function Name : AscTime
 * Description   : ¿¿ ¿¿¿ time¿¿ Asc String¿¿ ¿¿¿¿.
 *                 picture ¿ ¿¿ format¿ ¿¿¿¿ ¿¿¿ ¿ ¿¿.
 * Parameters    : char *picture - asc string format
 * Return value  : char *: ¿¿¿ Date ¿¿.
 */

char *CATMTime::AscTime(char *_picture)
{
    struct tm  *t_block ;
    struct tm   t_result ;
    char        timeStr[24] ;
    char        picture[32] ;
    char       *CC,*YY,*DD,*MM,*hh,*mm,*ss,*XX ;
    char       *dest ;
    char       *ptr ;
    char        value ;
    int         picCnt[8] ;
    int         picLen = NULL;
    int         i;

   	picLen = strlen(_picture);

    if ( picLen > (int)(sizeof(this->ascTm_))-1 )
    {
        NL_DISPLAY_LIB_ERROR("picture size too long") ;
        return NULL ;
    }

    strncpy(picture, _picture, 32) ;
    ptr = strstr(picture,"YYYY") ;
    if ( ptr != NULL )
    {
        strncpy(ptr,"CC",2) ;
    }
 	
    t_block = localtime_r(&this->now_.time,&t_result) ;

    sprintf(timeStr,"%04d%02d%02d%02d%02d%02d%03d",t_block->tm_year+1900,
                                                   t_block->tm_mon+1,
                                                   t_block->tm_mday,
                                                   t_block->tm_hour,
                                                   t_block->tm_min,
                                                   t_block->tm_sec,
                                                   this->now_.millitm) ;

    dest = this->ascTm_ ;

    CC = timeStr +  0  ;
    YY = timeStr +  2 ;
    MM = timeStr +  4 ;
    DD = timeStr +  6 ;
    hh = timeStr +  8 ;
    mm = timeStr + 10 ;
    ss = timeStr + 12 ;
    XX = timeStr + 14 ;
    
    for(i=0;i<8;i++) 
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
            case 'h' :
                if ( picCnt[4] == 2 )
                    goto picture_error ;
                value = *hh++ ;   // Hour
                picCnt[4]++ ;
                break ;
            case 'm' :
                if ( picCnt[5] == 2 )
                    goto picture_error ;
                value = *mm++ ;   // Minute
                picCnt[5]++ ;
                break ;
            case 's' :
                if ( picCnt[6] == 2 )
                    goto picture_error ;
                value = *ss++ ;   // Second
                picCnt[6]++ ;
                break ;
            case 'X' :            // Milli Second 
	    	case 'c' :
                if ( picCnt[7] == 3 )
                    goto picture_error ;
                value = *XX++ ;
                picCnt[7]++ ;
                break ;
            default :
                value = picture[i] ;
                break ;
        }
        *dest++ = value ;
    }
    *dest = 0x00 ;

    return this->ascTm_ ;

picture_error :

    return NULL ;
}

CATMTime::CATMTime(time_t nowTime)
{
    this->now_.time    =  nowTime;
    this->now_.millitm =  0;
}

/*!
* Function Name : operator> 
* Description   : > operator over loading
*/
int operator > (CATMTime const &_x , CATMTime const &_y)
{ 
    unsigned long a,b ;

    a = _x.now_.time * 100 + (unsigned long)(_x.now_.millitm * 100) ;
    b = _y.now_.time * 100 + (unsigned long)(_y.now_.millitm * 100) ;

    return (a  > b ) ;
}

/*!
* Function Name : operator< 
* Description   : < operator over loading
*/
int operator < (CATMTime const &_x , CATMTime const &_y)
{ 
    unsigned long a,b ;

    a = _x.now_.time * 100 + (unsigned long)(_x.now_.millitm * 100) ;
    b = _y.now_.time * 100 + (unsigned long)(_y.now_.millitm * 100) ;

    return (a  < b ) ;
}

/*!
* Function Name : operator<= 
* Description   : <= operator over loading
*/
int operator <= (CATMTime const &_x , CATMTime const &_y)
{ 
    unsigned long a,b ;

    a = _x.now_.time * 100 + (unsigned long)(_x.now_.millitm * 100) ;
    b = _y.now_.time * 100 + (unsigned long)(_y.now_.millitm * 100) ;

    return (a <= b ) ;
}

/*!
* Function Name : operator>=
* Description   : >= operator over loading
*/
int operator >= (CATMTime const &_x , CATMTime const &_y)
{ 
    unsigned long a,b ;

    a = _x.now_.time * 100 + (unsigned long)(_x.now_.millitm * 100) ;
    b = _y.now_.time * 100 + (unsigned long)(_y.now_.millitm * 100) ;

    return (a >= b ) ;
}

/*!
* Function Name : operator=
* Description   : = operator over loading
*/
CATMTime& CATMTime::operator=(struct timeb &nowTime)
{
    this->now_.time    =  nowTime.time ;
    this->now_.millitm =  nowTime.millitm ;

    return *this ;
}

/*!
* Function Name : operator=
* Description   : = operator over loading
*/
CATMTime& CATMTime::operator=(time_t nowTime)
{
    this->now_.time    =  nowTime;
    this->now_.millitm =  0;

    return *this ;
}

/*!
* Function Name : operator+=
* Description   : += operator over loading
*/
CATMTime& CATMTime::operator+=(CATMTime &nowTime)
{
    this->now_.time    += nowTime.now_.time ;
    this->now_.millitm += nowTime.now_.millitm ;

    if ( this->now_.millitm >= 100 )
    {
        this->now_.time++ ;
        this->now_.millitm -= 100 ;
    }

    return *this ;
}			

/*!
* Function Name : operator+=
* Description   : += operator over loading
* Parameters    : 
* Return value  :
*/
CATMTime& CATMTime::operator+=(time_t _nowTime)
{
    this->now_.time    += _nowTime ;

    return *this ;
}			

/*!
* Function Name : operator-=
* Description   : += operator over loading
*/
CATMTime& CATMTime::operator-=(time_t _nowTime)
{
    this->now_.time    -= _nowTime ;

    return *this ;
}
/*!
* Function Name : operator-=
* Description   : -= operator over loading
*/
CATMTime& CATMTime::operator-=(CATMTime &nowTime)
{
    short tmp ;

    this->now_.time    -= nowTime.now_.time ;
    tmp = this->now_.millitm - nowTime.now_.millitm ;

    if ( tmp < 0 )
    {
        this->now_.time-- ;
        this->now_.millitm = tmp + 100 ;
    }
    else
    {
        this->now_.millitm = tmp ;
    }

    return *this ;
}			

/*!
* Function Name : GetYear
* Description   : ¿¿ ¿¿¿ time¿¿¿ Year¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : year
*/
int CATMTime::GetYear(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_year+1900 ;
}

/*!
* Function Name : GetMonth
* Description   : ¿¿ ¿¿¿ time¿¿¿ Month¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : month
*/
int CATMTime::GetMonth(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_mon+1;
}

/*!
* Function Name : GetDay
* Description   : ¿¿ ¿¿¿ date¿¿¿ day¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : day
*/
int CATMTime::GetDay(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_mday;
}

/*!
* Function Name : GetHour
* Description   : ¿¿ ¿¿¿ time¿¿¿ hour¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : hour
*/
int CATMTime::GetHour(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_hour;
}

/*!
* Function Name : GetMin
* Description   : ¿¿ ¿¿¿ time¿¿¿ minute¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : minute
*/
int CATMTime::GetMin(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_min;
}

/*!
* Function Name : GetMin
* Description   : ¿¿ ¿¿¿ time¿¿¿ second ¿ ¿¿¿.
* Parameters    : None
* Return value  :
*                 int : second
*/
int CATMTime::GetSec(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_sec;
}

/*!
* Function Name : GetMin
* Description   : Get mili second from current time 
* Parameters    : None
* Return value  :
*                 int : milli second
*/
int CATMTime::GetMilliSec(void)
{
    return this->now_.millitm ;
}

/*!
* Function Name : IsDST
* Description   : Get the Dst info inthe Current Time value
* Parameters    : None
* Return value  :
*				  true : Effect Daylight Saving Time
*				  false : No Effect Daylight Saving Time
*/
bool CATMTime::IsDST(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return (t_block->tm_isdst > 0) ? true : false;
}
/*!
*	\brief get GMT time info Offset
*	\details use localtime_r 
*   \param none
*	\return t_block->tm_gmtoff
*/
int CATMTime::GetGmtOffset(void)
{
    struct tm *t_block ;
    struct tm  t_result ;

    t_block = localtime_r(&this->now_.time,&t_result) ;

    return t_block->tm_gmtoff;
}


