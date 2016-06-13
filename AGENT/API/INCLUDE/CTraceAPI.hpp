#ifndef CTRACE_API_HPP__
#define CTRACE_API_HPP__

#include <string>
#include <cerrno>
#include "CProtocol.hpp"
#include "TRM_Define.hpp"

#define DEF_BUFFER_128  128

using namespace std;

class CTraceAPI{

private:
	char* 		 space_remove(char *a_str);

public:
	CTraceAPI();
	~CTraceAPI();							// Destructor 

	CProtocol			 m_clsProtocol;

	ST_TRACE_REQUEST 	 *m_stTraceRequest;
	ST_TRACE_RESPONSE 	 m_stTraceResponse;
	ST_COWORK_INFO		 m_stCoworkInfo;		// Structure of Connection info

	vector<ST_TRACE_REQUEST *> m_vTraceRequest;

	unsigned int		 m_nRunMode;			// Debug (1) - Normal (2)
	unsigned int 		 m_nTraceCount;

	int 		 GetTraceCount() 		{ return m_nTraceCount;  };
	int 		 GetRunMode(int n) 		{ return m_vTraceRequest[n]->run_mode;  };
	int 		 GetSearchMode(int n) 	{ return m_vTraceRequest[n]->search_mode;    };
	char*		 GetKeyWord(int n)		{ return m_vTraceRequest[n]->keyword; 	};

	int			 GetTraceRequest(CProtocol *a_pclsProtocol, string a_strTraceRequest, int a_nSequence);
	string		 SetTraceResponse(ST_TRACE_REQUEST *a_tracerequest, bool a_traceOn, bool a_bRet);
	string		 SetTraceData(ST_TRACE_REQUEST *a_tracerequest, bool a_bRet, string a_strData);
	string 		 SetTraceProcess(string a_strPkgName, string a_strNode_type, string a_strProc_name, int a_nProcNo, string a_cmd, int a_node_no, string a_node_name);
	string		 SendTraceData(int a_nTraceCount, bool a_bRet, string a_strData);
	string 		 SetTraceResponseMsg(int a_nTraceCount, bool a_bTraceOn, bool a_bRet);
	int 		 SendMessage(string a_strMessage);
	bool 		 IsOn();
	int 		 CheckIMSI(string a_TargetString);
	int	 		 CheckMSISDN(string a_TargetString);
	int 		 CheckCallingIP(string a_TargetString);
	int 		 CheckCalledIP(string a_TargetString);
	int			 CheckString(int a_mode, string a_TargetString);
};

#endif	// CTRACE_API_HPP__
