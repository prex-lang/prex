#pragma once

#include "../Token/Token.hpp"
#include "../Token/TokenType.hpp"
#include <string>
#include <vector>

class Lexer {
public:
  Lexer(const std::string &input, const std::string &filename);
  std::vector<Token> tokenize();

private:
  uint pos;
  std::string input;
  std::string filename;
  std::vector<Token> tokens;

  void advance();
  char peek();
  char peekNext();
  bool isAtEnd();
  bool isDigit(char c);
  bool isAlpha(char c);
  Token nextToken();
  std::string readIdentifier();
  std::string readNumber();
  std::string readString();
  TokenType checkKeyword(const std::string &word);
};