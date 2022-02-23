#ifndef SMCTEMP_SMCTEMP_TYPE_H_
#define SMCTEMP_SMCTEMP_TYPE_H_
#include <cstdint>

namespace smctemp {
constexpr char kDataTypeFlt[]  = "flt ";
constexpr char kDataTypeFp1f[] = "fp1f";
constexpr char kDataTypeFp4c[] = "fp4c";
constexpr char kDataTypeFp5b[] = "fp5b";
constexpr char kDataTypeFp6a[] = "fp6a";
constexpr char kDataTypeFp79[] = "fp79";
constexpr char kDataTypeFp88[] = "fp88";
constexpr char kDataTypeFpa6[] = "fpa6";
constexpr char kDataTypeFpc4[] = "fpc4";
constexpr char kDataTypeFpe2[] = "fpe2";
constexpr char kDataTypeSp1e[] = "sp1e";
constexpr char kDataTypeSp3c[] = "sp3c";
constexpr char kDataTypeSp4b[] = "sp4b";
constexpr char kDataTypeSp5a[] = "sp5a";
constexpr char kDataTypeSp69[] = "sp69";
constexpr char kDataTypeSp78[] = "sp78";
constexpr char kDataTypeSp87[] = "sp87";
constexpr char kDataTypeSp96[] = "sp96";
constexpr char kDataTypeSpb4[] = "spb4";
constexpr char kDataTypeSpf0[] = "spf0";
constexpr char kDataTypeUi8[]  = "ui8 ";
constexpr char kDataTypeUi16[] = "ui16";
constexpr char kDataTypeUi32[] = "ui32";
constexpr char kDataTypeUi64[] = "ui64";
constexpr char kDataTypeSi8[]  = "si8 ";
constexpr char kDataTypeSi16[] = "si16";
constexpr char kDataTypePwm[]  = "{pwm";

typedef struct {
  char major;
  char minor;
  char build;
  char reserved[1];
  uint16_t release;
} SmcKeyData_vers_t;

typedef struct {
  uint16_t version;
  uint16_t length;
  uint32_t cpuPLimit;
  uint32_t gpuPLimit;
  uint32_t memPLimit;
} SmcKeyData_pLimitData_t;

typedef struct {
  uint32_t dataSize;
  uint32_t dataType;
  char     dataAttributes;
} SmcKeyData_keyInfo_t;

typedef unsigned char SmcBytes_t[32];

typedef struct {
  uint32_t                key;
  SmcKeyData_vers_t       vers;
  SmcKeyData_pLimitData_t pLimitData;
  SmcKeyData_keyInfo_t    keyInfo;
  char                    result;
  char                    status;
  char                    data8;
  uint32_t                data32;
  SmcBytes_t              bytes;
} SmcKeyData_t;

typedef char UInt32Char_t[5];

typedef struct {
  UInt32Char_t key;
  uint32_t     dataSize;
  UInt32Char_t dataType;
  SmcBytes_t   bytes;
} SmcVal_t;
}
#endif // #ifndef SMCTEMP_SMCTEMP_TYPE_H_
