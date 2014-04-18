#include "r_shiftout.h"

#include "r_die.h"

void FileShiftout::die_on_error() {
  die("Error writing to FILE* by operator<<.");
}
