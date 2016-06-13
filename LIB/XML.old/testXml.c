#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

int main()
{
	xmlDocPtr pDoc;
	xmlXPathContextPtr pCtx = NULL;
	xmlNodeSetPtr pNodeSet;
	xmlNodePtr pNode;
	xmlNodePtr newNode;
	xmlXPathObjectPtr pPathObj;
	char *data = NULL;
	char s[100];

	pDoc = xmlParseFile("./test.xml");

	/* path init */
	pCtx = xmlXPathNewContext(pDoc);

	pPathObj = xmlXPathEvalExpression("/TEST/VAL",pCtx);

	xmlXPathFreeContext(pCtx);

	pNodeSet = pPathObj->nodesetval;

	pNode = pNodeSet->nodeTab[0];

	data = xmlNodeListGetString(pDoc, pNode->xmlChildrenNode, 1);

	xmlNewProp(pNode,"ADD","ADDVAL");
	xmlNodeSetContent(pNode,"CONTENXT");

	newNode = xmlNewNode(NULL,"NEW");
	xmlNodeSetContent(newNode,"NEW DATA");

	xmlAddChild(pNode, newNode);


	fprintf(stderr,"data=%s\n",data);

xmlSaveFile("./new.xml",pDoc);

	return 0;
}
