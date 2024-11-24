#include "vm/EvaVm.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  EvaVm vm;

  auto program = R"(
    (begin (var x 5) (var z 6))
  )";

  log(program);

  auto result = vm.exec(program);

  log(result);

  std::cout << "All done!\n";

  return 0;
}
