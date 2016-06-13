#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#define XML_PARSEING_FAILED 100
#define XML_XPATH_INIT_FAILED 101
#define XML_XPATH_PARSING_FAILED 102
#define XML_NODE_SET_NOT_EXIST 103
#define XML_NODE_NOT_EXIST 104
#define XML_NODE_CREATE_FAILED 105
#define XML_SAVE_FAILED  106
#define XML_NODE_ATTR_SET_FAILED 107
#define XML_ATTR_NOT_EXIT    108
#define XML_BUFFER_OVERFLOW 109

#define XML_OK 1
#define XML_NOK 0

#define XML_ERR 1

#define XML_LOG(_LVL,...) {} \

class CXML{
		private:
				xmlDocPtr m_stDoc;
				xmlXPathContextPtr m_stCtx;
				//MakeXPath();
				int GetNodeFromXPath(const char *a_chPath, xmlNodePtr *a_pstNode);
				int GetAttrPtr(const char *a_chPath, const char *a_chAttrName, int nIndex, xmlAttrPtr *a_stAttrPtr);
		public:
				CXML();
				~CXML();
				int ParseDoc(const char *path);
				int MakeDoc();	
				int SaveDoc(const char *a_chPath, int a_nIndent=1);

				int AddChildNode(const char *path, const char *nodeName);
				int DelNode(const char *a_chPath, const char *a_chNodeName=NULL, int nIndex=0);
				int SetNode(const char *a_chPath, const char *a_chNodeName, int nIndex=0);

				int AddAttr(const char *a_chPath, const char *a_chAttrName, const char *a_chValue);
				int SetAttr(const char *a_chPath, const char *a_chAttrName, int nIndex, const char *a_chValue);
				int DelAttr(const char *a_chPath, const char *a_chAttrName, int nIndex=1);
				int GetAttr(const char *a_chPath, const char *a_chAttrName, int nIndex, char *a_chValue, 
								int a_nMaxValueLen, int *a_nValueLen);

				int AddValue(const char *a_chPath, const char *a_chValue);
				int DelValue(const char *a_chPath);
				int SetValue(const char *a_chPath, const char *a_chValue);
				int GetValue(const char *a_chPath, char *a_chValue, int a_nMaxValueLen, int *a_nValueLen);
};
