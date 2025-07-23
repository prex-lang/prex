#pragma once
#include "../Node.hpp"
#include <string>

class Arg {
public:
  std::string name;
  std::string type;
  Arg(std::string name, std::string type) {
    this->name = name;
    this->type = type;
  }
  ~Arg() = default;
};