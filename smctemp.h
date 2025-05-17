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
#include <utility>
#include <vector>

#include "smctemp_types.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace smctemp {
const char kVersion[] = "0.6.0";
constexpr char kIOAppleSmcHiddenClassName[] = "AppleSMC";
constexpr char kSmcCmdReadBytes = 5;
constexpr char kSmcCmdReadIndex = 8;
constexpr char kSmcCmdReadKeyInfo = 9;
constexpr uint32_t kKernelIndexSmc = 2;
constexpr int kOpNone = 0;
constexpr int kOpList = 1;
constexpr int kOpReadCpuTemp = 2;
constexpr int kOpReadGpuTemp = 3;
constexpr int kOpReadIndividualCpuTemp = 4;
constexpr int kOpReadIndividualGpuTemp = 5;

// List of key and name: 
// - https://github.com/exelban/stats/blob/6b88eb1f60a0eb5b1a7b51b54f044bf637fd785b/Modules/Sensors/values.swift
// - https://github.com/acidanthera/VirtualSMC/blob/632fec680d996a5dd015afd9acf0ba40f75e69e2/Docs/SMCSensorKeys.txt
#if defined(ARCH_TYPE_X86_64)
// CPU
constexpr UInt32Char_t kSensorTC0D = "TC0D";  // CPU die temperature
constexpr UInt32Char_t kSensorTC0E = "TC0E";  // CPU PECI die filtered temperature
constexpr UInt32Char_t kSensorTC0F = "TC0F";  // CPU PECI die temperature filtered then adjusted
constexpr UInt32Char_t kSensorTC0P = "TC0P";  // CPU proximity temperature
// GPU
constexpr UInt32Char_t kSensorTG0D = "TG0D";  // PCH Die Temp
constexpr UInt32Char_t kSensorTPCD = "TPCD";  // PCH Die Temp (digital)
#elif defined(ARCH_TYPE_ARM64)
// CPU
constexpr UInt32Char_t kSensorTc0a = "Tc0a";
constexpr UInt32Char_t kSensorTc0b = "Tc0b";
constexpr UInt32Char_t kSensorTc0x = "Tc0x";
constexpr UInt32Char_t kSensorTc0z = "Tc0z";
constexpr UInt32Char_t kSensorTp01 = "Tp01";
constexpr UInt32Char_t kSensorTp05 = "Tp05";
constexpr UInt32Char_t kSensorTp09 = "Tp09";
constexpr UInt32Char_t kSensorTp0D = "Tp0D";
constexpr UInt32Char_t kSensorTp0H = "Tp0H";
constexpr UInt32Char_t kSensorTp0L = "Tp0L";
constexpr UInt32Char_t kSensorTp0P = "Tp0P";
constexpr UInt32Char_t kSensorTp0T = "Tp0T";
constexpr UInt32Char_t kSensorTp0X = "Tp0X";
constexpr UInt32Char_t kSensorTp0b = "Tp0b";
constexpr UInt32Char_t kSensorTp0f = "Tp0f";
constexpr UInt32Char_t kSensorTp0j = "Tp0j";
constexpr UInt32Char_t kSensorTp0n = "Tp0n";
constexpr UInt32Char_t kSensorTp0r = "Tp0r";
constexpr UInt32Char_t kSensorTp1h = "Tp1h";
constexpr UInt32Char_t kSensorTp1t = "Tp1t";
constexpr UInt32Char_t kSensorTp1p = "Tp1p";
constexpr UInt32Char_t kSensorTp1l = "Tp1l";
// GPU
constexpr UInt32Char_t kSensorTg05 = "Tg05";
constexpr UInt32Char_t kSensorTg0D = "Tg0D";
constexpr UInt32Char_t kSensorTg0L = "Tg0L";
constexpr UInt32Char_t kSensorTg0P = "Tg0P";
constexpr UInt32Char_t kSensorTg0T = "Tg0T";
constexpr UInt32Char_t kSensorTg0X = "Tg0X";
constexpr UInt32Char_t kSensorTg0b = "Tg0b";
constexpr UInt32Char_t kSensorTg0f = "Tg0f";
constexpr UInt32Char_t kSensorTg0j = "Tg0j";
constexpr UInt32Char_t kSensorTg0v = "Tg0v";
constexpr UInt32Char_t kSensorTg1b = "Tg1b";
constexpr UInt32Char_t kSensorTg4b = "Tg4b";
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
  double CalculateAverageTemperature(const std::vector<std::string>& sensors,
                                     const std::pair<unsigned int, unsigned int>& limits);
  bool StoreValidTemperature(double temperature, std::string file_name);
  // Get labeled CPU core sensors for the current CPU model
  std::vector<std::pair<std::string, std::string>> GetCpuCoreSensors();
  // Get labeled GPU core sensors for the current CPU model
  std::vector<std::pair<std::string, std::string>> GetGpuCoreSensors();
  SmcAccessor smc_accessor_;
  bool is_fail_soft_;
  const std::string storage_path_ = "/tmp/smctemp/";
  const std::string cpu_file_ = "cpu_temperature.txt";
  const std::string gpu_file_ = "gpu_temperature.txt";

 public:
  explicit SmcTemp(bool isFailSoft);
  ~SmcTemp() = default;
  double GetCpuTemp();
  double GetGpuTemp();
  double GetLastValidCpuTemp();
  double GetLastValidGpuTemp();
  bool IsValidTemperature(double temperature, const std::pair<unsigned int, unsigned int>& limits);
  
  // Individual core temperatures
  std::vector<std::pair<std::string, double>> GetIndividualCpuTemps();
  std::vector<std::pair<std::string, double>> GetIndividualGpuTemps();
};

typedef struct {
  UInt32Char_t key;
  char name[40];
} Sensor_t;

}
#endif //#ifndef SMCTEMP_H_
