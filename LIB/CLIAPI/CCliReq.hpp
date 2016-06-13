#ifndef __CCLICMD_HPP__
#define __CCLICMD_HPP__

#include <string>
#include <vector>
#include <map>
#include <list>

#include "CCliApi.hpp"
#include "CCliArg.hpp"

class CCliReq : public CCliApi{
    private:
        unsigned int m_nSessionId;
        unsigned int m_nCmdCode;
        std::string m_strCmdLine;
        std::string m_strPkgName;
        std::string m_strCmdName;
        std::list<CCliArg*> m_lstArg;
        std::map<std::string, std::list<CCliArg*>::iterator> m_mapArg;

		CCliArg *PushArg(std::string &a_strArgName);
    public:
        CCliReq();
        CCliReq(unsigned int a_nSessionId);
        CCliReq(unsigned int a_nSessionId, int a_nCmdCode, std::string &a_strCmdName);
        CCliReq(unsigned int a_nSessionId, int a_nCmdCode, const char *a_szCmdName);
        CCliReq(unsigned int a_nSessionId, int a_nCmdCode, std::string &a_strCmdName, std::string &a_strPkgName );
        CCliReq(unsigned int a_nSessionId, int a_nCmdCode, const char *a_szCmdName, const char *a_szPkgName );
		~CCliReq();

        CCliArg& operator[] (std::string &a_strArgName);
        CCliArg& operator[] (const char *a_szArgName);
        CCliArg& operator[] (unsigned int a_nIndex);

		int IsExist(std::string &a_strArgName);
		int IsExist(const char *a_szArgName);
        unsigned int GetCount() { return m_mapArg.size(); };
		void SetSessionId(unsigned int a_nSessionId) { m_nSessionId = a_nSessionId; };
		bool CmdLineIsExist(){ return m_strCmdLine.size()?true:false; };
		void SetCmdLine(std::string &a_strCmdLine){ m_strCmdLine = a_strCmdLine; };
		void SetCmdLine(const char *a_szCmdLine){ m_strCmdLine = a_szCmdLine; };
		std::string GetCmdLine() { return m_strCmdLine; };
        void SetPkgName(std::string &a_strPkgName) { m_strPkgName = a_strPkgName; };
        void SetPkgName(const char *a_szPkgName) { m_strPkgName = a_szPkgName; };
        void SetCmdName(std::string &a_strCmdName) { m_strCmdName = a_strCmdName; };
        void SetCmdName(const char *a_szCmdName) { m_strCmdName = a_szCmdName; };
        void SetCmdCode(unsigned int a_nCmdCode) { m_nCmdCode = a_nCmdCode; };
		unsigned int GetSessionId() { return m_nSessionId; };
        unsigned int GetCmdCode() { return m_nCmdCode; };
		bool PkgIsExist() { return m_strPkgName.size()?true:false;};
        std::string& GetPkgName() { return m_strPkgName; };
        std::string& GetCmdName() { return m_strCmdName; };
		CCliArg* GetArg(unsigned int a_nIndex);
		CCliArg* GetFirstArg();
		CCliArg* GetLastArg();
        void PopFirst();
        void PopLast();
        void Pop(std::string &a_strArgName);
        void Pop(const char *a_szArgName);
        void Pop(unsigned int a_nIndex);
        void PopAll();
        int EncodeMessage(std::vector<char> &a_strData);
        int EncodeMessage(std::string &a_strData);
        int DecodeMessage(std::vector<char> &a_strData);
        int DecodeMessage(std::string &a_strData);
};

#endif
