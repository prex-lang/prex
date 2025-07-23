#include "Parser.hpp"
#include "../Token/Token.hpp"
#include "../Token/TokenType.hpp"
#include "Ast/Arg.hpp"
#include "Ast/BinOpNode.hpp"
#include "Ast/BodyNode.hpp"
#include "Ast/ConstBool.hpp"
#include "Ast/ConstFloat.hpp"
#include "Ast/ConstIdentifier.hpp"
#include "Ast/ConstInt.hpp"
#include "Ast/ConstString.hpp"
#include "Ast/DefunNode.hpp"
#include "Ast/ExprNode.hpp"
#include "Ast/FunctionCallNode.hpp"
#include "Ast/IfNode.hpp"
#include "Ast/ImportNode.hpp"
#include "Ast/RetNode.hpp"
#include "Ast/RootNode.hpp"
#include "Ast/UnaryOpNode.hpp"
#include "Ast/VarAssignNode.hpp"
#include "Ast/VarNode.hpp"
#include <iostream>
#include <sstream>
#include <string>

const std::unordered_map<std::string, int> Parser::precedence = {
    {"SYMBOL_LOGICAL_OR", 1}, {"SYMBOL_LOGICAL_AND", 2},
    {"SYMBOL_ASSIGN", 3},     {"SYMBOL_EQUAL", 4},
    {"SYMBOL_NOT_EQUAL", 4},  {"SYMBOL_GREATER", 5},
    {"SYMBOL_LESS", 5},       {"SYMBOL_GREATER_EQUAL", 5},
    {"SYMBOL_LESS_EQUAL", 5},

    {"SYMBOL_PLUS", 6},       {"SYMBOL_MINUS", 6},
    {"SYMBOL_MULTIPLY", 7},   {"SYMBOL_DIVIDE", 7},
    {"SYMBOL_MODULO", 7},     {"SYMBOL_XOR", 8},
};

static std::pair<int, int> getLineCol(const std::string &source, uint pos) {
  int line = 1, col = 1;
  for (uint i = 0; i < pos && i < source.size(); ++i) {
    if (source[i] == '\n') {
      ++line;
      col = 1;
    } else {
      ++col;
    }
  }
  return {line, col};
}

Parser::Parser(const std::vector<Token> &tokens, const std::string &source_code,
               const std::string &filename)
    : tokens(tokens), source_code(source_code), filename(filename) {}

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), source_code(""), filename("") {}

Parser::Parser(const std::vector<Token> &tokens, const std::string &source_code)
    : tokens(tokens), source_code(source_code), filename("") {}

RootNode *Parser::parse() {
  std::vector<Node *> nodes;
  while (peek().type != "EOF_TOKEN") {
    nodes.push_back(parseStatement());
  }
  return new RootNode(nodes);
}

Node *Parser::parseStatement() {
  Token current = peek();
  if (current.type == "KEYWORD_DEFUN") {
    return parseDefun();
  }
  if (current.type == "KEYWORD_IMPORT") {
    return parseImport();
  }
  std::cerr << "Unknown statement type" << std::endl;
  std::exit(1);
}

DefunNode *Parser::parseDefun() {
  std::string ret_type = "void";
  consume("KEYWORD_DEFUN");
  std::string name = consume("IDENTIFIER", "Expected function name.").value;
  consume("SYMBOL_LPAREN", "Expected '(' after function name.");
  std::vector<Arg> args = parseArgsDecl();
  consume("SYMBOL_RPAREN", "Expected ')' after arguments.");
  consume("SYMBOL_GREATER", "Expected '>' after arguments.");
  ret_type = consume("IDENTIFIER", "Expected return type.").value;
  consume("SYMBOL_LBRACE");
  BodyNode *body = parseBody();
  consume("SYMBOL_RBRACE");
  return new DefunNode(name, args, ret_type, body);
}

