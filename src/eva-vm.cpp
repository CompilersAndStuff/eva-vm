#include "Logger.h"
#include "vm/EvaVm.h"
#include <iostream>

int main(int argc, char *argv[]) {
  {
    EvaVm vm;

    auto program = R"(
    (+ "Hello, " "World!")
    (+ "Hello, " "World!")
    )";

    log(program);

    auto result = vm.exec(program);

    log(result);
  }
  std::cout << "All done!\n";

  return 0;
}
