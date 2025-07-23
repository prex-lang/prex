#include "Token.hpp"
#include <iostream>

Token::Token(TokenType type, const std::string &value, uint pos)
    : type(tokenTypeToString(type)), value(value), pos(pos) {}

std::string Token::tokenTypeToString(TokenType type) {
  switch (type) {
  case KEYWORD_DEFUN:
    return "KEYWORD_DEFUN";
  case KEYWORD_RET:
    return "KEYWORD_RET";
  case KEYWORD_IF:
    return "KEYWORD_IF";
  case KEYWORD_ELSE:
    return "KEYWORD_ELSE";
  case KEYWORD_LOOP:
    return "KEYWORD_LOOP";
  case KEYWORD_FOR:
    return "KEYWORD_FOR";
  case KEYWORD_STRUCT:
    return "KEYWORD_STRUCT";
  case KEYWORD_ENUM:
    return "KEYWORD_ENUM";
  case KEYWORD_USE:
    return "KEYWORD_USE";
  case KEYWORD_IMPORT:
    return "KEYWORD_IMPORT";
  case KEYWORD_AS:
    return "KEYWORD_AS";
  case KEYWORD_FROM:
    return "KEYWORD_FROM";
  case KEYWORD_IMPL:
    return "KEYWORD_IMPL";
  case SYMBOL_PLUS:
    return "SYMBOL_PLUS";
  case SYMBOL_MINUS:
    return "SYMBOL_MINUS";
  case SYMBOL_MULTIPLY:
    return "SYMBOL_MULTIPLY";
  case SYMBOL_DIVIDE:
    return "SYMBOL_DIVIDE";
  case SYMBOL_MODULO:
    return "SYMBOL_MODULO";
  case SYMBOL_ASSIGN:
    return "SYMBOL_ASSIGN";
  case SYMBOL_LPAREN:
    return "SYMBOL_LPAREN";
  case SYMBOL_RPAREN:
    return "SYMBOL_RPAREN";
  case SYMBOL_LBRACE:
    return "SYMBOL_LBRACE";
  case SYMBOL_RBRACE:
    return "SYMBOL_RBRACE";
  case SYMBOL_LBRACKET:
    return "SYMBOL_LBRACKET";
  case SYMBOL_RBRACKET:
    return "SYMBOL_RBRACKET";
  case SYMBOL_SEMICOLON:
    return "SYMBOL_SEMICOLON";
  case SYMBOL_COMMA:
    return "SYMBOL_COMMA";
  case SYMBOL_COLON:
    return "SYMBOL_COLON";
  case SYMBOL_DOT:
    return "SYMBOL_DOT";
  case SYMBOL_GREATER:
    return "SYMBOL_GREATER";
  case SYMBOL_LESS:
    return "SYMBOL_LESS";
  case SYMBOL_PLUS_ASSIGN:
    return "SYMBOL_PLUS_ASSIGN";
  case SYMBOL_MINUS_ASSIGN:
    return "SYMBOL_MINUS_ASSIGN";
  case SYMBOL_MULTIPLY_ASSIGN:
    return "SYMBOL_MULTIPLY_ASSIGN";
  case SYMBOL_DIVIDE_ASSIGN:
    return "SYMBOL_DIVIDE_ASSIGN";
  case SYMBOL_XOR:
    return "SYMBOL_XOR";
  case SYMBOL_XOR_ASSIGN:
    return "SYMBOL_XOR_ASSIGN";
  case SYMBOL_GREATER_EQUAL:
    return "SYMBOL_GREATER_EQUAL";
  case SYMBOL_LESS_EQUAL:
    return "SYMBOL_LESS_EQUAL";
  case SYMBOL_LOGICAL_AND:
    return "SYMBOL_LOGICAL_AND";
  case SYMBOL_LOGICAL_OR:
    return "SYMBOL_LOGICAL_OR";
  case SYMBOL_BIT_AND:
    return "SYMBOL_BIT_AND";
  case SYMBOL_BIT_AND_ASSIGN:
    return "SYMBOL_BIT_AND_ASSIGN";
  case SYMBOL_BIT_OR:
    return "SYMBOL_BIT_OR";
  case SYMBOL_BIT_OR_ASSIGN:
    return "SYMBOL_BIT_OR_ASSIGN";
  case SYMBOL_BIT_SHIFT_LEFT:
    return "SYMBOL_BIT_SHIFT_LEFT";
  case SYMBOL_BIT_SHIFT_LEFT_ASSIGN:
    return "SYMBOL_BIT_SHIFT_LEFT_ASSIGN";
  case SYMBOL_BIT_SHIFT_RIGHT:
    return "SYMBOL_BIT_SHIFT_RIGHT";
  case SYMBOL_BIT_SHIFT_RIGHT_ASSIGN:
    return "SYMBOL_BIT_SHIFT_RIGHT_ASSIGN";
  case CONSTANT_TRUE:
    return "CONSTANT_TRUE";
  case CONSTANT_FALSE:
    return "CONSTANT_FALSE";
  case CONSTANT_NULL:
    return "CONSTANT_NULL";
  case CONSTANT_STRING:
    return "CONSTANT_STRING";
  case CONSTANT_NUMBER:
    return "CONSTANT_NUMBER";
  case CONSTANT_FLOAT:
    return "CONSTANT_FLOAT";
  case CONSTANT_DOUBLE:
    return "CONSTANT_DOUBLE";
  case CONSTANT_CHAR:
    return "CONSTANT_CHAR";
  case IDENTIFIER:
    return "IDENTIFIER";
  case EOF_TOKEN:
    return "EOF_TOKEN";
  case SYMBOL_NOT_EQUAL:
    return "SYMBOL_NOT_EQUAL";
  case SYMBOL_LOGICAL_NOT:
    return "SYMBOL_LOGICAL_NOT";
  case SYMBOL_EQUAL:
    return "SYMBOL_EQUAL";
  default:
    return "UNKNOWN";
  }
}

void Token::prettyPrint() const {
  std::cout << "Token(type='" << type << "', value='" << value
            << "', pos=" << pos << ")" << std::endl;
}
