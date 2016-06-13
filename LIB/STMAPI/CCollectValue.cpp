#include <stdio.h>
#include"CCollectValue.hpp"

CCollectValue::CCollectValue()
{
		m_lstPrimaryKey = NULL;
		m_lstValue = NULL;
}

CCollectValue::~CCollectValue()
{
		if(m_lstPrimaryKey != NULL){
				delete m_lstPrimaryKey;
		}

		if(m_lstValue != NULL){
				delete m_lstValue;
		}
}
