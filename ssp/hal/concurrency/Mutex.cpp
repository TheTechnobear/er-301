#include <hal/concurrency/Mutex.h>

#include <pthread.h>
#include <unistd.h>

namespace od
{

  Mutex::Mutex()
  {
    pthread_mutex_t *h = new pthread_mutex_t;
    pthread_mutex_init(h, 0);
    mHandle = (void *)h;
  }

  Mutex::~Mutex()
  {
    pthread_mutex_t *h = (pthread_mutex_t *)mHandle;
    pthread_mutex_destroy(h);
    delete h;
  }

  void Mutex::enter()
  {
    pthread_mutex_t *h = (pthread_mutex_t *)mHandle;
    pthread_mutex_lock(h);
  }

  void Mutex::leave()
  {
    pthread_mutex_t *h = (pthread_mutex_t *)mHandle;
    pthread_mutex_unlock(h);
  }

  bool Mutex::tryEnter()
  {
    pthread_mutex_t *h = (pthread_mutex_t *)mHandle;
    return pthread_mutex_trylock(h) == 0;
  }

  bool Mutex::tryEnter(unsigned int timeout)
  {
    // Keep behavior simple and portable: spin/yield until timeout expires.
    if (timeout == 0)
    {
      return tryEnter();
    }

    for (unsigned int i = 0; i < timeout; i++)
    {
      if (tryEnter())
      {
        return true;
      }
      usleep(1000);
    }

    return false;
  }

} /* namespace od */
