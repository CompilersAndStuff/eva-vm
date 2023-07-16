#include <iostream>

#include "src/vm/EvaVM.h"

int main(int argc, char *argv[]) {
  EvaVM vm;

  vm.exec(R"(
             42
            )");

  std::cout << "All done!\n";

  return 0;
}
