#include "Logger.h"
#include <cstdlib>
#include <sstream>

ErrorLogMessage::~ErrorLogMessage() {
  fprintf(stderr, "\nFatal error: %s\n", str().c_str());
  exit(EXIT_FAILURE);
}
