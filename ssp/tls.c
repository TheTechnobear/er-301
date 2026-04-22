#include <ssp/tls.h>

#include <pthread.h>

static pthread_key_t thread_local_storage;
static pthread_key_t thread_name;
static pthread_once_t tls_once = PTHREAD_ONCE_INIT;

static void initializeTLS(void)
{
  pthread_key_create(&thread_local_storage, 0);
  pthread_key_create(&thread_name, 0);
}

void TLS_set(void *value)
{
  pthread_once(&tls_once, initializeTLS);
  pthread_setspecific(thread_local_storage, value);
}

void *TLS_get(void)
{
  pthread_once(&tls_once, initializeTLS);
  return pthread_getspecific(thread_local_storage);
}

void TLS_setName(const char *value)
{
  pthread_once(&tls_once, initializeTLS);
  pthread_setspecific(thread_name, value);
}

const char *TLS_getName()
{
  pthread_once(&tls_once, initializeTLS);
  return (const char *)pthread_getspecific(thread_name);
}