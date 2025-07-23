#pragma once

#include "../Node.hpp"
#include "ExprNode.hpp"
#include <string>

class VarAssignNode : public Node {
public:
  std::string name;
  ExprNode *value;
  VarAssignNode(std::string name, ExprNode *value) : name(name), value(value) {}
  ~VarAssignNode() = default;
};