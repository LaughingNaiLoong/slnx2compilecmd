#include "generator.h"
#include "globalVariables.h"
#include "logging.h"
#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>
#include <list>
#include <pugixml.hpp>
#include <string>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("Slnx2CompileCommand", "1.0");
  program.add_argument("file").help("Path to slnx file");
  program.add_argument("--outdir")
      .default_value(std::string{"."})
      .help("Specify the output directory");
  program.add_argument("-D").append().nargs(1).help("Set a variable's value");

  try {
    program.parse_args(argc, argv);
    for (auto def : program.get<std::vector<std::string>>("-D")) {
      size_t assignPos = def.find_first_of("=");
      if (assignPos == std::string::npos || assignPos + 1 == def.length()) {
        std::cerr << "Syntax error: variable \"" << def
                  << "\"'s value is not assigned";
        return 1;
      }
      auto name = def.substr(0, assignPos);
      auto value = def.substr(assignPos + 1);
      setGlobalVariable(name, value);
    }
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  return generateCompileCommands(
      program.get("file"),
      (std::filesystem::path(program.get("outdir")) / "compile_commands.json")
          .string());
}