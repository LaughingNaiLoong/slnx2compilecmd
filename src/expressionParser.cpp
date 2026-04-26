#include "logging.h"
#include "utils.h"
#include <cctype>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>

enum class TokenType { String, Equal, End };

struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
private:
  std::string input;
  size_t pos = 0;

public:
  Lexer(std::string input) : input(input) {}
  Token next() {
    skipWhiteSpace();
    if (pos >= input.size()) {
      return {TokenType::End, ""};
    }
    char current = input[pos];
    if (current == '\'') {
      return scanString();
    }
    if (current == '=') {
      return scanEqual();
    }
    throw std::runtime_error("Illegal token.");
  };

private:
  Token scanString() {
    pos++;
    std::string str = "";
    while (pos < input.size() && input[pos] != '\'') {
      str += input[pos];
      pos++;
    }
    if (pos >= input.size()) {
      throw std::runtime_error("Unclosed string.");
    }
    pos++;
    return {TokenType::String, str};
  }

  Token scanEqual() {
    pos++;
    if (pos < input.size() && input[pos] == '=') {
      pos++;
      return {TokenType::Equal, ""};
    } else
      throw std::runtime_error("Unexpected token '='.");
  }

  void skipWhiteSpace() {
    while (pos < input.size() && std::isspace(input[pos]))
      pos++;
  }
};

bool evaluateMSBuildExpression(std::string expression,
                               std::map<std::string, std::string> vars) {
  std::string exp = s2cc::utils::replaceWithFunction(
      expression, std::regex{R"(\$\(([^)]+)\))"},
      [&](std::smatch m) -> std::string {
        if (vars.find(m[1].str()) != vars.end()) {
          std::string val = vars[m[1].str()];
          return val;
        }
        return m.str();
      });
  Lexer lexer(exp);
  Token leftVal = lexer.next();
  Token op = lexer.next();
  Token rightVal = lexer.next();
  LOG_DEBUG("Evaluating condition:L={} R={} RESULT={}", leftVal.value,
            rightVal.value, leftVal.value == rightVal.value);

  if (lexer.next().type != TokenType::End) {
    throw std::runtime_error("Unexpected token.");
  }

  return leftVal.value == rightVal.value;
}