#include "CLucene/StdHeader.h"
#include "Lock.h"

namespace lucene{ namespace store{

    //Calls {@link #doBody} while <i>lock</i> is obtained.  Blocks if lock
    //cannot be obtained immediately.  Retries to obtain lock once per second
    //until it is obtained, or until it has tried ten times.
    void* LuceneLockWith::run() {
        bool locked = false;
      void* ret = NULL;
#ifndef CLUCENE_LITE
        _TRY {
          locked = lock->obtain();

          int_t sleepCount = 0;
            while (!locked) {
                if (++sleepCount == maxSleeps) {
                  _THROWC("Timed out waiting for lock.");
              }

              processSleep(sleepInterval);

                locked = lock->obtain();
            }
#endif
            ret = doBody();
#ifndef CLUCENE_LITE
          }_FINALLY(
              if (locked) lock->release();
          );
#endif
      return ret;
      }
}}
