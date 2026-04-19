#include <functional>
#include <regex>
#include <string>

namespace s2cc::utils {
std::string replaceWithFunction(std::string str, std::regex reg,
                                std::function<std::string(std::smatch match)>);
}