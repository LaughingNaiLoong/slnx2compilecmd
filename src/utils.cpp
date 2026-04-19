#include "utils.h"
#include <regex>

std::string
s2cc::utils::replaceWithFunction(std::string str, std::regex reg,
                                 std::function<std::string(std::smatch)> func) {
  std::string result;
  std::sregex_iterator it(str.begin(), str.end(), reg);
  std::sregex_iterator end;

  size_t lastPos = 0;
  for (; it != end; ++it) {
    std::smatch match = *it;
    size_t matchStart = match.position();
    result.append(str, lastPos, matchStart - lastPos);
    result.append(func(match));
    lastPos = matchStart + match.length();
  }
  result.append(str, lastPos, str.length() - lastPos);
  return result;
}