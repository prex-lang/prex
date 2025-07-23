#include "Lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string &input, const std::string &filename)
    : input(input), filename(filename), pos(0) {}

std::vector<Token> Lexer::tokenize() {
  tokens.clear();
  while (!isAtEnd()) {
    Token token = nextToken();
    if (token.type != "EOF_TOKEN")
      tokens.push_back(token);
  }
  tokens.push_back(Token(EOF_TOKEN, "", pos));
  return tokens;
}

void Lexer::advance() { pos++; }

char Lexer::peek() { return isAtEnd() ? '\0' : input[pos]; }

char Lexer::peekNext() {
  return (pos + 1 < input.size()) ? input[pos + 1] : '\0';
}

bool Lexer::isAtEnd() { return pos >= input.size(); }

bool Lexer::isDigit(char c) {
  return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::isAlpha(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

std::string Lexer::readIdentifier() {
  size_t start = pos;
  while (!isAtEnd() && (isAlpha(peek()) || isDigit(peek())))
    advance();
  return input.substr(start, pos - start);
}

std::string Lexer::readNumber() {
  size_t start = pos;
  bool dotFound = false;
  while (!isAtEnd()) {
    if (isDigit(peek())) {
      advance();
    } else if (peek() == '.' && !dotFound) {
      dotFound = true;
      advance();
    } else {
      break;
    }
  }
  return input.substr(start, pos - start);
}

std::string Lexer::readString() {
  advance(); // skip opening "
  std::string str;
  while (!isAtEnd() && peek() != '"') {
    if (peek() == '\\') {
      advance();
      char esc = peek();
      switch (esc) {
      case 'n':
        str += '\n';
        break;
      case 't':
        str += '\t';
        break;
      case 'r':
        str += '\r';
        break;
      case '\\':
        str += '\\';
        break;
      case '"':
        str += '"';
        break;
      default:
        str += esc;
        break;
      }
      advance();
    } else {
      str += peek();
      advance();
    }
  }
  advance(); // skip closing "
  return str;
}

TokenType Lexer::checkKeyword(const std::string &word) {
  if (word == "defun")
    return KEYWORD_DEFUN;
  if (word == "ret")
    return KEYWORD_RET;
  if (word == "if")
    return KEYWORD_IF;
  if (word == "else")
    return KEYWORD_ELSE;
  if (word == "loop")
    return KEYWORD_LOOP;
  if (word == "for")
    return KEYWORD_FOR;
  if (word == "struct")
    return KEYWORD_STRUCT;
  if (word == "enum")
    return KEYWORD_ENUM;
  if (word == "use")
    return KEYWORD_USE;
  if (word == "import")
    return KEYWORD_IMPORT;
  if (word == "as")
    return KEYWORD_AS;
  if (word == "from")
    return KEYWORD_FROM;
  if (word == "impl")
    return KEYWORD_IMPL;
  if (word == "true")
    return CONSTANT_TRUE;
  if (word == "false")
    return CONSTANT_FALSE;
  return IDENTIFIER;
}

Token Lexer::nextToken() {
  while (!isAtEnd()) {
    char c = peek();
    if (std::isspace(static_cast<unsigned char>(c))) {
      advance();
      continue;
    }
    if (isAlpha(c)) {
      size_t start = pos;
      std::string word = readIdentifier();
      TokenType type = checkKeyword(word);
      return Token(type, word, start);
    }

    if (c == '"') {
      size_t start = pos;
      std::string str = readString();
      return Token(CONSTANT_STRING, str, start);
    }
    if (isDigit(c)) {
      size_t start = pos;
      std::string num = readNumber();
      if (num.find('.') != std::string::npos) {
        return Token(CONSTANT_DOUBLE, num, start);
      } else {
        return Token(CONSTANT_NUMBER, num, start);
      }
    }
    size_t start = pos;
    switch (c) {
    case '+':
      advance();
      return Token(SYMBOL_PLUS, "+", start);
    case '-':
      advance();
      return Token(SYMBOL_MINUS, "-", start);
    case '*':
      advance();
      return Token(SYMBOL_MULTIPLY, "*", start);
    case '/':
      advance();
      return Token(SYMBOL_DIVIDE, "/", start);
    case '%':
      advance();
      return Token(SYMBOL_MODULO, "%", start);
    case '=':
      advance();
      if (peek() == '=') {
        advance();
        return Token(SYMBOL_EQUAL, "==", start);
      }
      return Token(SYMBOL_ASSIGN, "=", start);
    case '!':
      advance();
      if (peek() == '=') {
        advance();
        return Token(SYMBOL_NOT_EQUAL, "!=", start);
      }
      return Token(SYMBOL_LOGICAL_NOT, "!", start);
    case '&':
      advance();
      if (peek() == '&') {
        advance();
        return Token(SYMBOL_LOGICAL_AND, "&&", start);
      } else if (peek() == '=') {
        advance();
        return Token(SYMBOL_BIT_AND_ASSIGN, "&=", start);
      }
      return Token(SYMBOL_BIT_AND, "&", start);
    case '|':
      advance();
      if (peek() == '|') {
        advance();
        return Token(SYMBOL_LOGICAL_OR, "||", start);
      } else if (peek() == '=') {
        advance();
        return Token(SYMBOL_BIT_OR_ASSIGN, "|=", start);
      }
      return Token(SYMBOL_BIT_OR, "|", start);
    case '^':
      advance();
      if (peek() == '=') {
        advance();
        return Token(SYMBOL_XOR_ASSIGN, "^=", start);
      }
      return Token(SYMBOL_XOR, "^", start);
    case '(':
      advance();
      return Token(SYMBOL_LPAREN, "(", start);
    case ')':
      advance();
      return Token(SYMBOL_RPAREN, ")", start);
    case '{':
      advance();
      return Token(SYMBOL_LBRACE, "{", start);
    case '}':
      advance();
      return Token(SYMBOL_RBRACE, "}", start);
    case '[':
      advance();
      return Token(SYMBOL_LBRACKET, "[", start);
    case ']':
      advance();
      return Token(SYMBOL_RBRACKET, "]", start);
    case ';':
      advance();
      return Token(SYMBOL_SEMICOLON, ";", start);
    case ',':
      advance();
      return Token(SYMBOL_COMMA, ",", start);
    case ':':
      advance();
      return Token(SYMBOL_COLON, ":", start);
    case '.':
      advance();
      return Token(SYMBOL_DOT, ".", start);
    case '>':
      advance();
      if (peek() == '=') {
        advance();
        return Token(SYMBOL_GREATER_EQUAL, ">=", start);
      } else if (peek() == '>') {
        advance();
        if (peek() == '=') {
          advance();
          return Token(SYMBOL_BIT_SHIFT_RIGHT_ASSIGN, ">>=", start);
        }
        return Token(SYMBOL_BIT_SHIFT_RIGHT, ">>", start);
      }
      return Token(SYMBOL_GREATER, ">", start);
    case '<':
      advance();
      if (peek() == '=') {
        advance();
        return Token(SYMBOL_LESS_EQUAL, "<=", start);
      } else if (peek() == '<') {
        advance();
        if (peek() == '=') {
          advance();
          return Token(SYMBOL_BIT_SHIFT_LEFT_ASSIGN, "<<=", start);
        }
        return Token(SYMBOL_BIT_SHIFT_LEFT, "<<", start);
      }
      return Token(SYMBOL_LESS, "<", start);
    default:
      advance();
      return Token(IDENTIFIER, std::string(1, c), start);
    }
  }
  return Token(EOF_TOKEN, "", pos);
}