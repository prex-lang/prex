#pragma once

#include "../Expression.hpp"
#include "ExprNode.hpp"

class UnaryOpNode : public Expression {
public:
  ExprNode *expr;
  std::string op;
  UnaryOpNode(ExprNode *expr, std::string op) : expr(expr), op(op) {}
  ~UnaryOpNode() = default;
};