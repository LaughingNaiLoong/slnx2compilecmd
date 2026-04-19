#include "macros.h"
#include "logging.h"
#include "utils.h"
#include <iostream>
#include <map>
#include <regex>
#include <string>

std::map<std::string, std::string> macros = {{"Platform", "x64"},
                                             {"Configuration", "Debug"}};

std::string replaceMacros(std::string data) {
  std::string res = s2cc::utils::replaceWithFunction(
      data, std::regex{R"(\$\(([^)]+)\))"}, [&](std::smatch m) -> std::string {
        if (macros.find(m[1].str()) != macros.end()) {
          std::string val = macros[m[1].str()];
          // LOG_DEBUG("macro {}=>{}", m[1].str(), val);
          return val;
        }
        LOG_WARN("Undefined macro: {}", m[1].str());
        return "";
      });
  return res;
}

void setMacro(std::string k, std::string v) { macros[k] = v; }