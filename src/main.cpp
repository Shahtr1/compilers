#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>

enum class TokenType
{
  _return,
  int_literal,
  semicolon,
};

struct Token
{
  TokenType type;
  std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& str){
  std::vector<Token> tokens;
  std::string buf;

  for (int i = 0; i < str.length(); i++) {
    char c = str[i];
    if (std::isalpha(c)) {
      buf.push_back(c);
      i++;
      while (std::isalnum(str[i])) {
        buf.push_back(str[i]);
        i++;
      }
      i--;

      if (buf == "return") {
        tokens.push_back({.type = TokenType::_return});
        buf.clear();
        continue;
      }
      else {
        std::cerr << "You messed up!" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
    else if (std::isdigit(c)) {
      buf.push_back(c);
      i++;
      while (std::isdigit(str[i])) {
        buf.push_back(str[i]);
        i++;
      }
      i--;
      tokens.push_back({.type = TokenType::int_literal, .value = buf});
      buf.clear();
    }
    else if (c == ';') {
      tokens.push_back({.type = TokenType::semicolon});
    }
    else if (std::isspace(c)) {
      continue;
    }
    else {
      std::cerr << "You messed up!" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens){
  std::stringstream output;
  output << "global _start\n_start:\n";
  for (int i = 0; i < tokens.size(); i++) {
    const Token& token = tokens[i];
    if (token.type == TokenType::_return) {
      if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::int_literal) {
        if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::semicolon) {
          output << "    mov rax, 60\n";
          output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
          output << "    syscall";
        }
      }
    }
  }

  return output.str();
}

int main(int argc, char* argv[]){
  if (argc != 2) {
    std::cerr << "Incorrect Usage: " << std::endl;
    std::cerr << "Usage: hydro <input.hy>" << std::endl;
    return EXIT_FAILURE;
  }


  std::string contents;

  {
    std::stringstream contents_stream;
    std::fstream input(argv[1], std::ios::in);
    contents_stream << input.rdbuf();
    contents = contents_stream.str();
  }

  std::vector<Token> tokens = tokenize(contents);

  {
    std::fstream file("out.asm", std::ios::out);
    file << tokens_to_asm(tokens);
  }

  system("nasm -felf64 out.asm");
  system("ld -o out out.o");

  return EXIT_SUCCESS;
}
