#ifndef __LNKLST_H__
#define __LNKLST_H__

#define LNKLST_OK                            1
#define LNKLST_NOK                           0

#define LNKLST_NOT_LIMIT                     0

/* Linked list error code */
#define LNKLST_ERR_INVALID_LNKLST            101
#define LNKLST_ERR_INVALID_LNKNODE           102
#define LNKLST_ERR_MAX_NODE                  103
#define LNKLST_ERR_LNKLST_IS_EMPTY           104

/* double Linked list macro (PUBLIC) */
#define LNKLST_DBL_GET_FIRST(_lnkLst, _entry) (_entry) = (_lnkLst)->first;
#define LNKLST_DBL_GET_TAIL(_lnkLst, _entry) (_entry) = (_lnkLst)->tail;
#define LNKLST_DBL_GET_NEXT_NODE(_entry) (_entry) = (_entry)->next;
#define LNKLST_DBL_GET_PREV_NODE(_entry) (_entry) = (_entry)->prev;

#define LNKLST_ERR                           0
#define LNKLST_LEV1                          1
#define LNKLST_LEV2                          2

#if 0
#define LNKLST_LOG(_LVL,...){\
	fprintf(stderr,__VA_ARGS__);\
}
#else
#define LNKLST_LOG(_LVL,...){\
}
#endif

/* Linked list */
typedef struct LnkLstDbl         LnkLstDbl;
typedef struct LnkLstDblNode     LnkLstDblNode;

struct LnkLstDblNode{
		LnkLstDblNode       *prev;
		LnkLstDblNode       *next;
		void                *data;
};

/* double linked list */
struct LnkLstDbl{
		unsigned int        maxNode;    /* max node (0 : Not limited) */
		unsigned int        nodeCnt;    /* Linked list node count */
		LnkLstDblNode       *first;
		LnkLstDblNode       *tail;
};

/* Linked list public function */
int            lnkLst_dblInit            (LnkLstDbl *lnkLst, unsigned int maxNode);
int            lnkLst_dblInsertFirst     (LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode);
int            lnkLst_dblInsertTail      (LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode);
int            lnkLst_dblInsertNextNode  (LnkLstDbl *lnkLst, LnkLstDblNode *org, LnkLstDblNode *node);
int            lnkLst_dblInsertPrevNode  (LnkLstDbl *lnkLst, LnkLstDblNode *org, LnkLstDblNode *node);
LnkLstDblNode* lnkLst_dblGetFirst        (LnkLstDbl *lnkLst);
LnkLstDblNode* lnkLst_dblGetTail         (LnkLstDbl *lnkLst);
int            lnkLst_dblDel             (LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode);

#endif /* __LNKLST_H__ */
