#ifndef SMCTEMP_SMCTEMP_STRING_H_
#define SMCTEMP_SMCTEMP_STRING_H_
#include <cstdint>

namespace smctemp {
namespace string_util {
uint32_t strtoul(const char * str, int size, int base);
void ultostr(char *str, uint32_t val);
}
}
#endif // #ifndef SMCTEMP_SMCTEMP_STRING_H_

