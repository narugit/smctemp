/*
 * Apple System Management Control (SMC) Tool
 * Copyright (C) 2006 devnull 
 * Portions Copyright (C) 2013 Michael Wilber
 * Copyright (C) 2022 narugit
 *    - Modified Date: 2022/2/22
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SMC_H__
#define __SMC_H__
#endif

#define CMD_TOOL
#define VERSION               "0.01"

#define OP_NONE               0
#define OP_LIST               1 
#define OP_READ_CPU_TEMPS     2

#define KERNEL_INDEX_SMC      2

#define SMC_CMD_READ_BYTES    5
#define SMC_CMD_READ_INDEX    8
#define SMC_CMD_READ_KEYINFO  9
#define SMC_CMD_READ_PLIMIT   11
#define SMC_CMD_READ_VERS     12

#define DATATYPE_FLT          "flt "
#define DATATYPE_FP1F         "fp1f"
#define DATATYPE_FP4C         "fp4c"
#define DATATYPE_FP5B         "fp5b"
#define DATATYPE_FP6A         "fp6a"
#define DATATYPE_FP79         "fp79"
#define DATATYPE_FP88         "fp88"
#define DATATYPE_FPA6         "fpa6"
#define DATATYPE_FPC4         "fpc4"
#define DATATYPE_FPE2         "fpe2"

#define DATATYPE_SP1E         "sp1e"
#define DATATYPE_SP3C         "sp3c"
#define DATATYPE_SP4B         "sp4b"
#define DATATYPE_SP5A         "sp5a"
#define DATATYPE_SP69         "sp69"
#define DATATYPE_SP78         "sp78"
#define DATATYPE_SP87         "sp87"
#define DATATYPE_SP96         "sp96"
#define DATATYPE_SPB4         "spb4"
#define DATATYPE_SPF0         "spf0"

#define DATATYPE_UINT8        "ui8 "
#define DATATYPE_UINT16       "ui16"
#define DATATYPE_UINT32       "ui32"

#define DATATYPE_SI8          "si8 "
#define DATATYPE_SI16         "si16"

#define DATATYPE_FLT          "flt "

#define DATATYPE_PWM          "{pwm"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

typedef struct {
    char                  major;
    char                  minor;
    char                  build;
    char                  reserved[1]; 
    UInt16                release;
} SMCKeyData_vers_t;

typedef struct {
    UInt16                version;
    UInt16                length;
    UInt32                cpuPLimit;
    UInt32                gpuPLimit;
    UInt32                memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct {
    UInt32                dataSize;
    UInt32                dataType;
    char                  dataAttributes;
} SMCKeyData_keyInfo_t;

typedef unsigned char              SMCBytes_t[32];

typedef struct {
  UInt32                  key; 
  SMCKeyData_vers_t       vers; 
  SMCKeyData_pLimitData_t pLimitData;
  SMCKeyData_keyInfo_t    keyInfo;
  char                    result;
  char                    status;
  char                    data8;
  UInt32                  data32;
  SMCBytes_t              bytes;
} SMCKeyData_t;

typedef char              UInt32Char_t[5];

typedef struct {
  UInt32Char_t            key;
  UInt32                  dataSize;
  UInt32Char_t            dataType;
  SMCBytes_t              bytes;
} SMCVal_t;

UInt32 _strtoul(char *str, int size, int base);
float _strtof(unsigned char *str, int size, int e);

// Exclude command-line only code from smcFanControl UI
#ifdef CMD_TOOL

void smc_init();
void smc_close();
kern_return_t SMCReadKey(UInt32Char_t key, SMCVal_t *val);

#endif //#ifdef CMD_TOOL

kern_return_t SMCOpen(io_connect_t *conn);
kern_return_t SMCClose(io_connect_t conn);
kern_return_t SMCReadKey2(UInt32Char_t key, SMCVal_t *val,io_connect_t conn);

typedef struct {
    UInt32Char_t key;
    char name[40];
} Sensor_t;

// List of key and name: https://github.com/exelban/stats/blob/0e2e13c626b650ac7743ef620869d2b7857665cd/Modules/Sensors/values.swift#L206-L215
Sensor_t CPUECoreTemps[] = {
    {"Tp09", "CPU efficient core 1 temperature"},
    {"Tp0T", "CPU efficient core 2 temperature"},
};

Sensor_t CPUPCoreTemps[] = {
    {"Tp01", "CPU performance core 1 temperature"},
    {"Tp05", "CPU performance core 2 temperature"},
    {"Tp0D", "CPU performance core 3 temperature"},
    {"Tp0H", "CPU performance core 4 temperature"},
    {"Tp0L", "CPU performance core 5 temperature"},
    {"Tp0P", "CPU performance core 6 temperature"},
    {"Tp0X", "CPU performance core 7 temperature"},
    {"Tp0b", "CPU performance core 8 temperature"},
};

