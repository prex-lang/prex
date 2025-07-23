#pragma once

#include "../Expression.hpp"
#include "ExprNode.hpp"
#include <vector>

class FunctionCallNode : public Expression {
public:
  std::string name;
  std::vector<ExprNode *> args;
  FunctionCallNode(std::string name, std::vector<ExprNode *> args)
      : name(name), args(args) {}
  ~FunctionCallNode() = default;
};