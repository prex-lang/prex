#pragma once

#include "../Expression.hpp"
#include <string>

class ConstString : public Expression {
public:
  ConstString(const std::string &value) : value(value) {}
  std::string getValue() const { return value; }
  ~ConstString() = default;

  std::string value;
};