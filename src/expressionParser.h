#pragma once

#include <map>
#include <string>

bool evaluateMSBuildExpression(std::string expression,
                               std::map<std::string, std::string> vars);