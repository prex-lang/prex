#pragma once

#include "../Node.hpp"
#include <vector>

class RootNode : public Node {
public:
  std::vector<Node *> nodes;
  RootNode(std::vector<Node *> nodes) : nodes(nodes) {}
  ~RootNode() = default;
};