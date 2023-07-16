#include <iostream>

#include "src/vm/EvaVM.h"
#include "src/vm/EvaValue.h"

int main(int argc, char *argv[]) {
  EvaVM vm;

  auto result = vm.exec(R"(
                            (+ "Hello, " "World!")
                          )");

  log(result);

  std::cout << "All done!\n";

  return 0;
}
