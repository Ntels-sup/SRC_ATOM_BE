#ifndef __CCLI_HPP__
#define __CCLI_HPP__

#include <string>
#include "MariaDB.hpp"
#include "FetchMaria.hpp"
#include "lnkLst.h"
#include "cli.h"
#include "CCliReq.hpp"
#include "CCliRsp.hpp"
#include "CRabbitAlloc.hpp"

using namespace std;

class CCli{
    private:
        CliCb m_stCliCb;
        CliDoc *m_stCliDoc;
        bool m_blnDbInitFlag;
        string m_strLocalPath;
        string m_strDefaultPkgName;
        string m_strDBName;
        string m_strDBIp;
        string m_strDumpFile;
        int m_nDBPort;
        string m_strDBUser;
        string m_strDBPassword;
        struct EnumArgValue{
            string strTable;
            string strColumn;
        };
        DB *m_cDb;

        static const int CMD_CODE_DUMP = 100;
        static const int CMD_CODE_LOAD = 101;
        static const int CMD_CODE_EDIT_CFG = 102;
        static const int CMD_CODE_START_NA = 103;
        static const int CMD_CODE_DISP_NA = 104;
        static const int CMD_CODE_STOP_NA = 105;

        int LoadArgEnumFromDb(string &a_strPkgName, CliCmdCb *a_stCmdCb, CliArgCb *a_stArgCb);
        int LoadArgFromDb(string &a_strPkgName, CliCmdCb *cmdCb);
        int LoadArgEnumFromCfg(CliArgCb *a_stArgCb, rabbit::object &a_cRecord);
        int LoadArgFromCfg(CliCmdCb *a_stCmdCb, rabbit::object &a_cRecord);
        int LoadCmdFromCfg(CliPkgCb *a_cPkgCb, rabbit::object &a_cRecord);
        int LoadPkgFromCfg(rabbit::object &a_cRecord);
        int DumpArgEnum(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliArgEnumCb &argEnumCb);
        int DumpArg(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliArgCb &argCb);
        int DumpCmd(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliCmdCb &cmdCb);
        int DumpPkg(CRabbitAlloc &alloc, rabbit::object &a_cRecord, CliPkgCb &pkgCb);
        int LoadDfltCmdDump(CliPkgCb *pkgCb);
        int LoadDfltCmdLoad(CliPkgCb *pkgCb);
        int LoadDfltCmdEditCfg(CliPkgCb *pkgCb);
        int LoadDfltCmdDispNa(CliPkgCb *pkgCb);
        int LoadDfltCmdStartNa(CliPkgCb *pkgCb);
        int LoadDfltCmdStopNa(CliPkgCb *pkgCb);
        int LoadDfltCmd();
        int CheckDefaultCmd(CliDoc *a_stDoc);
    public:
        CCli(int &nRet);
        ~CCli();
        int InitDbConfig(const char *a_szDBName, const char *a_szDBIp, int a_nDBPort,
                const char *a_szDBUser, const char *a_szDBPassword);
        int LoadArgEnumParaFromDb(CliArgCb *a_stArgCb, 
                const char *a_szTableName, const char *a_szColumnName, int a_nColumnType);
        int LoadPkgFromDb(string &a_strPkgName);
        int LoadFromDb();
        int LoadFromCfg(string &a_strPath);
        int SetDefaultPkg(string &a_strPkgName);
        void SetLocalPath(string &a_strLocalPath) { m_strLocalPath = a_strLocalPath; };
        void SetDumpFile(string &a_strDumpFile) { m_strDumpFile = a_strDumpFile; };
        int Dump();
        int LoadConfig(string &cfg);
        int Read(char *a_szLine);
        int Read(string &a_strLine);
        int Read();
        int DisplayHeader(time_t a_time, unsigned int a_nCmdCode, string &a_strCmd, string &a_strCmdDesc);
        int Display(unsigned int a_nResultCode, string &a_strResult, string &a_strText);
        int GetCmd(unsigned int *a_nCmdCode, string &a_strCmd, string &a_strCmdDesc);
        int GetJsonMessage(unsigned int a_nSessionId, string &a_strMessage );
};

#endif
