#include "CXML.hpp"

int main()
{
		CXML xml;
		int bufferLen = 0;
		char buffer[1024];


		xml.ParseDoc("./test.xml");

		xml.AddChildNode("/TEST","NEW_NODE");
		xml.AddChildNode("/TEST","NEW_NODE");

		//xml.DelNode("/TEST","NEW_NODE");
		//xml.DelNode("/TEST/NEW_NODE");
		//xml.DelNode("/TEST/NEW_NODE",NULL,2);
		//xml.DelNode("/TEST/NEW_NODE",NULL,1);


		//xml.AddAttr("/TEST/NEW_NODE","TEST_ATTR","10");
		//xml.AddAttr("/TEST/NEW_NODE","TEST_ATTR2","30");
		//xml.AddAttr("/TEST/NEW_NODE","TEST_ATTR","70");
		//xml.DelAttr("/TEST/NEW_NODE","TEST_ATTR",2);
		//xml.GetAttr("/TEST/NEW_NODE","TEST_ATTR",1, buffer,1024, &bufferLen);
		//printf("buffer=%s\n",buffer);
		//xml.SetAttr("/TEST/NEW_NODE","TEST_ATTR2",1,"90");

		//xml.SetValue("/TEST/NEW_NODE[2]","CONTENNTS");
		//xml.GetValue("/TEST/DEC",buffer,1024,&bufferLen);
		//printf("buffer2=%s\n",buffer);
		//xml.GetValue("/TEST/NEW_NODE[2]",buffer,1024,&bufferLen);
		//printf("buffer3=%s\n",buffer);
		//xml.DelValue("/TEST/NEW_NODE[2]");


		//xml.DelNode("/TEST/NEW_NODE",NULL,1);
		xml.SaveDoc("./new.xml",0);
		//xml.SaveDoc("-",3);

		return 0;
}
