#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <string>
#include <vector>

using namespace nlohmann;

namespace fs = std::filesystem;

std::string make_absolute(const std::string &path, const fs::path &base_path) {
  fs::path p(path);
  if (p.is_absolute()) {
    return p.string();
  }
  return (base_path / p).lexically_normal().string();
}

std::string trim(const std::string &str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, last - first + 1);
}

std::map<std::string, std::vector<std::string>>
parse_vcxproj(const fs::path &vcxproj_path, const fs::path &solution_dir) {

  std::map<std::string, std::vector<std::string>> result;
  pugi::xml_document doc;
  if (!doc.load_file(vcxproj_path.string().c_str())) {
    std::cerr << "Failed to load vcxproj: " << vcxproj_path << std::endl;
    return result;
  }

  fs::path project_dir = vcxproj_path.parent_path();

  std::vector<std::string> global_include_dirs;
  std::vector<std::string> global_defines;
  std::string language_standard;

  auto split_semicolon =
      [](const std::string &str) -> std::vector<std::string> {
    std::vector<std::string> result;
    size_t start = 0, end = 0;
    while ((end = str.find(';', start)) != std::string::npos) {
      std::string token = trim(str.substr(start, end - start));
      if (!token.empty())
        result.push_back(token);
      start = end + 1;
    }
    std::string last = trim(str.substr(start));
    if (!last.empty())
      result.push_back(last);
    return result;
  };

  pugi::xml_node cl_compile_settings;

  for (pugi::xml_node prop_group :
       doc.child("Project").children("PropertyGroup")) {
    if (prop_group.attribute("Condition") &&
        !std::string(prop_group.attribute("Condition").value()).empty()) {

      continue;
    }
    pugi::xml_node cl_node = prop_group.child("ClCompile");
    if (cl_node) {
      cl_compile_settings = cl_node;
      break;
    }
  }

  if (!cl_compile_settings) {
    for (pugi::xml_node prop_group :
         doc.child("Project").children("PropertyGroup")) {
      pugi::xml_node cl_node = prop_group.child("ClCompile");
      if (cl_node) {
        cl_compile_settings = cl_node;
        break;
      }
    }
  }

  if (cl_compile_settings) {

    if (auto inc = cl_compile_settings.child("AdditionalIncludeDirectories")) {
      std::string inc_str = inc.child_value();
      auto dirs = split_semicolon(inc_str);
      for (auto &dir : dirs) {

        std::string abs_dir = make_absolute(dir, project_dir);
        global_include_dirs.push_back("/I\"" + abs_dir + "\"");
      }
    }

    if (auto def = cl_compile_settings.child("PreprocessorDefinitions")) {
      std::string def_str = def.child_value();
      auto defines = split_semicolon(def_str);
      for (auto &d : defines) {
        global_defines.push_back("/D" + d);
      }
    }

    if (auto lang = cl_compile_settings.child("LanguageStandard")) {
      std::string lang_val = lang.child_value();
      if (lang_val == "stdcpp17")
        language_standard = "/std:c++17";
      else if (lang_val == "stdcpp14")
        language_standard = "/std:c++14";
      else if (lang_val == "stdcpp20")
        language_standard = "/std:c++20";
      else if (lang_val == "stdcpplatest")
        language_standard = "/std:c++latest";
    }
  }

  for (pugi::xml_node item_def :
       doc.child("Project").children("ItemDefinitionGroup")) {
    pugi::xml_node cl_node = item_def.child("ClCompile");
    if (cl_node) {

      if (auto inc = cl_node.child("AdditionalIncludeDirectories")) {
        std::string inc_str = inc.child_value();
        auto dirs = split_semicolon(inc_str);
        for (auto &dir : dirs) {
          std::string abs_dir = make_absolute(dir, project_dir);
          global_include_dirs.push_back("/I\"" + abs_dir + "\"");
        }
      }

      if (auto def = cl_node.child("PreprocessorDefinitions")) {
        std::string def_str = def.child_value();
        auto defines = split_semicolon(def_str);
        for (auto &d : defines) {
          global_defines.push_back("/D" + d);
        }
      }

      if (auto lang = cl_node.child("LanguageStandard")) {
        std::string lang_val = lang.child_value();
        if (lang_val == "stdcpp17")
          language_standard = "/std:c++17";
        else if (lang_val == "stdcpp14")
          language_standard = "/std:c++14";
        else if (lang_val == "stdcpp20")
          language_standard = "/std:c++20";
        else if (lang_val == "stdcpplatest")
          language_standard = "/std:c++latest";
      }
      break;
    }
  }

  std::vector<std::string> base_args;
  base_args.push_back("cl.exe");
  for (const auto &inc : global_include_dirs)
    base_args.push_back(inc);
  for (const auto &def : global_defines)
    base_args.push_back(def);
  if (!language_standard.empty())
    base_args.push_back(language_standard);

  base_args.push_back("/c");
  base_args.push_back("/Zi");

  for (pugi::xml_node cl_item : doc.child("Project").children("ItemGroup")) {
    for (pugi::xml_node compile_node : cl_item.children("ClCompile")) {
      const char *include_attr = compile_node.attribute("Include").value();
      if (!include_attr || strlen(include_attr) == 0)
        continue;

      fs::path source_path(include_attr);
      std::string ext = source_path.extension().string();

      if (ext != ".cpp" && ext != ".c" && ext != ".cc")
        continue;

      std::string abs_source = make_absolute(include_attr, project_dir);

      std::vector<std::string> file_args = base_args;
      file_args.push_back("\"" + abs_source + "\"");
      result[abs_source] = file_args;
    }
  }

  return result;
}

void generateCompileCommands(const std::string &slnx_path,
                             const std::string &output_path) {
  fs::path slnx_file(slnx_path);
  if (!fs::exists(slnx_file)) {
    std::cerr << "slnx file not found: " << slnx_path << std::endl;
    return;
  }

  fs::path solution_dir = slnx_file.parent_path();

  pugi::xml_document doc;
  if (!doc.load_file(slnx_path.c_str())) {
    std::cerr << "Failed to parse slnx file: " << slnx_path << std::endl;
    return;
  }

  std::vector<std::map<std::string, std::string>> commands;

  pugi::xml_node solution = doc.child("Solution");
  if (!solution) {
    std::cerr << "Invalid slnx: missing <Solution> root" << std::endl;
    return;
  }

  for (pugi::xml_node project_node : solution.children("Project")) {
    const char *path_attr = project_node.attribute("Path").value();
    if (!path_attr)
      continue;

    fs::path proj_path = solution_dir / path_attr;
    if (!fs::exists(proj_path)) {
      std::cerr << "Project file not found: " << proj_path << std::endl;
      continue;
    }

    std::cout << "Processing project: " << proj_path << std::endl;
    auto source_map = parse_vcxproj(proj_path, solution_dir);

    std::string working_dir = proj_path.parent_path().string();

    for (const auto &[source_file, args] : source_map) {
      std::map<std::string, std::string> entry;
      entry["directory"] = working_dir;
      entry["file"] = source_file;

      std::string command;
      for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0)
          command += " ";
        command += args[i];
      }
      entry["command"] = command;
      commands.push_back(entry);
    }
  }

  std::ofstream out(output_path);
  if (!out) {
    std::cerr << "Failed to open output file: " << output_path << std::endl;
    return;
  }

  json j = commands;
  out << j;

  std::cout << "Generated " << commands.size() << " entries in " << output_path
            << std::endl;
}