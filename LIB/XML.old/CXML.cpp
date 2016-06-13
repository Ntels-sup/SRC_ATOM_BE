#include <string.h>
#include "CXML.hpp"

CXML::CXML()
{
		m_stDoc = NULL;
		m_stCtx = NULL;
}

CXML::~CXML()
{
}

int CXML::GetNodeFromXPath(const char *a_chPath, xmlNodePtr *a_pstNode)
{
		xmlXPathObjectPtr pPathObj = NULL;
		xmlNodeSetPtr pNodeSet = NULL;
		xmlNodePtr pNode = NULL;

		pPathObj = xmlXPathEvalExpression((const xmlChar*)a_chPath, m_stCtx);
		if(pPathObj == NULL){
				XML_LOG(XML_ERR,"XPath parsing failed(path=%s, name=%s)\n", a_chPath, a_chAttrName);
				return XML_XPATH_PARSING_FAILED;
		}

		if(pPathObj->nodesetval == NULL){
				XML_LOG(XML_ERR,"NodeSet not exit(path=%s, name=%s)\n", a_chPath, a_chAttrName);
				return XML_NODE_SET_NOT_EXIST;
		}
		pNodeSet = pPathObj->nodesetval;

		if(pNodeSet->nodeTab == NULL){
				XML_LOG(XML_ERR,"Node not exit(path=%s, name=%s)\n", a_chPath, a_chAttrName);
				return XML_NODE_NOT_EXIST;
		}
		pNode = pNodeSet->nodeTab[0];
		if(pNode == NULL){
				XML_LOG(XML_ERR,"Node not exit(path=%s, name=%s)\n", a_chPath, a_chAttrName);
				return XML_NODE_NOT_EXIST;
		}

		*a_pstNode = pNode;

		return XML_OK;
}

int CXML::GetAttrPtr(const char *a_chPath, const char *a_chAttrName, int nIndex, xmlAttrPtr *a_stAttrPtr)
{
		int nRet = XML_OK;
		int i = 0;
		xmlAttrPtr pAttr = NULL;
		xmlAttrPtr pFindAttr = NULL;
		xmlNodePtr pNode = NULL;

		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		pAttr = xmlHasProp(pNode, (const xmlChar*)a_chAttrName);

		if(nIndex < 1){
				nIndex = 1;
		}

		for(i=1;(pAttr != NULL);pAttr=pAttr->next){
				if(strcasecmp((const char*)pAttr->name,a_chAttrName) == 0){
						if(nIndex == i){
								*a_stAttrPtr = pAttr;
								return XML_OK;
						}
						else {
								i++;
						}
				}
		}
		XML_LOG(XML_ERR,"Attribute not exit(path=%s, attrName=%s, index=%d)\n",a_chPath, a_chPath, nIndex);
		return XML_ATTR_NOT_EXIT;
}

int CXML::ParseDoc(const char *path)
{
		m_stDoc = xmlParseFile(path);
		if(m_stDoc == NULL){
				XML_LOG(XML_ERR,"XML Parsing failed(path=%s)\n", path);
				return XML_PARSEING_FAILED;
		}

		/* XPATH Init */
		m_stCtx = xmlXPathNewContext(m_stDoc);
		if(m_stCtx == NULL){
				XML_LOG(XML_ERR,"XPATH Init failed(path=%s)\n", path);
				return XML_XPATH_INIT_FAILED;
		}

		return XML_OK;
}
int CXML::MakeDoc()
{
		if(m_stCtx != NULL){
				xmlXPathFreeContext(m_stCtx);
		}

		if(m_stDoc != NULL){
				xmlFreeDoc(m_stDoc);
		}

		return XML_OK;
}

int CXML::SaveDoc(const char *a_chPath, int a_nIndent)
{
		int nRet = 0;

		nRet = xmlSaveFormatFile(a_chPath, m_stDoc, a_nIndent);
		if(nRet < 0){
				return XML_SAVE_FAILED;
		}

		return XML_OK;
}

int CXML::AddChildNode(const char *a_chPath, const char *a_chNodeName)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;
		xmlNodePtr pNewNode = NULL;

		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		pNewNode = xmlNewNode(NULL,(const xmlChar*)a_chNodeName);
		if(pNewNode == NULL){
				XML_LOG(XML_ERR,"Node create failed(path=%s, name=%s)\n", a_chPath, a_chNodeName);
				return XML_NODE_CREATE_FAILED;
		}

		xmlAddChild(pNode, pNewNode);

		return XML_OK;
}

