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

std::vector<std::string> s2cc::utils::split(const std::string &s,
                                            const std::string &sep,
                                            bool skipEmptyStr) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = s.find(sep);
  while (end != std::string::npos) {
    std::string token = s.substr(start, end - start);
    if (!skipEmptyStr || !token.empty())
      tokens.push_back(std::move(token));
    start = end + sep.length();
    end = s.find(sep, start);
  }
  std::string last_token = s.substr(start);
  if (!skipEmptyStr || !last_token.empty())
    tokens.push_back(std::move(last_token));
  return tokens;
}

std::string s2cc::utils::join(std::vector<std::string> v, std::string sep) {
  std::string res = "";
  for (auto s : v) {
    res += s + sep;
  }
  return res;
}