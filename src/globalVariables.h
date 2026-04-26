#pragma once

#include <map>
#include <string>

std::map<std::string, std::string> getGlobalVariables();
void setGlobalVariable(std::string key, std::string val);