#include "function_ref.hpp"
#include <exception>
#include <cstdio>

namespace veg {

[[noreturn]] void terminate() {
  std::fputs("function_ref called in null state", stderr);
  std::terminate();
}

} // namespace veg
