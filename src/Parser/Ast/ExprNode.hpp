#pragma once
#include "../Expression.hpp"
#include "../Node.hpp"
#include <string>

class ExprNode : public Node, public Expression {
public:
  ExprNode(Expression *value) : value(value) {}
  ~ExprNode() = default;
  Expression *value;
};