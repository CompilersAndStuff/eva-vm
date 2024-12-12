#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <vector>

/**
 * Expression type.
 */
enum class ExpType {
  NUMBER,
  STRING,
  SYMBOL,
  LIST,
};

/**
 * Expression.
 */
struct Exp {
  ExpType type;

  int number;
  std::string string;
  std::vector<Exp> list;

  // Numbers:
  Exp(int number);

  // Strings, Symbols:
  Exp(std::string& strVal);

  // Lists:
  Exp(std::vector<Exp> list);

};

#endif
