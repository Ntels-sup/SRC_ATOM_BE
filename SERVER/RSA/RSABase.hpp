#ifndef __DEF_RSA_BASE__
#define __DEF_RSA_BASE__ 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <math.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

using namespace rapidjson;

#include "CFileLog.hpp"
#include "RSAGlobal.hpp"

#include "CStmResRspApi.hpp"
#include "CStmResReqApi.hpp"

#include "EventAPI.hpp"

#if 0
double Rounding (double x, int digit)
{
	return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}
#endif

class RSABase
{
	private:
	public:
		RSABase() {};
		virtual ~RSABase() {};
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain) = 0;
		virtual int MakeJson(time_t a_tCur) = 0;
		virtual int Run() = 0;

		//통계 데이터를 포함하는 JSON MSG 생성 (CPU, DISK, MEMORY, TABLESPACE, TEMP, TOT_CPU 모듈만 생성)
		virtual int MakeStatJson(time_t a_tCur) { return 0; }
		//NIC 의 IP 정보를 포함하는 JSON MSG 생성 (NIC PLUGIN 에서만 사용)
		virtual int MakeIPJson(const char *a_szIFName, const char *a_szVersion, std::string &a_strResult) { return 0; }
		//연결 된 노드들의 PING 정보를 포함하는 JSON MSG 생성 (PING PLUGIN 에서만 사용)
		virtual int MakePingJson(std::string &a_strResult) { return 0; }
		//연결 된 노드들의 IP 정보를 포함하는 JSON MSG 생성 (PING PLUGIN 에서만 사용)
		virtual int MakeNodeJson(const char *a_szPkgName, const char *a_szPeerType, std::string &a_strResult) 
		{
			return 0;
		}

};

#endif
