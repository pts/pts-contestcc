#include "r_shiftout.h"

#include "r_die.h"

namespace r {

void FileShiftout::die_on_error() {
  die("Error writing to FILE* by operator<<.");
}

} // namespace r
