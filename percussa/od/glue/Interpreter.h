#pragma once

#include <memory>
#include <string>

extern "C"
{
#include "lua.h"
}

namespace od
{
  class Interpreter
  {
  public:
    Interpreter();
    virtual ~Interpreter();

    void init(void);
    bool execute(const char *code);

    template <typename... Args>
    bool execute(const std::string &format, Args... args)
    {
      int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
      if (size <= 0)
      {
        return false;
      }
      std::unique_ptr<char[]> buf(new char[(size_t)size]);
      snprintf(buf.get(), (size_t)size, format.c_str(), args...);
      return execute(buf.get());
    }

  protected:
    lua_State *L = nullptr;

    static void *reallocateCallback(void *ud, void *ptr, size_t osize, size_t nsize);
    static void writeStringCallback(const char *buffer, size_t sz);
    static void writeStringErrorCallback(const char *s1, const char *s2);

    void onError(const char *format, ...);
  };
}