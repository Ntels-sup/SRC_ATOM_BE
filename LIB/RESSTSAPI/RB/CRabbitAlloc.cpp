#include "CRabbitAlloc.hpp"

CRabbitAlloc::CRabbitAlloc()
{
		m_lstRabbitObject = NULL;
		m_lstRabbitArray = NULL;
}

CRabbitAlloc::~CRabbitAlloc()
{
        std::list<rabbit::object*>::iterator  objectIter;
        std::list<rabbit::array*>::iterator  arrayIter;

        if(m_lstRabbitObject != NULL){
                for(objectIter = m_lstRabbitObject->begin();objectIter != m_lstRabbitObject->end();){
                        delete *objectIter;
                        m_lstRabbitObject->erase(objectIter++);
                }

                delete m_lstRabbitObject;
        }

        if(m_lstRabbitArray != NULL){
                for(arrayIter = m_lstRabbitArray->begin();arrayIter != m_lstRabbitArray->end();){
                        delete *arrayIter;
                        m_lstRabbitArray->erase(arrayIter++);
                }

                delete m_lstRabbitArray;
        }
}

rabbit::object *CRabbitAlloc::GetRabbitObject()
{
        rabbit::object *cObject = NULL;

        if(m_lstRabbitObject == NULL){
                m_lstRabbitObject = new std::list<rabbit::object*>;
        }

        cObject = new rabbit::object;

        m_lstRabbitObject->push_back(cObject);

        return cObject;
}

rabbit::array *CRabbitAlloc::GetRabbitArray()
{
        rabbit::array *cArray = NULL;

        if(m_lstRabbitArray == NULL){
                m_lstRabbitArray = new std::list<rabbit::array*>;
        }

        cArray = new rabbit::array;

        m_lstRabbitArray->push_back(cArray);

        return cArray;
}
