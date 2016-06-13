/*!
 * \file CATMTime.hpp
 * \brief CATMTime Class Header File
 * \details class of For Log Write
 * \author
 * \date 2016.03.30
 */
#ifndef __CATMTimeHpp
#define __CATMTimeHpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include "CATMTimeDef.hpp"

class CATMTime
{
    public :
        CATMTime      (time_t nowTime) ;
        CATMTime      ()  { Now(); }
        ~CATMTime     ()  { }

        // set current timestamp
        void   Now   (void) { ftime(&this->now_) ; }

        void   CutMSec(void) { this->now_.millitm = 0 ; } ;

        // operator overloading
        int operator == (CATMTime &_y)
        {
            return ( now_.time == _y.now_.time && 
                     now_.millitm == _y.now_.millitm );
        }

        friend int   operator != (const CATMTime &_x , const CATMTime &_y) 
        {
            return ( _x.now_.time != _y.now_.time || 
                     _x.now_.millitm != _y.now_.millitm );
        }

        friend int   operator >  (const CATMTime &_x , const CATMTime &_y) ;
        friend int   operator <  (const CATMTime &_x , const CATMTime &_y) ;
        friend int   operator >= (const CATMTime &_x , const CATMTime &_y) ;
        friend int   operator <= (const CATMTime &_x , const CATMTime &_y) ;

        CATMTime &         operator =  (struct timeb &_nowTime) ;
        CATMTime &         operator =  (time_t       _nowTime) ;
        CATMTime &         operator += (CATMTime      &_nowTime) ;
        CATMTime &         operator += (time_t       _nowT) ;
        CATMTime &         operator -= (CATMTime      &_nowTime) ;
        CATMTime &         operator -= (time_t       _nowT) ;

        operator struct timeb * ()   { return GetTm(); } ;
        operator time_t ()           { return Get(); } ;

        struct timeb *    GetTm()    { return &this->now_ ; }
        time_t            Get()      { return this->now_.time ; }
        char *            AscTime    (char *_picture) ;
        char *            AscTime    (const char *_picture) { return AscTime((char*)_picture); }
        int               GetYear    (void) ;
        int               GetMonth   (void) ;
        int               GetDay     (void) ;
        int               GetHour    (void) ;
        int               GetMin     (void) ;
        int               GetSec     (void) ;
        int               GetMilliSec(void) ;
		static time_t     MakeTime   (char *_picture,char *_value) ;

		bool				IsDST(void);
		int					GetGmtOffset(void);
    private : 
        struct timeb      now_  ;
        char              ascTm_[32+1] ;
};

#endif
