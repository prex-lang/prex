#pragma once

#include "../Node.hpp"

class ConstChar : public Node {
public:
  ConstChar(char value) : value(value) {}
  char getValue() const { return value; }
  ~ConstChar() = default;

  char value;
};