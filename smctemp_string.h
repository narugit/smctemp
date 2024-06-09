#ifndef SMCTEMP_SMCTEMP_STRING_H_
#define SMCTEMP_SMCTEMP_STRING_H_
#include <cstdint>
#include <cstdlib>

namespace smctemp {
namespace string_util {
uint32_t strtoul(const char * str, int size, int base);
void ultostr(char* str, size_t strlen, uint32_t val);
}
}
#endif // #ifndef SMCTEMP_SMCTEMP_STRING_H_

