#include "generator.h"
#include <argparse/argparse.hpp>
#include <filesystem>
#include <pugixml.hpp>

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("Slnx2CompileCommand", "1.0");
  program.add_argument("file").help("Path to slnx file");
  program.add_argument("--outpath")
      .default_value(std::string{"."})
      .help("Specify the output directory");
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }
  generateCompileCommands(
      program.get("file"),
      (std::filesystem::path(program.get("outpath")) / "compile_commands.json")
          .string());
}