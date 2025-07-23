#pragma once

#include "../Expression.hpp"

class ConstInt : public Expression {
public:
  ConstInt(long long value) : value(value) {}
  long long getValue() const { return value; }
  ~ConstInt() = default;
  long long value;
};