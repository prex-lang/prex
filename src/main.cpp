#include "Compiler/Compiler.hpp"
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <file1.prx> [file2.prx ...]\n", argv[0]);
    return 1;
  }

  std::vector<std::string> file_contents;
  std::vector<Token> all_tokens;
  std::string all_sources;

  for (int i = 1; i < argc; ++i) {
    std::ifstream file(argv[i]);
    if (!file.is_open()) {
      printf("Error: Could not open file %s\n", argv[i]);
      return 1;
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file_contents.push_back(content);
    all_sources += content + "\n";
    file.close();
  }

  for (int i = 0; i < file_contents.size(); ++i) {
    Lexer lexer(file_contents[i], argv[i + 1]);
    std::vector<Token> tokens = lexer.tokenize();
    all_tokens.insert(all_tokens.end(), tokens.begin(), tokens.end() - 1);
  }
  all_tokens.push_back(Token(EOF_TOKEN, "", 0));

  Parser parser(all_tokens, all_sources);
  RootNode *ast = parser.parse();

  Compiler compiler;
  compiler.root = ast;
  compiler.compile();
  compiler.writeLlvmToFile("output.ll");
  int ret = system("clang output.ll -o output.elf");
  if (ret != 0) {
    printf("Error: Failed to compile LLVM IR to ELF!\n");
  } else {
    printf("Generated executable: output.elf\n");
  }
  return 0;
}