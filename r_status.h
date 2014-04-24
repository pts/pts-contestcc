#ifndef R_STATUS_H
#define R_STATUS_H 1

#include "r_die.h"

namespace r {

class Status {
 public:
  // TODO(pts): Don't inline these constructs and destrutors?
  Status(bool is_ok)
      : msg_(is_ok ? 0/*nullptr*/ : "Something failed."), is_used_(false) {}
  // msg is owned externally.
  Status(char const *msg): msg_(msg), is_used_(false) {}
  ~Status() {
    // TODO(pts): Show stack trace.
    if (!is_used_ && msg_) die(msg_);
  }
  bool ok() const { is_used_ = true; return !msg_; }
  operator bool() const { return ok(); }

 private:
  const char *msg_;
  mutable bool is_used_;
};

}  // namespace r

#endif  // R_STATUS_H
