#include <hal/concurrency/EventFlags.h>

#include <errno.h>
#include <pthread.h>
#include <time.h>

static constexpr uint32_t kWaitForever = 0xFFFFFFFFu;

namespace od
{
  struct Pimp
  {
    Pimp()
    {
      mMutex = new pthread_mutex_t;
      mCond = new pthread_cond_t;
      pthread_mutex_init(mMutex, 0);
      pthread_cond_init(mCond, 0);
    }

    ~Pimp()
    {
      pthread_cond_destroy(mCond);
      pthread_mutex_destroy(mMutex);
      delete mCond;
      delete mMutex;
    }

    uint32_t pend(uint32_t all, uint32_t any, uint32_t timeout)
    {
      uint32_t matching;
      pthread_mutex_lock(mMutex);
      while (!(matching = check(all, any)))
      {
        if (timeout == kWaitForever)
        {
          pthread_cond_wait(mCond, mMutex);
          continue;
        }

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        ts.tv_nsec += (long)(timeout % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L)
        {
          ts.tv_sec += 1;
          ts.tv_nsec -= 1000000000L;
        }

        if (pthread_cond_timedwait(mCond, mMutex, &ts) == ETIMEDOUT)
        {
          break;
        }
      }
      pthread_mutex_unlock(mMutex);
      return matching;
    }

    void post(uint32_t flags)
    {
      pthread_mutex_lock(mMutex);
      mPosted |= flags;
      pthread_cond_signal(mCond);
      pthread_mutex_unlock(mMutex);
    }

    uint32_t getPosted()
    {
      uint32_t posted;
      pthread_mutex_lock(mMutex);
      posted = mPosted;
      pthread_mutex_unlock(mMutex);
      return posted;
    }

  private:
    pthread_mutex_t *mMutex;
    pthread_cond_t *mCond;
    uint32_t mPosted = 0;

    uint32_t check(uint32_t andMask, uint32_t orMask)
    {
      uint32_t matching = orMask & mPosted;
      if ((andMask & mPosted) == andMask)
      {
        matching |= andMask;
      }

      if (matching)
      {
        mPosted &= ~matching;
      }

      return matching;
    }
  };

  EventFlags::EventFlags()
  {
    mHandle = (void *)new Pimp();
  }

  EventFlags::~EventFlags()
  {
    delete (Pimp *)mHandle;
  }

  void EventFlags::clear(uint32_t flags)
  {
    Pimp *pimp = (Pimp *)mHandle;
    pimp->pend(0, flags, 0);
  }

  void EventFlags::post(uint32_t flags)
  {
    Pimp *pimp = (Pimp *)mHandle;
    pimp->post(flags);
  }

  uint32_t EventFlags::getPosted()
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->getPosted();
  }

  uint32_t EventFlags::waitForAny(uint32_t flags)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(0, flags, kWaitForever);
  }

  uint32_t EventFlags::waitForAny(uint32_t flags, uint32_t timeout)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(0, flags, timeout);
  }

  uint32_t EventFlags::waitForAll(uint32_t flags)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(flags, 0, kWaitForever);
  }

  uint32_t EventFlags::waitForAll(uint32_t flags, uint32_t timeout)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(flags, 0, timeout);
  }

  uint32_t EventFlags::wait(uint32_t allFlags, uint32_t anyFlags)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(allFlags, anyFlags, kWaitForever);
  }

  uint32_t EventFlags::wait(uint32_t allFlags, uint32_t anyFlags, uint32_t timeout)
  {
    Pimp *pimp = (Pimp *)mHandle;
    return pimp->pend(allFlags, anyFlags, timeout);
  }
}