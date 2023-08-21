#include <iostream>

#include "src/vm/EvaVM.h"
#include "src/vm/EvaValue.h"

int main(int argc, char *argv[]) {
  EvaVM vm;

  auto result = vm.exec(R"(

                        (var x 5)
                        (set x (+ x 10))

                        (begin
                          (set x 1000)
                          (var x 100)
                          x)

                        x

                       )");

  log(result);


  std::cout << "All done!\n";

  return 0;
}
