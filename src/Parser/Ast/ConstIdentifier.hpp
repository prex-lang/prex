#pragma once

#include "../Expression.hpp"
#include <string>

class ConstIdentifier : public Expression {
public:
  std::string name;
  ConstIdentifier(std::string name) : name(name) {}
  ~ConstIdentifier() = default;
};