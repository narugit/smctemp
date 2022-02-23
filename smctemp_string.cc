#include "smctemp_string.h"

#include <string>
namespace smctemp {
namespace string_util {
uint32_t strtoul(const char* str, int size, int base) {
  uint32_t total = 0;

  for (int i = 0; i < size; i++) {
    if (base == 16)
      total += str[i] << (size - 1 - i) * 8;
    else
      total += ((unsigned char) (str[i]) << (size - 1 - i) * 8);
  }
  return total;
}

void ultostr(char *str, uint32_t val) {
  str[0] = '\0';
  sprintf(str, "%c%c%c%c",
          (unsigned int) val >> 24,
          (unsigned int) val >> 16,
          (unsigned int) val >> 8,
          (unsigned int) val);
}
}
}
