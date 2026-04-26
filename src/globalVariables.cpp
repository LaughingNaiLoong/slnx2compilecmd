#include "globalVariables.h"

std::map<std::string, std::string> globalVariables;

std::map<std::string, std::string> getGlobalVariables() {
  return globalVariables;
}
void setGlobalVariable(std::string key, std::string val) {
  globalVariables[key] = val;
}
void clearGlobalVariables() { globalVariables.clear(); }