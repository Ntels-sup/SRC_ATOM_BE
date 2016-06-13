/*
 ***************************************************************************
 *
 * CATMConv.hpp -  nTels Library -  conversion class.
 *
 ***************************************************************************
 * Description:
 *
 *    data conversion에 관한 다양한 method를 제공한다..       
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
#ifndef __CATMCONV_HPP__
#define __CATMCONV_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

#include "CATMTimeDef.hpp"

typedef unsigned char byte ;
typedef int           boolean ;

class CATMConv
{
    public :
        static char    GetBCDHex  (char _ch) ;
        static char    GetBCDChar (char _hex);
        static char    GetTBCDHex (char _ch) ; //for TBCD (ITU BCD Extension)
        static char    GetTBCDChar(char _hex); //for TBCD (ITU BCD Extension)

        // convert BCD <-> STR function
        static int     Str2BCD    (char *_dest,char *_src) ;
        static void    BCD2Str    (char *_dest,char *_src,int _len) ;
        static int     Str2Int    (char *_num ,int _len) ;
        static char   *Int2Str    (int   _num) ;

        // convert TBCD <-> STR function
        static int     Str2TBCD   (char *_dest,char *_src) ;
        static int     TBCD2Str   (char *_dest,char *_src,int _len) ;
        static int     TBCD2Str   (char *_dest,char *_src,int _len,int _mkLen) ;
		
		// convert HEX <-> STR function
		static int     Str2Hex(char *_dest,char *_src);
        static void    Hex2Str(char *_dest,char *_src,int _len);
        static void    Hex2Str(char *_dest,char *_src,int _len,int _mkLen);
        
        // convert TIME <-> STR function
        static time_t  Str2Time(char *_ptime);
        static void    Time2Str(char *_dest,time_t _tTime);
		// add new. GMT Convert. 2013-06-24
        static time_t  Str2GMTTime(char *_ptime);
        static void    Time2GMTStr(char *_dest,time_t _tTime);
        
        // convert IPv4 <-> STR function
        static bool    Str2Ipv4(char *_dest,char *_src);
        static void    Ipv42Str(char *_dest, unsigned int _unIp);			
        
        // convert NIBBLE -> STR function
        static void    MiddleNibbleToStr(char *_dest, char *_src, int _nLen );
		static void    LastNibbleToStr(char *_dest, char *_src, int _nLen );
		static void    FirstNibbleToStr(char *_dest, char *_src, int _nLen );
		static void    FirstNibbleToNum(char *_dest, char *_src, int _nLen );
		static void    ResultForFirstNibble(char *_dest, char *_src, int _nLen );	
		static void    ResultForLastNibble(char *_dest, char *_src, int _nLen );
		static void    ResultForMiddleNibble(char *_dest, char *_src, int _nLen );
		
        // convert byte order ( big-endian <-> little-endian )
        static  void   ChangeByteOrder(byte *_value,size_t _size) ;
};

#endif
