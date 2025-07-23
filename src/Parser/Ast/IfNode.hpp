#pragma once

#include "../Node.hpp"
#include "BodyNode.hpp"
#include "ExprNode.hpp"
#include <vector>

class IfNode : public Node {
public:
  ExprNode *condition;
  BodyNode *body;
  IfNode *elseIf;     // else if chain
  BodyNode *elseBody; // else block
  IfNode(ExprNode *condition, BodyNode *body, IfNode *elseIf = nullptr,
         BodyNode *elseBody = nullptr)
      : condition(condition), body(body), elseIf(elseIf), elseBody(elseBody) {}
};