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

#ifndef SMCTEMP_H_
#define SMCTEMP_H_

#include <IOKit/IOKitLib.h>
#include <string>

#include "smctemp_types.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace smctemp {
constexpr float kVersion = 0.1;
constexpr char kIOAppleSmcHiddenClassName[] = "AppleSMC";
constexpr char kSmcCmdReadBytes = 5;
constexpr char kSmcCmdReadIndex = 8;
constexpr char kSmcCmdReadKeyInfo = 9;
constexpr uint32_t kKernelIndexSmc = 2;
constexpr int kOpNone = 0;
constexpr int kOpList = 1;
constexpr int kOpReadCpuTemp = 2;

// List of key and name: 
// - https://github.com/exelban/stats/blob/0e2e13c626b650ac7743ef620869d2b7857665cd/Modules/Sensors/values.swift
// - https://github.com/acidanthera/VirtualSMC/blob/632fec680d996a5dd015afd9acf0ba40f75e69e2/Docs/SMCSensorKeys.txt
#if defined(ARCH_TYPE_X86_64)
constexpr UInt32Char_t kSensorTc0d = "TC0D"; // CPU die temperature
constexpr UInt32Char_t kSensorTc0p = "TC0P"; // CPU proximity temperature
#elif defined(ARCH_TYPE_ARM64)
constexpr UInt32Char_t kSensorTp01 = "Tp01"; // CPU performance core 1 temperature
constexpr UInt32Char_t kSensorTp05 = "Tp05"; // CPU performance core 2 temperature
constexpr UInt32Char_t kSensorTp0d = "Tp0D"; // CPU performance core 3 temperature
constexpr UInt32Char_t kSensorTp0h = "Tp0H"; // CPU performance core 4 temperature
constexpr UInt32Char_t kSensorTp0l = "Tp0L"; // CPU performance core 5 temperature
constexpr UInt32Char_t kSensorTp0p = "Tp0P"; // CPU performance core 6 temperature
constexpr UInt32Char_t kSensorTp0x = "Tp0X"; // CPU performance core 7 temperature
constexpr UInt32Char_t kSensorTp0b = "Tp0b"; // CPU performance core 8 temperature
constexpr UInt32Char_t kSensorTp09 = "Tp09"; // CPU efficient core 1 temperature
constexpr UInt32Char_t kSensorTp0t = "Tp0T"; // CPU efficient core 2 temperature
#endif

class SmcAccessor {
 private:
  kern_return_t Open();
  kern_return_t Close();
  kern_return_t ReadSmcVal(const UInt32Char_t key, SmcVal_t& val);

  io_connect_t conn_ = 0;

 public:
  SmcAccessor();
  ~SmcAccessor();
  kern_return_t Call(int index, SmcKeyData_t *inputStructure, SmcKeyData_t *outputStructure);
  kern_return_t GetKeyInfo(const uint32_t key, SmcKeyData_keyInfo_t& key_info);
  double ReadValue(const UInt32Char_t key);
  uint32_t ReadIndexCount();
  kern_return_t PrintAll();
  void PrintSmcVal(SmcVal_t val);
  void PrintByteReadable(SmcVal_t val);
};

class SmcTemp {
 private:
  SmcAccessor smc_accessor_;
  double GetCpuTemp();

 public:
  SmcTemp() = default;
  ~SmcTemp() = default;
  void PrintCpuTemp();
};

typedef struct {
  UInt32Char_t key;
  char name[40];
} Sensor_t;

}
#endif //#ifndef SMCTEMP_H_
