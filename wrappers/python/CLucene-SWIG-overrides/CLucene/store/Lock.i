/* Reviewed 2004.08.20: necessary */

namespace lucene{ namespace store{
  class LuceneLock{
  public:
    virtual bool obtain() = 0;
    virtual void release() = 0;
    virtual ~LuceneLock();
  };

  class LuceneLockWith {
  /* DSR: Force lucene to consider this an abstract class. */
  private:
    LuceneLockWith();

  public:
    ~LuceneLockWith();
    void* run();
  };
}}