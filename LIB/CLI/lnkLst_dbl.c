#include <stdio.h>
#include <unistd.h>

#include "lnkLst.h"

/* double linked list  */

/* Linked list macro (PRIVATE) */
#define CHECK_LNKLST(lnkLst){\
    if(lnkLst == NULL){\
        LNKLST_LOG(LNKLST_ERR,"lnkLst is NULL\n");\
        return LNKLST_ERR_INVALID_LNKLST;\
    }\
}

#define CHECK_LNKLST_P(lnkLst){\
	if(lnkLst == NULL){\
		LNKLST_LOG(LNKLST_ERR,"lnkLst is NULL\n");\
		return NULL;\
	}\
}

#define CHECK_LNKNODE(lnkNode){\
	if(lnkNode == NULL){\
		LNKLST_LOG(LNKLST_ERR,"LnkNode is NULL\n");\
		return LNKLST_ERR_INVALID_LNKNODE;\
	}\
}

#define CHECK_LNKENTCNT(lnkLst){\
	if(lnkLst->maxNode != LNKLST_NOT_LIMIT){\
		if(lnkLst->maxNode <= lnkLst->nodeCnt){\
			LNKLST_LOG(LNKLST_ERR,"LnkNode is Max\n");\
			return LNKLST_ERR_MAX_NODE;\
		}\
	}\
}

/*-------------------- double linked list functions --------------------*/
int lnkLst_dblInit(LnkLstDbl *lnkLst, unsigned int maxNode)
{
	/* check argument */
	CHECK_LNKLST(lnkLst);

	lnkLst->maxNode = maxNode;
	lnkLst->nodeCnt = 0;
	lnkLst->first = NULL;
	lnkLst->tail = NULL;

	return LNKLST_OK;
}

 int lnkLst_dblInsertFirst(LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);
	CHECK_LNKENTCNT(lnkLst);

	if(lnkLst->first){
		lnkNode->next = lnkLst->first;
		lnkLst->first->prev = lnkNode;
		lnkLst->first = lnkNode;
	}
	else {
		lnkLst->first = lnkNode;
		lnkLst->tail = lnkNode;
		lnkNode->next = NULL;
	}

	lnkNode->prev = NULL;

	lnkLst->nodeCnt++;
	return LNKLST_OK;
}

 int lnkLst_dblInsertTail(LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);
	CHECK_LNKENTCNT(lnkLst);

	/* insert LnkNode */
	if(lnkLst->tail){
		lnkNode->prev = lnkLst->tail;
		lnkLst->tail->next = lnkNode;
		lnkLst->tail = lnkNode;
	}
	else {
		lnkLst->first = lnkNode;
		lnkLst->tail = lnkNode;
		lnkNode->prev = NULL;
	}

	lnkNode->next = NULL;

	lnkLst->nodeCnt++;

	return LNKLST_OK;
}

 int lnkLst_dblInsertNextNode(LnkLstDbl *lnkLst, LnkLstDblNode *org, LnkLstDblNode *node)
{
	LnkLstDblNode *tmp = NULL;

	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(org);
	CHECK_LNKNODE(node);
	CHECK_LNKENTCNT(lnkLst);

	if(org->next == NULL){
		org->next = node;
		node->prev = org;
		node->next = NULL;
		lnkLst->tail = node;
	}
	else {
		tmp = org->next;

		org->next = node;
		node->prev = org;

		node->next = tmp;
		tmp->prev = node;
	}

	lnkLst->nodeCnt++;

	return LNKLST_OK;
}

 int lnkLst_dblInsertPrevNode(LnkLstDbl *lnkLst, LnkLstDblNode *org, LnkLstDblNode *node)
{
	LnkLstDblNode *tmp = NULL;

	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(org);
	CHECK_LNKNODE(node);
	CHECK_LNKENTCNT(lnkLst);

	if(org->prev == NULL){
		org->prev = node;
		node->next = org;
		node->prev = NULL;
		lnkLst->first = node;
	}
	else {
		tmp = org->prev;

		org->prev = node;
		node->next = org;

		node->prev = tmp;
		tmp->next = node;
	}

	lnkLst->nodeCnt++;

	return LNKLST_OK;
}

 LnkLstDblNode *lnkLst_dblGetFirst(LnkLstDbl *lnkLst)
{
    LnkLstDblNode *node;

    CHECK_LNKLST_P(lnkLst);

    if(lnkLst->nodeCnt == 0){
        LNKLST_LOG(LNKLST_ERR,"Linked list is empty\n");
        return NULL;
    }

    node = lnkLst->first;

    if(node->next == NULL){
        lnkLst->first = NULL;
        lnkLst->tail = NULL;
    }
    else{
        lnkLst->first = node->next;
        lnkLst->first->prev = NULL;
    }

    node->next = NULL;

    lnkLst->nodeCnt--;

    return node;
}

 LnkLstDblNode *lnkLst_dblGetTail(LnkLstDbl *lnkLst)
{
	LnkLstDblNode *node;

	CHECK_LNKLST_P(lnkLst);

	if(lnkLst->nodeCnt == 0){
		LNKLST_LOG(LNKLST_ERR,"Linked list is empty\n");
		return NULL;
	}

	node = lnkLst->tail;

	if(node->prev == NULL){
		lnkLst->first = NULL;
		lnkLst->tail = NULL;
	}
	else {
		lnkLst->tail = node->prev;
		lnkLst->tail->next = NULL;
	}

	node->prev = NULL;

	lnkLst->nodeCnt--;

	return node;
}

 int lnkLst_dblDel(LnkLstDbl *lnkLst, LnkLstDblNode *lnkNode)
{
	CHECK_LNKLST(lnkLst);
	CHECK_LNKNODE(lnkNode);

	/* check node count */
	if(lnkLst->nodeCnt == 0){
		LNKLST_LOG(LNKLST_ERR,"Linked list is empty\n");
		return LNKLST_ERR_LNKLST_IS_EMPTY;
	}

	/* if first data */
	if(lnkNode->prev == NULL){
		lnkLst->first = lnkNode->next;
		if(lnkLst->first){
			lnkLst->first->prev = NULL;
		}
	}

	/* if tail data */
	if(lnkNode->next == NULL){
		lnkLst->tail = lnkNode->prev;
		if(lnkLst->tail){
			lnkLst->tail->next = NULL;
		}
	}

	if(lnkNode->prev != NULL && lnkNode->next != NULL){
		lnkNode->next->prev = lnkNode->prev;
		lnkNode->prev->next = lnkNode->next;
	}

	lnkNode->prev = NULL;
	lnkNode->next = NULL;

	lnkLst->nodeCnt--;

	return LNKLST_OK;
}