void Parser::printAst(Node *node, const std::string &indent, bool isLast) {
  std::string branch = isLast ? "└── " : "├── ";
  std::string newIndent = indent + (isLast ? "    " : "│   ");

  if (auto root = dynamic_cast<RootNode *>(node)) {
    std::cout << indent << "Root" << std::endl;
    for (size_t i = 0; i < root->nodes.size(); ++i) {
      printAst(root->nodes[i], newIndent, i == root->nodes.size() - 1);
    }
  } else if (auto defun = dynamic_cast<DefunNode *>(node)) {
    std::cout << indent << branch << "Defun: " << defun->name << " -> "
              << defun->ret_type << std::endl;
    std::cout << newIndent << "├── Args:" << std::endl;
    for (size_t i = 0; i < defun->args.size(); ++i) {
      std::string argBranch = (i == defun->args.size() - 1) ? "└── " : "├── ";
      std::cout << newIndent << "│   " << argBranch << defun->args[i].type
                << " " << defun->args[i].name << std::endl;
    }
    std::cout << newIndent << "└── Body:" << std::endl;
    printAst(defun->body, newIndent + "    ", true);
  } else if (auto body = dynamic_cast<BodyNode *>(node)) {
    for (size_t i = 0; i < body->nodes.size(); ++i) {
      printAst(body->nodes[i], newIndent, i == body->nodes.size() - 1);
    }
  } else if (auto var = dynamic_cast<VarNode *>(node)) {
    std::cout << indent << branch << "VarDecl: type: " << var->type
              << ", name: " << var->name << std::endl;
    if (var->value) {
      std::cout << newIndent << "└── InitExpr:" << std::endl;
      printExpression(var->value, newIndent + "    ", true);
    }
  } else if (auto assign = dynamic_cast<VarAssignNode *>(node)) {
    std::cout << indent << branch << "VarAssign: name: " << assign->name
              << std::endl;
    if (assign->value) {
      std::cout << newIndent << "└── Value:" << std::endl;
      printExpression(assign->value, newIndent + "    ", true);
    }
  } else if (auto exprNode = dynamic_cast<ExprNode *>(node)) {
    std::cout << indent << branch << "ExprStmt:" << std::endl;
    printExpression(exprNode, newIndent, true);
  } else if (auto ret = dynamic_cast<RetNode *>(node)) {
    std::cout << indent << branch << "Return" << std::endl;
    printExpression(ret->expr, newIndent, true);
  } else if (auto ifNode = dynamic_cast<IfNode *>(node)) {
    std::cout << indent << branch << "If" << std::endl;
    std::cout << newIndent << "├── Condition:" << std::endl;
    printExpression(ifNode->condition, newIndent + "│   ", true);
    std::cout << newIndent << "├── Body:" << std::endl;
    printAst(ifNode->body, newIndent + "│   ", true);
    if (ifNode->elseIf) {
      std::cout << newIndent << "├── ElseIf:" << std::endl;
      printAst(ifNode->elseIf, newIndent + "│   ", true);
    }
    if (ifNode->elseBody) {
      std::cout << newIndent << "└── Else:" << std::endl;
      printAst(ifNode->elseBody, newIndent + "    ", true);
    }
  } else {
    std::cout << indent << branch << "UnknownNode" << std::endl;
  }
}

void Parser::printExpression(Expression *expr, const std::string &indent,
                             bool isLast) {
  std::string branch = isLast ? "└── " : "├── ";
  std::string newIndent = indent + (isLast ? "    " : "│   ");

  if (!expr) {
    std::cout << indent << branch << "<null expr>" << std::endl;
    return;
  }

  std::cout << indent << branch << "[type: " << typeid(*expr).name() << "]"
            << std::endl;
  if (auto intNode = dynamic_cast<ConstInt *>(expr)) {
    std::cout << indent << branch << "ConstInt: " << intNode->getValue()
              << std::endl;
  } else if (auto floatNode = dynamic_cast<ConstFloat *>(expr)) {
    std::cout << indent << branch << "ConstFloat: " << floatNode->getValue()
              << std::endl;
  } else if (auto strNode = dynamic_cast<ConstString *>(expr)) {
    std::cout << indent << branch << "ConstString: \"" << strNode->getValue()
              << "\"" << std::endl;
  } else if (auto idNode = dynamic_cast<ConstIdentifier *>(expr)) {
    std::cout << indent << branch << "Identifier: " << idNode->name
              << std::endl;
  } else if (auto binOp = dynamic_cast<BinOpNode *>(expr)) {
    std::cout << indent << branch << "BinOp: " << binOp->op << std::endl;
    printExpression(binOp->left, newIndent, false);
    printExpression(binOp->right, newIndent, true);
  } else if (auto unaryOp = dynamic_cast<UnaryOpNode *>(expr)) {
    std::cout << indent << branch << "UnaryOp: " << unaryOp->op << std::endl;
    printExpression(unaryOp->expr, newIndent, true);
  } else if (auto funCall = dynamic_cast<FunctionCallNode *>(expr)) {
    std::cout << indent << branch << "FunctionCall: " << funCall->name
              << std::endl;
    for (size_t i = 0; i < funCall->args.size(); ++i) {
      printExpression(funCall->args[i], newIndent,
                      i == funCall->args.size() - 1);
    }
  } else if (auto exprNode = dynamic_cast<ExprNode *>(expr)) {
    std::cout << indent << branch << "ExprNode:" << std::endl;
    printExpression(exprNode->value, newIndent, true);
  } else {
    std::cout << indent << branch << "UnknownExpr" << std::endl;
  }
}

