#include <percussa/od/glue/Interpreter.h>

#include <hal/log.h>

#include <cstdarg>
#include <cstdlib>
#include <cstdio>

extern "C"
{
#include "lauxlib.h"
#include "lualib.h"
}

namespace od
{
  Interpreter::Interpreter()
  {
    luaL_set_writestring_function(writeStringCallback);
    luaL_set_writestringerror_function(writeStringErrorCallback);
  }

  Interpreter::~Interpreter()
  {
  }

  void Interpreter::init(void)
  {
    L = lua_newstate(Interpreter::reallocateCallback, this);
    luaL_openlibs(L);
  }

  void Interpreter::onError(const char *format, ...)
  {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    logError("%s", buffer);

    if (!lua_isnil(L, -1))
    {
      logError("%s", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }

  bool Interpreter::execute(const char *code)
  {
    logInfo("Executing: %s", code);
    if (luaL_loadstring(L, code))
    {
      onError("Interpreter::executeString: Failed to load string");
      return false;
    }

    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    switch (result)
    {
    case 0:
      return true;
    case LUA_ERRRUN:
      onError("Interpreter::executeString: Error in pcall, runtime error.");
      return false;
    case LUA_ERRMEM:
      onError("Interpreter::executeString: Error in pcall, memory allocation error.");
      return false;
    case LUA_ERRERR:
      onError("Interpreter::executeString: Error in pcall, error while running error handler.");
      return false;
    default:
      return false;
    }
  }

  void *Interpreter::reallocateCallback(void *ud, void *ptr, size_t osize, size_t nsize)
  {
    (void)ud;
    (void)osize;
    return realloc(ptr, nsize);
  }

  void Interpreter::writeStringCallback(const char *buffer, size_t sz)
  {
    logWrite(buffer, (int)sz);
  }

  void Interpreter::writeStringErrorCallback(const char *s1, const char *s2)
  {
    logError("%s%s", s1, s2 ? s2 : "");
  }
}