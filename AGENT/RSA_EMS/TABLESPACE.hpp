#ifndef __DEF_MONITOR_DB__
#define __DEF_MONITOR_DB__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"

//Include For Db Connection
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#define DEF_MAX_FILE_COUNT	20	

typedef enum
{
	IDX_DB_USAGE = 0,
	IDX_DB_TOTAL,
	IDX_DB_FREE,
	IDX_DB_USED,
	MAX_DB_IDX
}DB_IDX;

const char *DB_COLUMN[] =
{
	"usage",
	"total",
	"free",
	"used"
};

typedef struct _db_value
{
	vector<double> vecAvgValue;
}DB_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class TABLESPACE : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		vector<RESOURCE_ATTR*> vecTop;
		RSAMain *m_pclsMain;
		DB *m_pclsDB;
        EventAPI *m_pclsEvent;

        uint64_t m_unTotalSize;
	public :
		TABLESPACE();
		virtual ~TABLESPACE();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int GetDBConnInfo(char *a_pszArgs, char **a_pszConnInfo);
		uint64_t ParsingFileSize(char *a_pszFileConf);
		uint64_t GetDBFileSize(char *a_pszValue);

		int ConnectDB();
		int InsertTopData();
		int MakeStatJson(time_t a_tCur);
		
};


#endif
