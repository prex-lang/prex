#pragma once

#include "../Node.hpp"
#include "ExprNode.hpp"

class RetNode : public Node {
public:
  ExprNode *expr;
  RetNode(ExprNode *expr) : expr(expr) {}
  ~RetNode() = default;
};