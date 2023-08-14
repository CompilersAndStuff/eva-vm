#include <iostream>

#include "src/vm/EvaVM.h"
#include "src/vm/EvaValue.h"

int main(int argc, char *argv[]) {
  EvaVM vm;

  auto result = vm.exec(R"(
                        (if (== 1 5) 1 2)
                        )");

  log(result);


  std::cout << "All done!\n";

  return 0;
}
