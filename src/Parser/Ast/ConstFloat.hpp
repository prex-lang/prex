#pragma once

#include "../Expression.hpp"

class ConstFloat : public Expression {
public:
  ConstFloat(double value) : value(value) {}
  double getValue() const { return value; }
  ~ConstFloat() = default;

  double value;
};