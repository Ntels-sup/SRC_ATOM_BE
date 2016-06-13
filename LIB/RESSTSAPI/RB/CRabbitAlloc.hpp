#ifndef __CRABBITALLOC_HPP__
#define __CRABBITALLOC_HPP__
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"
#include <list>

class CRabbitAlloc{
		private:
				std::list<rabbit::object*> *m_lstRabbitObject;
				std::list<rabbit::array*> *m_lstRabbitArray;

		public:
				CRabbitAlloc();
				~CRabbitAlloc();
				rabbit::object *GetRabbitObject();
				rabbit::array *GetRabbitArray();
};

#endif
