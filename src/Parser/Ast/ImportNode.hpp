#pragma once
#include "../Node.hpp"
#include <string>

class ImportNode : public Node {
public:
  std::string modulePath;
  ImportNode(const std::string &modulePath) : modulePath(modulePath) {}
};