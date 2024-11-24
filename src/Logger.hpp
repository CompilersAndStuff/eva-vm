#ifndef __Logger_hpp
#define __Logger_hpp

#include <cstdlib>
#include <sstream>
class ErrorLogMessage : public std::basic_ostringstream<char> {
public:
  ~ErrorLogMessage() {
    fprintf(stderr, "\nFatal error: %s\n", str().c_str());
    exit(EXIT_FAILURE);
  }
};

#define DIE ErrorLogMessage()

#define log(value) std::cout << #value << " = " << (value) << "\n";

#endif // __Logger_hpp
