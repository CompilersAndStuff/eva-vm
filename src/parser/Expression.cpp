#include "Expression.h"

Exp::Exp(int number) : type(ExpType::NUMBER), number(number) {}

Exp::Exp(std::string &strVal) {
  if (strVal[0] == '"') {
    type = ExpType::STRING;
    string = strVal.substr(1, strVal.size() - 2);
  } else {
    type = ExpType::SYMBOL;
    string = strVal;
  }
}

Exp::Exp(std::vector<Exp> list) : type(ExpType::LIST), list(list) {}
