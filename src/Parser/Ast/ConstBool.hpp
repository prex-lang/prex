#pragma once
#include "../Expression.hpp"

class ConstBool : public Expression {
public:
  ConstBool(bool value) : value(value) {}
  bool getValue() const { return value; }
  ~ConstBool() = default;
  bool value;
};