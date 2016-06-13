/*
 ***************************************************************************
 *
 * CATMDate.hpp -  nTels Library -  date class.
 *
 ***************************************************************************
 * Description:
 *
 *    date type 변수를 대체하는 class로 date에 관한 다양한 method를
 *    제공한다..       
 *
 ***************************************************************************
 *
 * (c) Copyright 2002 nTels Inc.
 *
 * ALL RIGHTS RESERVED
 *
 * The software and information contained herein are proprietary to, and
 * comprise valuable trade secrets of, nTels Inc., which intends to preserve
 * as trade secrets such software and information.
 * This software is furnished pursuant to a written license agreement and
 * may be used, copied, transmitted, and stored only in accordance with
 * the terms of such license and with the inclusion of the above copyright
 * notice.  This software and information or any other copies thereof may
 * not be provided or otherwise made available to any other person.
 *
 * Notwithstanding any other lease or license that may pertain to, or
 * accompany the delivery of, this computer software and information, the
 * rights of the Government regarding its use, reproduction and disclosure
 * are as set forth in Section 52.227-19 of the FARS Computer
 * Software-Restricted Rights clause.
 *
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
 * Technical Data and Computer Software clause at DFARS 252.227-7013.
 * Contractor/Manufacturer is nTels Inc., 6rd, Simone Building,
 * Samsung-Dong, Kangnam-Ku, Seoul, Korea 135-090
 *
 * This computer software and information is distributed with "restricted
 * rights."  Use, duplication or disclosure is subject to restrictions as
 * set forth in NASA FAR SUP 18-52.227-79 (April 1985) "Commercial
 * Computer Software-Restricted Rights (April 1985)."  If the Clause at
 * 18-52.227-74 "Rights in Data General" is specified in the contract,
 * then the "Alternate III" clause applies.
 *
 *      Auther : Jeong, Chan-Ho                 2002.11.15
 *               jch2001@ntels.com
 *
 **************************************************************************
 *
 *      Patch Report:
 *
 **************************************************************************
 *      Version    Date    Patch Number             Description
 **************************************************************************
 *
 *
 **************************************************************************
 *
 *      Amendment History
 *      Begin
 **************************************************************************
 *		Version    Date          Author         Description
 **************************************************************************
 *       0.01    2003.11.25      Jeong, Chan-Ho		Initial
 *
 *      End of Amendment
 **************************************************************************
 */

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
