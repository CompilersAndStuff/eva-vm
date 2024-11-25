#include "vm/EvaVm.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  EvaVm vm;

  auto program = R"(
    (var i 10)
    (var count 0)

    (while (> i 0)
      (begin 
        (set i (- i 1))
        (set count (+ count 1))))

    count
  )";

  log(program);

  auto result = vm.exec(program);

  log(result);

  std::cout << "All done!\n";

  return 0;
}
