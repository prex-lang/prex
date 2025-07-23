#pragma once

#include "../Node.hpp"
#include "BodyNode.hpp"
#include "ExprNode.hpp"
#include <vector>

class LoopNode : public Node {
public:
  ExprNode *condition;
  BodyNode *body;
  LoopNode(ExprNode *condition, BodyNode *body)
      : condition(condition), body(body) {}
};