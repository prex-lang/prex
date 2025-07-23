#pragma once

#include "../Node.hpp"
#include <vector>

class BodyNode : public Node {
public:
  std::vector<Node *> nodes;
  BodyNode(std::vector<Node *> nodes) : nodes(nodes) {}
  ~BodyNode() = default;
};