#include "include/info.h"
#include "include/types.h"
#include "include/syscall.h"
#include "include/string.h"

int uname(struct utsname *uts)
{
  strncpy(uts -> sysname, "origin", sizeof(uts -> sysname));
  strncpy(uts -> nodename, "qemu", sizeof(uts -> nodename));
  strncpy(uts -> release, "0.0", sizeof(uts -> release));
  strncpy(uts -> version, "a very first version", sizeof(uts -> version));
  strncpy(uts -> machine, "rv64 on qemu", sizeof(uts -> machine));
  strncpy(uts -> domainname, "0.0.0.0", sizeof(uts -> domainname));
  return 0;
}