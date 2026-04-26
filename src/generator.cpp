#include "expressionParser.h"
#include "globalVariables.h"
#include "logging.h"
#include "nlohmann/detail/macro_scope.hpp"
#include "utils.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <regex>
#include <string>
#include <unordered_map>

#define FORMAT_ERROR(...)                                                      \
  LOG_ERROR("Format error: {}", __VA_ARGS__);                                  \
  return 1;

using namespace nlohmann;

namespace fs = std::filesystem;

struct Command {
  std::string command;
  std::string directory;
  std::string file;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Command, command, directory, file);
};

#define _GET_OPTION(NAME, FUNC)                                                \
  {                                                                            \
    auto n = clOptions.child(NAME);                                            \
    if (!n.empty()) {                                                          \
      std::string value = n.child_value();                                     \
      (FUNC)();                                                                \
    }                                                                          \
  }

std::string clearUnusedExpression(std::string s) {
  return std::regex_replace(s, std::regex{R"(\%\(([^)]+)\))"}, "");
}
fs::path absPath(fs::path base, fs::path p) {
  return p.is_absolute() ? p : (base / p);
}

/**
false - <none>

Async - /EHa

Sync - /EHsc

SyncCThrow - /EHs
*/
std::unordered_map<std::string, std::string> errorHandleringTypes = {
    {"false", ""},
    {"Async", "/EHa"},
    {"Sync", "/EHsc"},
    {"SyncCThrow", "/EHs"}};

/**
MultiThreaded - /MT

MultiThreadedDebug - /MTd

MultiThreadedDLL - /MD

MultiThreadedDebugDLL - /MDd
*/
std::unordered_map<std::string, std::string> runtimeLibTypes = {
    {"MultiThreaded", "/MT"},
    {"MultiThreadedDebug", "/MTd"},
    {"MultiThreadedDLL", "/MD"},
    {"MultiThreadedDebugDLL", "/MDd"}};

std::vector<Command> parseVcxproj(fs::path projectPath, fs::path slnxRoot) {
  LOG_INFO("Processing project: {}", projectPath.string());
  std::vector<Command> result{};
  std::map<std::string, std::string> variables;
  std::vector<std::string> args = {"cl.exe"};
  auto global = getGlobalVariables();

  std::copy(global.begin(), global.end(),
            std::inserter(variables, variables.begin()));

  variables.insert({{"P", ""}});

  pugi::xml_document project;
  if (!project.load_file(projectPath.c_str())) {
    LOG_ERROR("Couldn't load file '{}'", projectPath.string());
    return {};
  }

  pugi::xml_node clOptions;

  for (auto r : project.select_nodes("//ItemDefinitionGroup")) {
    if (auto node = r.node()) {
      std::string expr = node.attribute("Condition").as_string();
      LOG_DEBUG("{}", expr);
      if (expr.empty() || !evaluateMSBuildExpression(expr, variables))
        continue;
      else {
        if (auto options = node.child("ClCompile")) {
          clOptions = options;
          LOG_DEBUG("Expression \"{}\" returns true.", expr);
          break;
        }
      }
    }
  }

  if (!clOptions) {
    LOG_WARN("[{}]: No matching configuration", projectPath.string());
    return result;
  }

  // Options

  _GET_OPTION("AdditionalOptions",
              [&]() { args.push_back(clearUnusedExpression(value)); });

  _GET_OPTION("AdditionalIncludeDirectories", [&]() {
    for (std::string i :
         s2cc::utils::split(clearUnusedExpression(value), ";", true)) {
      args.push_back("/I\"" + i + "\"");
    }
  });

  _GET_OPTION("PreprocessorDefinitions", [&]() {
    for (std::string i :
         s2cc::utils::split(clearUnusedExpression(value), ";", true)) {
      args.push_back("/D\"" +
                     std::regex_replace(i, std::regex{R"(\")"}, "\\\"") + "\"");
    }
  });

  _GET_OPTION("LanguageStandard", [&]() {
    args.push_back(
        std::regex_replace(value, std::regex{R"(stdcpp)"}, "/std:c++"));
  });

  _GET_OPTION("ExceptionHandling",
              [&]() { args.push_back(errorHandleringTypes[value]); });

  _GET_OPTION("RuntimeLibrary",
              [&]() { args.push_back(runtimeLibTypes[value]); });

  for (auto r : project.select_nodes("//ItemGroup/ClCompile[@Include]")) {
    if (auto node = r.node()) {
      auto src = absPath(projectPath.parent_path(),
                         node.attribute("Include").as_string());
      LOG_DEBUG("Found source file: {}", src.string());
      Command cmd{
          std::format("{} \"{}\"", s2cc::utils::join(args), src.string()),
          src.parent_path().string(), src.string()};
      result.push_back(cmd);
    }
  }

  return result;
}

int generateCompileCommands(const std::string &slnxPath,
                            const std::string &outputPath) {
  fs::path slnxRoot = fs::absolute(slnxPath).parent_path();
  pugi::xml_document slnx;
  std::vector<Command> allResult;
  if (!slnx.load_file(slnxPath.c_str())) {
    LOG_ERROR("Couldn't load file '{}'", slnxPath);
    return 1;
  }
  auto solution = slnx.child("Solution");
  if (!solution) {
    FORMAT_ERROR("The file is broken.");
  }
  for (auto project : solution.children("Project")) {
    auto p = absPath(slnxRoot, project.attribute("Path").as_string(""));
    if (!fs::exists(p)) {
      LOG_ERROR("Project not found: {}", p.string());
      return 1;
    }
    auto v = parseVcxproj(p, slnxRoot);
    allResult.insert(allResult.end(), v.begin(), v.end());
  }
  json j = allResult;
  std::ofstream ofs(outputPath);
  if (ofs.is_open()) {
    ofs << j.dump(4);
    ofs.close();
    LOG_INFO("\033[92mGenerated {} commands.\033[0m", allResult.size());
  } else
    LOG_ERROR("Unable to open file.");
  return 0;
}