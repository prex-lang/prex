#pragma once
#include "../Expression.hpp"
#include "ExprNode.hpp"
#include <string>

class BinOpNode : public Expression {
public:
  ExprNode *left;
  ExprNode *right;
  std::string op;
  BinOpNode(ExprNode *left, ExprNode *right, std::string op)
      : left(left), right(right), op(op) {}
  ~BinOpNode() = default;
};