int CXML::DelNode(const char *a_chPath, const char *a_chNodeName, int nIndex)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;
		char *chPathBuffer = NULL;
		int chPathBufferLen = 0;

		/* make path */
		if( a_chNodeName != NULL){
				chPathBufferLen = strlen(a_chPath) + 1/* / */;
				chPathBufferLen += strlen(a_chNodeName);

				if(nIndex > 1){
						chPathBufferLen += 2; /* [ ] */
						chPathBufferLen += 11; /* number length */
				}

				chPathBufferLen += 1; /* NULL */

				chPathBuffer = new char[chPathBufferLen];

				if(nIndex == 0){
						snprintf(chPathBuffer,chPathBufferLen,"%s/%s",a_chPath, a_chNodeName);
				}
				else {
						snprintf(chPathBuffer,chPathBufferLen,"%s/%s[%d]",a_chPath, a_chNodeName, nIndex);
				}
		}
		else {
				if(nIndex > 1){
						chPathBufferLen = strlen(a_chPath);
						chPathBufferLen += 2; /* [ ] */
						chPathBufferLen += 11; /* number length */

						chPathBufferLen += 1; /* NULL */

						chPathBuffer = new char[chPathBufferLen];
						snprintf(chPathBuffer,chPathBufferLen,"%s[%d]",a_chPath, nIndex);
				}
				else {
						chPathBufferLen = 0;
						chPathBuffer = (char*)a_chPath;
				}
		}

		nRet = GetNodeFromXPath(chPathBuffer, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		xmlUnlinkNode(pNode);
		xmlFreeNode(pNode);

		if(chPathBufferLen != 0){
				delete[] chPathBuffer;
		}

		return XML_OK;
}

int CXML::AddAttr(const char *a_chPath, const char *a_chAttrName, const char *a_chValue)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;

		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		if(xmlNewProp(pNode, (const xmlChar*)a_chAttrName, (const xmlChar*)a_chValue) == NULL){
				XML_LOG(XML_ERR,"Property setting failed (path=%s, name=%s)\n", a_chPath, a_chAttrName);
				return XML_NODE_ATTR_SET_FAILED;
		}

		return XML_OK;
}

int CXML::SetAttr(const char *a_chPath, const char *a_chAttrName, int nIndex, const char *a_chValue)
{
		int nRet = XML_OK;
		xmlAttrPtr pAttr = NULL;

		nRet = GetAttrPtr(a_chPath, a_chAttrName, nIndex, &pAttr);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Attribute not exist(ret=%d)\n",nRet);
				return nRet;
		}

		xmlNodeSetContent(pAttr->children, (const xmlChar*)a_chValue);

		return XML_OK;
}

int CXML::DelAttr(const char *a_chPath, const char *a_chAttrName, int nIndex)
{
		int nRet = XML_OK;
		int i = 0;
		xmlAttrPtr pAttr = NULL;
		xmlAttrPtr pFindAttr = NULL;
		xmlNodePtr pNode = NULL;

		nRet = GetAttrPtr(a_chPath, a_chAttrName, nIndex, &pAttr);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Attribute not exist(ret=%d)\n",nRet);
				return nRet;
		}

		xmlRemoveProp(pAttr);

		return XML_OK;
}

int CXML::GetAttr(const char *a_chPath, const char *a_chAttrName, int nIndex, char *a_chValue, 
				int a_nMaxValueLen, int *a_nValueLen)
{
		int nRet = XML_OK;
		int nLen = 0;
		char *chAttrValue = NULL;
		xmlAttrPtr pAttr = NULL;

		nRet = GetAttrPtr(a_chPath, a_chAttrName, nIndex, &pAttr);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Attribute not exist(ret=%d)\n",nRet);
				return nRet;
		}

		chAttrValue = (char*)xmlNodeListGetString(m_stDoc, pAttr->children, 1);

		nLen = strlen(chAttrValue);
		if(nLen > a_nMaxValueLen){
				XML_LOG(XML_ERR,"Attribute buffer overflow\n");
				return XML_BUFFER_OVERFLOW;
		}

		strncpy(a_chValue, chAttrValue, nLen);

		*a_nValueLen = nLen;
		return XML_OK;
}

int CXML::AddValue(const char *a_chPath, const char *a_chValue)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;
		
		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		xmlNodeAddContent(pNode, (const xmlChar*)a_chValue);

		return XML_OK;
}

int CXML::DelValue(const char *a_chPath)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;

		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		if(pNode->xmlChildrenNode == NULL){
				return XML_OK;
		}

		pNode = pNode->xmlChildrenNode;
		xmlUnlinkNode(pNode);
		xmlFreeNode(pNode);

		return XML_OK;
}

int CXML::SetValue(const char *a_chPath, const char *a_chValue)
{
		int nRet = XML_OK;
		xmlNodePtr pNode = NULL;
		
		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		xmlNodeSetContent(pNode, (const xmlChar*)a_chValue);

		return XML_OK;
}

int CXML::GetValue(const char *a_chPath, char *a_chValue, int a_nMaxValueLen, int *a_nValueLen)
{
		int nRet = XML_OK;
		int nLen = 0;
		xmlNodePtr pNode = NULL;
		char *chNodeValue = NULL;

		nRet = GetNodeFromXPath(a_chPath, &pNode);
		if(nRet != XML_OK){
				XML_LOG(XML_ERR,"Node not exist(ret=%d, path=%s, attrName=%s)\n",nRet, a_chPath, a_chAttrName);
				return nRet;
		}

		chNodeValue = (char*)xmlNodeListGetString(m_stDoc, pNode->xmlChildrenNode, 1);

		nLen = strlen(chNodeValue);
		if(nLen > a_nMaxValueLen){
				XML_LOG(XML_ERR,"Node buffer overflow\n");
				return XML_BUFFER_OVERFLOW;
		}

		strncpy(a_chValue, chNodeValue, nLen);

		*a_nValueLen = nLen;
		return XML_OK;
}
