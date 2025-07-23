#pragma once

#include "../Node.hpp"
#include "ExprNode.hpp"
#include <string>

class VarNode : public Node {
public:
  std::string name;
  std::string type;
  ExprNode *value;
  VarNode(std::string name, std::string type, ExprNode *value)
      : name(name), type(type), value(value) {}
  ~VarNode() = default;
};