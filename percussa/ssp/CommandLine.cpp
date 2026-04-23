#include <percussa/ssp/CommandLine.h>

#include <algorithm>

namespace ssp
{
  CommandLine::CommandLine(int &argc, char **argv)
  {
    for (int i = 1; i < argc; ++i)
    {
      tokens.push_back(std::string(argv[i]));
    }
  }

  const std::string &CommandLine::getOption(const std::string &option)
  {
    std::vector<std::string>::const_iterator itr = std::find(tokens.begin(), tokens.end(), option);
    if (itr != tokens.end() && ++itr != tokens.end())
    {
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }

  bool CommandLine::optionExists(const std::string &option)
  {
    return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
  }
}