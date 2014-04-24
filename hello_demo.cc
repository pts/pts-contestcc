#include "r.h"

#include <string>

int main() {
  sout << "What is your name?\n";
  std::string name;
  // TODO(pts): Read line, and strip.
  sin >> word(&name);
  sout << "Hello, " << name << '!' << endl;
  static const int kAnswers[] = {-42, 42};
  dump("Answers: ", kAnswers);
  return 0;
}
