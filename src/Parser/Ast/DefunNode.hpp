#pragma once

#include "../Node.hpp"
#include "Arg.hpp"
#include "BodyNode.hpp"
#include <string>
#include <vector>

class DefunNode : public Node {
public:
  std::string name;
  std::vector<Arg> args;
  std::string ret_type;
  BodyNode *body;
  DefunNode(std::string name, std::vector<Arg> args, std::string ret_type,
            BodyNode *body) {
    this->name = name;
    this->args = args;
    this->ret_type = ret_type;
    this->body = body;
  }
  ~DefunNode() = default;
};