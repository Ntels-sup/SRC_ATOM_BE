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
        static char    GetTBCDHex (char _ch) ;
        static char    GetTBCDChar(char _hex);

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
