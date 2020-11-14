#include "function_ref.hpp"
#include <exception>

namespace veg {

[[noreturn]] void terminate() {
  std::terminate();
}

} // namespace veg
