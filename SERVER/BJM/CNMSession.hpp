#ifndef __CNMSession_hpp__
#define __CNMSession_hpp__

#include <vector>
//#include <map>
#include <iostream>
#include <cstring>
#include "BJM_Define.hpp"
#include "CBatchHist.hpp"
#include "CMesgExchSocket.hpp"
#include "rabbit.hpp"

using namespace std;

class CNMSession
{
private:
	DB *            	m_pDB;
    CBatchHist      	m_BatchHist;
	rabbit::object  	m_root;

	int 				m_nStartFlag;
	int 				m_nResponseSeq;

	int 				SetRequestMsg(ST_BatchRequest *a_batchrequest);
	int 				GetRequestMsg(ST_BatchRequest *a_batchrequest);
    char*               space_remove(char *a_str);
	int 				MsgSend(string a_strPayload);

public:
    CNMSession() ;
    ~CNMSession() ;

	CMesgExchSocket 	m_sock;
	ST_BatchRequest 	*m_stbatchrequest;
	vector<ST_BatchRequest *> m_vBatchRequest;

    int                 Initial();
    int                 Final  ();
    int                 Run    ();
	int					IsRequest();
	int 				ProcessCtrl();
	int					GetSequence();
	int					Regist(ST_COWORK_INFO *a_coworkinfo);
	int					SendInitResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_BatchRequest *a_batchrequest);
	int					SendWSMResponseMsg(int ret, ST_COWORK_INFO *a_coworkinfo, ST_BatchRequest *a_batchrequest, ST_BatchResponse *a_batchresponse);
	int					SendRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_BatchRequest *a_batchrequest);
	int					SendBackupRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_BACKUP_REQUEST *a_backuprequest);
	int					SendRestoreRequestMsg(ST_COWORK_INFO *a_coworkinfo, ST_BatchJob *batchJobInfo_, ST_RESTORE_REQUEST *a_restorerequest);
	int					RecvMsg(DB * a_pDB, ST_BatchRequest *a_batchrequest, ST_BatchResponse *a_batchresponse);
	int 				GetResponseMsg(ST_BatchResponse *a_batchresponse, vector<ST_BatchRequest *> a_vBatchRequest);
	int 				GetBackupRequest(ST_BACKUP_REQUEST *a_backuprequest);
	int 				SetBackupRequest(ST_BACKUP_REQUEST *a_backuprequest);
	int 				GetBackupResponse(ST_COWORK_INFO *a_coworkinfo, ST_BACKUP_REQUEST *a_backuprequest);
	int 				SetBackupResponse(ST_COWORK_INFO *a_coworkinfo, ST_BACKUP_REQUEST *a_backuprequest, bool a_ret);
	int 				GetRestoreRequest(ST_RESTORE_REQUEST *a_restorerequest);
	int 				SetRestoreRequest(ST_RESTORE_REQUEST *a_restorerequest);
	int 				GetRestoreResponse(ST_COWORK_INFO *a_coworkinfo, ST_RESTORE_REQUEST *a_restorerequest);
	int 				SetRestoreResponse(ST_COWORK_INFO *a_coworkinfo, ST_RESTORE_REQUEST *a_restorerequest, bool a_ret);
};

#endif