Node *Parser::parseBodyStmt() {
  Token current = peek();
  if (current.type == "IDENTIFIER") {
    Token next = peek2();
    if (next.type == "SYMBOL_LPAREN") {
      return new ExprNode(parseFunctionCall());
    } else if (next.type == "IDENTIFIER") {
      return parseVarDecl();
    } else if (next.type == "SYMBOL_ASSIGN") {
      return parseVarAssign();
    } else {
      return new ExprNode(parseFunctionCall());
    }
  } else if (current.type == "KEYWORD_RET") {
    consume("KEYWORD_RET");
    Expression *v = parseExpression();
    return new RetNode(static_cast<ExprNode *>(v));
  } else if (current.type == "KEYWORD_IF") {
    return parseIf();
  } else if (current.type == "KEYWORD_LOOP") {
    return parseLoop();
  }
  nextToken();
  return nullptr;
}

FunctionCallNode *Parser::parseFunctionCall() {
  std::string name = consume("IDENTIFIER").value;
  std::vector<ExprNode *> params;
  consume("SYMBOL_LPAREN");
  while (peek().type != "SYMBOL_RPAREN") {
    params.push_back(static_cast<ExprNode *>(parseExpression()));
    if (peek().type == "SYMBOL_COMMA") {
      nextToken();
    } else {
      break;
    }
  }
  consume("SYMBOL_RPAREN");
  consume("SYMBOL_SEMICOLON");
  return new FunctionCallNode(name, params);
}

VarNode *Parser::parseVarDecl() {
  std::string type = consume("IDENTIFIER").value;
  std::string name = consume("IDENTIFIER").value;
  if (peek().type == "SYMBOL_SEMICOLON") {
    consume("SYMBOL_SEMICOLON");
    return new VarNode(name, type, nullptr);
  } else {
    consume("SYMBOL_ASSIGN", "Expected '=' or ';' after variable declaration");
    ExprNode *expr = static_cast<ExprNode *>(parseExpression());
    consume("SYMBOL_SEMICOLON", "Missing semicolon after variable declaration");
    return new VarNode(name, type, expr);
  }
}

VarAssignNode *Parser::parseVarAssign() {
  std::string name = consume("IDENTIFIER").value;
  consume("SYMBOL_ASSIGN", "Expected '=' after variable name");
  ExprNode *expr = static_cast<ExprNode *>(parseExpression());
  consume("SYMBOL_SEMICOLON", "Missing semicolon after assignment");
  return new VarAssignNode(name, expr);
}

int Parser::getPrecedence(const Token &token) {
  auto it = precedence.find(token.type);
  return it != precedence.end() ? it->second : -1;
}

Expression *Parser::parseExpression(int parentPrecedence) {
  Expression *left = parsePrimary();
  while (true) {
    Token opToken = peek();
    int prec = getPrecedence(opToken);
    if (prec == -1 || prec < parentPrecedence)
      break;
    nextToken();
    Expression *right = parseExpression(prec + 1);
    left = new ExprNode(new BinOpNode(static_cast<ExprNode *>(left),
                                      static_cast<ExprNode *>(right),
                                      opToken.value));
  }
  return left;
}

Expression *Parser::parsePrimary() {
  Token token = peek();
  if (token.type == "SYMBOL_MINUS" || token.type == "SYMBOL_BIT_AND" ||
      token.type == "SYMBOL_MULTIPLY") {
    Token op = nextToken();
    Expression *right = parsePrimary();
    return new ExprNode(
        new UnaryOpNode(static_cast<ExprNode *>(right), op.value));
  }
  if (token.type == "CONSTANT_NUMBER") {
    return new ExprNode(
        new ConstInt(std::stoll(consume("CONSTANT_NUMBER").value)));
  } else if (token.type == "CONSTANT_FLOAT" ||
             token.type == "CONSTANT_DOUBLE") {

    return new ExprNode(new ConstFloat(std::stod(consume(token.type).value)));
  } else if (token.type == "CONSTANT_STRING") {
    return new ExprNode(new ConstString(consume("CONSTANT_STRING").value));
  } else if (token.type == "CONSTANT_TRUE") {
    consume("CONSTANT_TRUE");
    return new ExprNode(new ConstBool(true));
  } else if (token.type == "CONSTANT_FALSE") {
    consume("CONSTANT_FALSE");
    return new ExprNode(new ConstBool(false));
  } else if (token.type == "IDENTIFIER") {
    return parseIdOrFunCall();
  } else if (token.type == "SYMBOL_LPAREN") {
    return parseGroupedExpression();
  } else {
    std::cerr << "Unexpected token in expression" << std::endl;
    std::exit(1);
  }
}

