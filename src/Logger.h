#ifndef __Logger_h
#define __Logger_h

#include <cstdlib>
#include <sstream>
class ErrorLogMessage : public std::basic_ostringstream<char> {
public:
  ~ErrorLogMessage();
};

#define DIE ErrorLogMessage()

#define log(value) std::cout << #value << " = " << (value) << "\n";

#endif // __Logger_h
