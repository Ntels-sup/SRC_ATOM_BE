struct ColumnInfo{
CHAR name;
};

struct TableInfo{
UINT columnCnt;
UINT collectTm;
UINT type;
UINT collectFlg;
PackageInfo ownPkg;
};

struct PackageInfo{
STRING DBName;
UINT tableCnt;
TableInfoList tblLst;
LnkLst AssociateNodeList; 
};

struct NodeInfo{
#define ACT
#define INACT
UINT status;
UINT HBSndTm;
UINT lastCletTm;
LnkLst queuedSession;
TCAInfo tcaInfoLst[];
};

struct SessionInfo{
UINT sndTm;
ReqTableName tblName;

};

struct SessionStr{
UINT sesCnt;
};

struct TmrInfo{
#define REQ_MSG_TM;
#define TABLE_SND_TM;
};

struct TCARule{
}

struct TCAInfo{
};

struct ReqMsgFmt{
STRING tblName;
UINT sesId;
UINT actFlg;
TIME startTm;
TIME collectTm;
};

/* insert table set 
 * insert table_name set
 * column1 = 'value1',
 * column2 = 'value2';
 */

struct ColumnInfoAvp {
columnName[];
columnVal[];
};


struct AnsMsgFmt{
UINT sesId;
TIME startTm;
TIME collectTm;
UINT dstFlg;
ColumnInfoAvp info[];
};

struct RdcMsgFmt{
UINT sesId;
TIME startTm;
TIME collectTm;
UINT dstFlg;
ColumnInfoAvp info[];
};


