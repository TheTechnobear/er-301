#include <ssp/tls.h>
#include <od/extras/ReferenceCounted.h>
#include <hal/concurrency/Thread.h>
//#define BUILDOPT_DEBUG_LEVEL 10
#include <hal/log.h>

#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#ifdef BUILDOPT_VERBOSE
#include <typeinfo>
#endif

namespace od
{

  struct ThreadRunner
  {
    static void *threadEntry(void *ptr)
    {
      od::Thread *thread = (od::Thread *)ptr;
      logAssert(thread);
      TLS_setName(thread->mName.c_str());
      logInfo("Thread starting.");
      thread->mThreadRunning = true;
      thread->run();
      thread->mThreadRunning = false;
      return 0;
    }
  };

  void Thread::sleep(uint32_t timeout)
  {
    usleep(timeout * 1000);
  }

  void Thread::yield()
  {
    sched_yield();
  }

  Thread::Thread(const char *name) : mName(name)
  {
    logDebug(1, "Thread(%s): constructor", name);
  }

  Thread::Thread(const char *name, int priority) : mName(name), mPriority(priority)
  {
    logDebug(1, "Thread(%s): constructor", name);
  }

  Thread::~Thread()
  {
    logDebug(1, "Thread(%s): destructor", mName.c_str());
    if (mThreadHandle)
    {
      stop();
      join();
    }
  }

  void Thread::start()
  {
#ifdef BUILDOPT_VERBOSE
    std::string classname = demangle(typeid(*this).name());
    logDebug(1, "%s(0x%x): start", classname.c_str(), this);
#endif
    pthread_t *thread = new pthread_t;
    if (pthread_create(thread, 0, ThreadRunner::threadEntry, (void *)this) != 0)
    {
      delete thread;
      mThreadHandle = 0;
      logError("Failed to create pthread.");
      return;
    }

    mThreadHandle = (void *)thread;
  }

  bool Thread::running()
  {
    return mThreadRunning;
  }

  void Thread::stop()
  {
#ifdef BUILDOPT_VERBOSE
    std::string classname = demangle(typeid(*this).name());
    logDebug(1, "%s(0x%x): requesting stop", classname.c_str(), this);
#endif
    mEvents.post(onThreadQuit);
  }

  void Thread::join()
  {
    if (mThreadHandle)
    {
    pthread_t *thread = (pthread_t *)mThreadHandle;
    pthread_join(*thread, 0);
    delete thread;
    mThreadHandle = 0;
#ifdef BUILDOPT_VERBOSE
      logDebug(1, "%s(0x%x): stopped", classname.c_str(), this);
#endif
    }
    else
    {
      logError("Trying to join a thread that has not started.");
    }
  }

} /* namespace od */
