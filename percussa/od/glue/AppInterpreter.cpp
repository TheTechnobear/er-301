#include <percussa/od/glue/AppInterpreter.h>

#include <hal/dir.h>
#include <hal/fileops.h>

#include <set>
#include <string>
#include <vector>

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

  int luaopen_app(lua_State *L);

  static int dir_iter(lua_State *L)
  {
    char *fname;
    uint32_t attributes;
    dir_t *d = (dir_t *)lua_touserdata(L, lua_upvalueindex(1));

    while (Dir_read(*d, &fname, &attributes))
    {
      if (attributes & (FILEOPS_HID | FILEOPS_SYS))
      {
        continue;
      }
      else if (fname[0] == '.')
      {
        continue;
      }
      else
      {
        lua_pushstring(L, fname);
        return 1;
      }
    }
    return 0;
  }

  static int l_dir(lua_State *L)
  {
    const char *path = luaL_checkstring(L, 1);

    dir_t *d = (dir_t *)lua_newuserdata(L, sizeof(dir_t));

    luaL_getmetatable(L, "LuaBook.dir");
    lua_setmetatable(L, -2);

    *d = Dir_open(path);
    if (*d == 0)
    {
      luaL_error(L, "cannot open %s", path);
    }

    lua_pushcclosure(L, dir_iter, 1);
    return 1;
  }

  static int dir_gc(lua_State *L)
  {
    dir_t *d = (dir_t *)lua_touserdata(L, 1);
    if (*d)
    {
      Dir_close(*d);
    }
    return 0;
  }

  int luaopen_dir(lua_State *L)
  {
    luaL_newmetatable(L, "LuaBook.dir");

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, dir_gc);
    lua_settable(L, -3);

    lua_pushcfunction(L, l_dir);
    lua_setglobal(L, "dir");

    return 0;
  }

  static const luaL_Reg loadedlibs[] = {
      {"_G", luaopen_base},
      {LUA_LOADLIBNAME, luaopen_package},
      {LUA_COLIBNAME, luaopen_coroutine},
      {LUA_TABLIBNAME, luaopen_table},
      {LUA_IOLIBNAME, luaopen_io},
      {LUA_STRLIBNAME, luaopen_string},
      {LUA_MATHLIBNAME, luaopen_math},
      {LUA_DBLIBNAME, luaopen_debug},
      {NULL, NULL}};

  static void openStandardlibs(lua_State *L)
  {
    const luaL_Reg *lib;
    for (lib = loadedlibs; lib->func; lib++)
    {
      luaL_requiref(L, lib->name, lib->func, 1);
      lua_pop(L, 1);
    }
  }
}

namespace od
{
  AppInterpreter::AppInterpreter()
  {
  }

  AppInterpreter::~AppInterpreter()
  {
  }

  void AppInterpreter::init()
  {
    Interpreter::init();

    lua_gc(L, LUA_GCGEN, 0, 0);
    openStandardlibs(L);
    luaopen_dir(L);
    luaopen_app(L);

    execute("app.FIRMWARE_VERSION = '%s'", FIRMWARE_VERSION);
    execute("app.BUILD_PROFILE = '%s'", BUILD_PROFILE);
  }
}