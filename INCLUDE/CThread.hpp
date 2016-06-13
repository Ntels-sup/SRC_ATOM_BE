#include <pthread.h>
 
class CThread {
    protected:
        pthread_t mThread;
        bool threadRunFlag;

    public:
        CThread ();
        virtual ~CThread ();
        virtual void Run ()=0;
        static void* Run_ (void*);
        void Start ();
        void Stop ();
        bool IsRun();
};
