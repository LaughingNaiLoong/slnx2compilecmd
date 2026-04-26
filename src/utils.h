#pragma once

#include <functional>
#include <regex>
#include <string>
#include <vector>

namespace s2cc::utils {
std::string replaceWithFunction(std::string str, std::regex reg,
                                std::function<std::string(std::smatch match)>);
std::vector<std::string> split(const std::string &s, const std::string &sep,
                               bool skipEmptyStr = false);
std::string join(std::vector<std::string>, std::string sep = " ");
} // namespace s2cc::utils