Expression *Parser::parseIdOrFunCall() {
  std::string name = consume("IDENTIFIER").value;
  if (peek().type == "SYMBOL_LPAREN") {
    nextToken();
    std::vector<ExprNode *> params;
    while (peek().type != "SYMBOL_RPAREN") {
      params.push_back(static_cast<ExprNode *>(parseExpression()));
      if (peek().type == "SYMBOL_COMMA")
        nextToken();
      else
        break;
    }
    consume("SYMBOL_RPAREN", "Expected ')' after function call arguments");
    return new ExprNode(new FunctionCallNode(name, params));
  }

  return new ExprNode(new ConstIdentifier(name));
}

Expression *Parser::parseGroupedExpression() {
  consume("SYMBOL_LPAREN", "Expected '(' at start of expression");
  Expression *expr = parseExpression();
  consume("SYMBOL_RPAREN", "Expected ')' after expression");
  return expr;
}

BodyNode *Parser::parseBody() {
  std::vector<Node *> nodes;
  while (peek().type != "SYMBOL_RBRACE") {
    nodes.push_back(parseBodyStmt());
  }
  return new BodyNode(nodes);
}

Token Parser::peek3() {
  if (position + 2 >= tokens.size())
    return tokens.back();
  return tokens[position + 2];
}

std::vector<Arg> Parser::parseArgsDecl() {
  std::vector<Arg> args;
  while (peek().type != "SYMBOL_RPAREN") {

    std::string type =
        consume("IDENTIFIER", "Expected type in argument declaration").value;
    consume("SYMBOL_COLON", "Expected ':' after type");

    while (true) {
      std::string name =
          consume("IDENTIFIER", "Expected argument name after ':'").value;
      args.emplace_back(name, type);
      if (peek().type == "SYMBOL_COMMA") {
        Token next = peek2();

        if (next.type == "IDENTIFIER") {
          if (peek3().type == "SYMBOL_COLON") {
            nextToken();
            break;
          } else {
            nextToken();
            continue;
          }
        } else {
          nextToken();
          break;
        }
      } else {
        break;
      }
    }
  }
  return args;
}

Token Parser::consume(const std::string &type,
                      const std::string &errorMessage) {
  if (peek().type == type) {
    return nextToken();
  }
  std::string msg =
      errorMessage.empty() ? "Expected token of type " + type : errorMessage;
  if (msg.find("Expected") != std::string::npos && !source_code.empty()) {
    auto [line, col] = getLineCol(source_code, peek().pos);
    std::cerr << "[" << filename << "] " << msg << " at <" << line << ", "
              << col << ">" << std::endl;
  } else {
    std::cerr << "[" << filename << "] " << msg << std::endl;
  }
  std::exit(1);
}

Token Parser::peek() {
  if (isAtEnd())
    return tokens.back();
  return tokens[position];
}

Token Parser::peek2() {
  if (position + 1 >= tokens.size())
    return tokens.back();
  return tokens[position + 1];
}

Token Parser::nextToken() {
  if (!isAtEnd())
    position++;
  return tokens[position - 1];
}

bool Parser::isAtEnd() { return position >= tokens.size(); }

IfNode *Parser::parseIf() {
  consume("KEYWORD_IF");
  consume("SYMBOL_LPAREN");
  ExprNode *condition = static_cast<ExprNode *>(parseExpression());
  consume("SYMBOL_RPAREN");
  consume("SYMBOL_LBRACE", "Expected '{' after if condition");
  BodyNode *body = parseBody();
  consume("SYMBOL_RBRACE", "Expected '}' after if body");

  IfNode *elseIf = nullptr;
  BodyNode *elseBody = nullptr;
  if (peek().type == "KEYWORD_ELSE") {
    consume("KEYWORD_ELSE");
    if (peek().type == "KEYWORD_IF") {
      elseIf = parseIf();
    } else if (peek().type == "SYMBOL_LBRACE") {
      consume("SYMBOL_LBRACE");
      elseBody = parseBody();
      consume("SYMBOL_RBRACE", "Expected '}' after else body");
    }
  }
  return new IfNode(condition, body, elseIf, elseBody);
}

LoopNode *Parser::parseLoop() {
  consume("KEYWORD_LOOP");
  consume("SYMBOL_LPAREN");
  ExprNode *condition = static_cast<ExprNode *>(parseExpression());
  consume("SYMBOL_RPAREN");
  consume("SYMBOL_LBRACE", "Expected '{' after loop condition");
  BodyNode *body = parseBody();
  consume("SYMBOL_RBRACE", "Expected '}' after loop body");
  return new LoopNode(condition, body);
}

Node *Parser::parseImport() {
  consume("KEYWORD_IMPORT");
  std::string modulePath =
      consume("IDENTIFIER", "Expected module name after 'import'").value;
  while (peek().type == "SYMBOL_DOT") {
    consume("SYMBOL_DOT");
    modulePath += "." + consume("IDENTIFIER",
                                "Expected identifier after '.' in import path")
                            .value;
  }
  consume("SYMBOL_SEMICOLON", "Expected ';' after import statement");
  return new ImportNode(modulePath);
}