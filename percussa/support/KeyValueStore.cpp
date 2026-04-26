#include <percussa/support/KeyValueStore.h>

#include <hal/log.h>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

namespace percussa
{
  namespace support
  {
    namespace
    {
      void split(const std::string &s, char delim, std::vector<std::string> &elems)
      {
        std::stringstream ss(s);
        std::string item;
        while (getline(ss, item, delim))
        {
          elems.push_back(item);
        }
      }

      bool startsWith(const std::string &text, const std::string &token)
      {
        return text.compare(0, token.length(), token) == 0;
      }
    } // namespace

    bool KeyValueStore::load(const std::string &fname)
    {
      std::ifstream f(fname);
      if (!f.is_open())
      {
        return false;
      }

      clear();

      int lineCount = 0;
      std::string line;
      std::vector<std::string> tokens;
      while (std::getline(f, line))
      {
        lineCount++;
        tokens.clear();
        split(line, ' ', tokens);
        if (tokens.empty())
        {
          continue;
        }
        if (startsWith(tokens[0], "#"))
        {
          continue;
        }
        if (tokens.size() == 1)
        {
          logWarn("%s:%d: found only 1 token '%s'", fname.c_str(), lineCount, line.c_str());
          continue;
        }
        if (tokens.size() == 2)
        {
          (*this)[tokens[0]] = tokens[1];
          continue;
        }
        logWarn("%s:%d: too many tokens '%s'", fname.c_str(), lineCount, line.c_str());
      }

      return true;
    }

    bool KeyValueStore::save(const std::string &fname)
    {
      std::ofstream f(fname);
      if (!f.is_open())
      {
        return false;
      }

      for (auto &kv : *this)
      {
        f << kv.first << " " << kv.second << '\n';
      }

      return true;
    }

    bool KeyValueStore::has(const std::string &key)
    {
      return find(key) != end();
    }

    const std::string &KeyValueStore::get(const std::string &key, const std::string &defaultValue)
    {
      auto i = find(key);
      if (i == end())
      {
        return defaultValue;
      }
      return i->second;
    }

    int KeyValueStore::getInteger(const std::string &key, int defaultValue)
    {
      auto i = find(key);
      if (i == end())
      {
        return defaultValue;
      }
      return atoi(i->second.c_str());
    }

    float KeyValueStore::getFloat(const std::string &key, float defaultValue)
    {
      auto i = find(key);
      if (i == end())
      {
        return defaultValue;
      }
      return (float)atof(i->second.c_str());
    }

    void KeyValueStore::setInteger(const std::string &key, int value)
    {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "%d", value);
      (*this)[key] = buffer;
    }

    void KeyValueStore::setFloat(const std::string &key, float value)
    {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "%f", value);
      (*this)[key] = buffer;
    }
  } // namespace support
} // namespace percussa