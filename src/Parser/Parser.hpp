#pragma once
#include "../Token/Token.hpp"
#include "../Token/TokenType.hpp"
#include "Ast/Arg.hpp"
#include "Ast/BodyNode.hpp"
#include "Ast/DefunNode.hpp"
#include "Ast/ExprNode.hpp"
#include "Ast/FunctionCallNode.hpp"
#include "Ast/IfNode.hpp"
#include "Ast/ImportNode.hpp"
#include "Ast/LoopNode.hpp"
#include "Ast/RetNode.hpp"
#include "Ast/RootNode.hpp"
#include "Ast/VarAssignNode.hpp"
#include "Ast/VarNode.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class Parser {
public:
  Parser(const std::vector<Token> &tokens);
  Parser(const std::vector<Token> &tokens, const std::string &source_code,
         const std::string &filename);
  Parser(const std::vector<Token> &tokens, const std::string &source_code);

  RootNode *parse();
  void printAst(Node *node, const std::string &indent = "", bool isLast = true);
  void printExpression(Expression *expr, const std::string &indent = "",
                       bool isLast = true);

private:
  std::vector<Token> tokens;
  int position = 0;
  static const std::unordered_map<std::string, int> precedence;
  std::string source_code;
  std::string filename;

  Node *parseStatement();
  DefunNode *parseDefun();
  Node *parseBodyStmt();
  FunctionCallNode *parseFunctionCall();
  VarNode *parseVarDecl();
  VarAssignNode *parseVarAssign();
  int getPrecedence(const Token &token);
  Expression *parseExpression(int parentPrecedence = 0);
  Expression *parsePrimary();
  Expression *parseIdOrFunCall();
  Expression *parseGroupedExpression();
  BodyNode *parseBody();
  std::vector<Arg> parseArgsDecl();
  Node *parseImport();

  Token consume(const std::string &type, const std::string &errorMessage = "");
  Token peek();
  Token peek2();
  Token nextToken();
  bool isAtEnd();
  Token peek3();
  IfNode *parseIf();
  LoopNode *parseLoop();
};