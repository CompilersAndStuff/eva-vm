#include "vm/EvaVm.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  EvaVm vm;

  auto program = R"(
  (var x 1)
  (var z 2)

  (var y (+ x 1))

  (begin 
    (var a 10)
    (var b 20)
    (set a 100)
    (+ a b))
  )";

  log(program);

  auto result = vm.exec(program);

  log(result);

  std::cout << "All done!\n";

  return 0;
}
