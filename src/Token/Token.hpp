#pragma once

#include "TokenType.hpp"
#include <string>

class Token {
public:
  Token(const std::string &type, const std::string &value);
  Token(TokenType type, const std::string &value, uint pos);
  std::string type;
  std::string value;
  uint pos;
  void prettyPrint() const;
  static std::string tokenTypeToString(TokenType type);
};