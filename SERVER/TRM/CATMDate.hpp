#ifndef __CATMDate_Hpp__
#define __CATMDate_Hpp__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "CATMTimeDef.hpp"
#include "CATMConv.hpp"
#include "CATMDate.hpp"

class CATMDate
{
    public :
        CATMDate          (void) ;
        CATMDate          (struct tm *dt) ;
        CATMDate          (time_t) ;
        CATMDate          (char *str) ;
       ~CATMDate          () {} 

	void             AddDate               (int _day)
	{
             if ( julian_ != 0x7fffffffl ) julian_ += _day ;
	}

	void             SetDate               (time_t _tm) ;

        long             GetJulian             (void) { return julian_ ; } 
        boolean          IsValid               (void) ;
        time_t           Get                   (void) ;
        int              GetYear               (void) ;
        int              GetMonth              (void) ;
        int              GetDay                (void) ;
        int              GetDayOfWeek          (void) ;
        int              GetDate               (char *_buffer)
        {
            if ( IsValid() == NL_TRUE )
            {
                this->FromJulian(_buffer) ;
                return NL_OK ;
            }
            else
            {
                return NL_ERROR ;
            }
        }
        const char *     AscDate               (const char *_picture) ;

        friend CATMDate   operator+(CATMDate &_date, long _val);
        friend CATMDate   operator-(CATMDate &_date, long _val);
        friend CATMDate   operator+(long _val, CATMDate &_date);
        friend CATMDate   operator-(long _val, CATMDate &_date);
        char *           operator()();
        CATMDate &        operator++();
        CATMDate &        operator++(int);
        CATMDate &        operator--();
        CATMDate &        operator--(int);
        CATMDate &        operator+=(long _val);
        CATMDate &        operator-=(long _val);
        int              operator==(CATMDate &_date);

        int              operator==(char *_date)
        {
            if ( strcmp(AscDate((char *)"CCYYMMDD"),_date) == 0 )
                return 1 ;
            else
                return 0 ;
        }
        int              operator!=(char *_date)
        {
            if ( strcmp(AscDate((char *)"CCYYMMDD"),_date) != 0 )
                return 1 ;
            else
                return 0 ;
        }

        int              operator!=(CATMDate &_date);
        int              operator< (CATMDate &_date);
        int              operator<=(CATMDate &_date);
        int              operator> (CATMDate &_date);
        int              operator>=(CATMDate &_date);
        int              operator!() {return !IsValid();};

    private : 
        long             julian_ ;
        char             dateStr_[13] ;
        void             ToJulian  (long , long , long ) ;
        void             FromJulian(char *) ;

};

#endif
