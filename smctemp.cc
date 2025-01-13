/*
 * Apple System Management Control (SMC) Tool
 * Copyright (C) 2006 devnull
 * Portions Copyright (C) 2013 Michael Wilber
 * Copyright (C) 2022 narugit
 *   - Modified Date: 2022/2/22
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

#include "smctemp.h"

#include <IOKit/IOKitLib.h>
#include <libkern/OSAtomic.h>
#include <sys/stat.h>

#include <cerrno>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "smctemp_string.h"

#if defined(ARCH_TYPE_ARM64)
#include <sys/sysctl.h>
#include <algorithm>
#include <array>

namespace {
std::string getCPUModel() {
  std::array<char, 512> buffer;
  size_t bufferLength = buffer.size();
  sysctlbyname("machdep.cpu.brand_string", buffer.data(), &bufferLength, nullptr, 0);
  std::string cpuModel = buffer.data();
  std::transform(cpuModel.begin(), cpuModel.end(), cpuModel.begin(), ::tolower);
  return cpuModel;
}
}
#endif

// Cache the keyInfo to lower the energy impact of SmcReadKey() / SmcReadKey2()
#define KEY_INFO_CACHE_SIZE 100
namespace smctemp {
struct {
    uint32_t key;
    SmcKeyData_keyInfo_t keyInfo;
} g_keyInfoCache[KEY_INFO_CACHE_SIZE];

int g_keyInfoCacheCount = 0;
OSSpinLock g_keyInfoSpinLock = 0;

void printFLT(SmcVal_t val) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::fixed << std::setprecision(0)
    << *reinterpret_cast<float*>(val.bytes);
  std::cout.flags(f);
}

void printFP(SmcVal_t val, int n, float m) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::fixed << std::setprecision(n)
    << ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / m;
  std::cout.flags(f);
}

void printUInt(SmcVal_t val) {
  char* bytes = (char *)val.bytes;
  uint64_t data = 0;
  for (uint32_t i = 0; i < val.dataSize; i++) {
    data += uint8_t(bytes[i]) * std::pow(256, val.dataSize - 1 -i);
  }
  std::cout << data;
}

void printSP(SmcVal_t val, int n, float m) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::fixed << std::setprecision(n)
    << (int16_t)ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / m;
  std::cout.flags(f);
}

void printSI8(SmcVal_t val) {
  signed char* bytes = (signed char *)val.bytes;
  int16_t data = 0;
  data += int8_t(bytes[0]);
  std::cout << data;
}

void printSI16(SmcVal_t val) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << ntohs(*reinterpret_cast<int16_t*>(val.bytes));
  std::cout.flags(f);
}

void printPWM(SmcVal_t val) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::fixed << std::setprecision(1)
    << (float)ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) * 100 / 65536.0;
  std::cout.flags(f);
}

void printBytesHex(SmcVal_t val) {
  std::cout << " (bytes:";
  for (uint32_t i = 0; i < val.dataSize; i++) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << " " << std::setw(2)
      << std::uppercase<< std::hex << std::setfill('0')
      << static_cast<unsigned int>(val.bytes[i]);
    std::cout.flags(f);
  }
  std::cout << ")" << std::endl;
}

void SmcAccessor::PrintByteReadable(SmcVal_t val) {
  double value = ReadValue(val.key);
  std::cout << std::fixed << std::setprecision(1) << value;
}

void SmcAccessor::PrintSmcVal(SmcVal_t val) {
  std::ios_base::fmtflags f(std::cout.flags());
  std::cout << std::setw(6) << std::setfill(' ') << val.key;
  std::cout << std::setw(10) << std::setfill(' ') << "[" + std::string(val.dataType) + "]  ";
  if (val.dataSize > 0) {
    PrintByteReadable(val);
    printBytesHex(val);
  } else {
    std::cout << "no data" << std::endl;
  }
  std::cout.flags(f);
}

SmcAccessor::SmcAccessor() {
  Open();
}

SmcAccessor::~SmcAccessor() {
  Close();
}

kern_return_t SmcAccessor::Open() {
  mach_port_t masterPort;
  IOMasterPort(MACH_PORT_NULL, &masterPort);
  CFMutableDictionaryRef matchingDictionary = IOServiceMatching(kIOAppleSmcHiddenClassName);

  io_iterator_t iterator;
  kern_return_t result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
  if (result != kIOReturnSuccess) {
    std::ios_base::fmtflags ef(std::cerr.flags());
    std::cerr << "Error: IOServiceGetMatchingServices() = "
      << std::hex << result << std::endl;
    std::cerr.flags(ef);
    return result;
  }

  io_object_t device = IOIteratorNext(iterator);
  IOObjectRelease(iterator);
  if (device == 0) {
    std::ios_base::fmtflags ef(std::cerr.flags());
    std::cerr << "Error: no Smc found" << std::endl;
    std::cerr.flags(ef);
    return result;
  }

  result = IOServiceOpen(device, mach_task_self(), 0, &conn_);
  IOObjectRelease(device);
  if (result != kIOReturnSuccess) {
    std::ios_base::fmtflags ef(std::cerr.flags());
    std::cerr << "Error: IOServiceGetMatchingServices() = "
      << std::hex << result << std::endl;
    std::cerr.flags(ef);
    return result;
  }
  return kIOReturnSuccess;
}

kern_return_t SmcAccessor::Close() {
  return IOServiceClose(conn_);
}

kern_return_t SmcAccessor::Call(int index, SmcKeyData_t *inputStructure, SmcKeyData_t *outputStructure) {
  size_t   structureInputSize;
  size_t   structureOutputSize;
  structureInputSize = sizeof(SmcKeyData_t);
  structureOutputSize = sizeof(SmcKeyData_t);

  return IOConnectCallStructMethod(conn_, index, inputStructure, structureInputSize, outputStructure, &structureOutputSize);
}
kern_return_t SmcCall2(int index, SmcKeyData_t *inputStructure, SmcKeyData_t *outputStructure,io_connect_t conn) {
  size_t   structureInputSize;
  size_t   structureOutputSize;
  structureInputSize = sizeof(SmcKeyData_t);
  structureOutputSize = sizeof(SmcKeyData_t);

  return IOConnectCallStructMethod(conn, index, inputStructure, structureInputSize, outputStructure, &structureOutputSize);
}

// Provides key info, using a cache to dramatically improve the energy impact of smcFanControl
kern_return_t SmcAccessor::GetKeyInfo(const uint32_t key, SmcKeyData_keyInfo_t& key_info) {
  SmcKeyData_t inputStructure;
  SmcKeyData_t outputStructure;
  kern_return_t result = kIOReturnSuccess;

  OSSpinLockLock(&g_keyInfoSpinLock);
  int i = 0;
  for (i = 0; i < g_keyInfoCacheCount; ++i) {
    if (key == g_keyInfoCache[i].key) {
      key_info = g_keyInfoCache[i].keyInfo;
      break;
    }
  }

  if (i == g_keyInfoCacheCount) {
    // Not in cache, must look it up.
    memset(&inputStructure, 0, sizeof(inputStructure));
    memset(&outputStructure, 0, sizeof(outputStructure));

    inputStructure.key = key;
    inputStructure.data8 = kSmcCmdReadKeyInfo;

    result = Call(kKernelIndexSmc, &inputStructure, &outputStructure);
    if (result == kIOReturnSuccess) {
      key_info = outputStructure.keyInfo;
      if (g_keyInfoCacheCount < KEY_INFO_CACHE_SIZE) {
        g_keyInfoCache[g_keyInfoCacheCount].key = key;
        g_keyInfoCache[g_keyInfoCacheCount].keyInfo = outputStructure.keyInfo;
        ++g_keyInfoCacheCount;
      }
    }
  }

  OSSpinLockUnlock(&g_keyInfoSpinLock);

  return result;
}

double SmcAccessor::ReadValue(const UInt32Char_t key) {
  SmcVal_t val;
  ReadSmcVal(key, val);
  double v = 0.0;

  if (std::string(val.dataType) == kDataTypeUi8 ||
      std::string(val.dataType) == kDataTypeUi16 ||
      std::string(val.dataType) == kDataTypeUi32 ||
      std::string(val.dataType) == kDataTypeUi64) {
    char* bytes = (char *)val.bytes;
    uint64_t tmp = 0;
    for (uint32_t i = 0; i < val.dataSize; i++) {
      tmp += uint8_t(bytes[i]) * std::pow(256, val.dataSize - 1 -i);
    }
    v = tmp;
  } else if (std::string(val.dataType) == kDataTypeFlt) {
    v = *reinterpret_cast<float*>(val.bytes);
  } else if (std::string(val.dataType) == kDataTypeFp1f && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 32768.0;
  } else if (std::string(val.dataType) == kDataTypeFp4c && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 4096.0;
  } else if (std::string(val.dataType) == kDataTypeFp5b && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 2048.0;
  } else if (std::string(val.dataType) == kDataTypeFp6a && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 1024.0;
  } else if (std::string(val.dataType) == kDataTypeFp79 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 512.0;
  } else if (std::string(val.dataType) == kDataTypeFp88 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 256.0;
  } else if (std::string(val.dataType) == kDataTypeFpa6 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 64.0;
  } else if (std::string(val.dataType) == kDataTypeFpc4 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 16.0;
  } else if (std::string(val.dataType) == kDataTypeFpe2 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 4.0;
  } else if (std::string(val.dataType) == kDataTypeSp1e && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 16384.0;
  } else if (std::string(val.dataType) == kDataTypeSp3c && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 4096.0;
  } else if (std::string(val.dataType) == kDataTypeSp4b && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 2048.0;
  } else if (std::string(val.dataType) == kDataTypeSp5a && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 1024.0;
  } else if (std::string(val.dataType) == kDataTypeSp69 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 512.0;
  } else if (std::string(val.dataType) == kDataTypeSp78 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 256.0;
  } else if (std::string(val.dataType) == kDataTypeSp87 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 128.0;
  } else if (std::string(val.dataType) == kDataTypeSp96 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 64.0;
  } else if (std::string(val.dataType) == kDataTypeSpb4 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 16.0;
  } else if (std::string(val.dataType) == kDataTypeSpf0 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) / 1.0;
  } else if (std::string(val.dataType) == kDataTypeSi8 && val.dataSize == 1) {
    signed char* bytes = (signed char *)val.bytes;
    int16_t temp = 0;
    temp += int8_t(bytes[0]);
    v = temp;
  } else if (std::string(val.dataType) == kDataTypeSi16 && val.dataSize == 2) {
    v = ntohs(*reinterpret_cast<int16_t*>(val.bytes));
  } else if (std::string(val.dataType) == kDataTypePwm && val.dataSize == 2) {
    v = (float)ntohs(*reinterpret_cast<uint16_t*>(val.bytes)) * 100 / 65536.0;
  }

  return static_cast<double>(v);
}

kern_return_t SmcAccessor::ReadSmcVal(const UInt32Char_t key, SmcVal_t& val) {
  kern_return_t result;
  SmcKeyData_t  inputStructure;
  SmcKeyData_t  outputStructure;

  memset(&inputStructure, 0, sizeof(SmcKeyData_t));
  memset(&outputStructure, 0, sizeof(SmcKeyData_t));
  memset(&val, 0, sizeof(SmcVal_t));

  inputStructure.key = string_util::strtoul(key, 4, 16);
  snprintf(val.key, sizeof(val.key), key);

  result = GetKeyInfo(inputStructure.key, outputStructure.keyInfo);
  if (result != kIOReturnSuccess) {
    return result;
  }

  val.dataSize = outputStructure.keyInfo.dataSize;
  string_util::ultostr(val.dataType, 5, outputStructure.keyInfo.dataType);
  inputStructure.keyInfo.dataSize = val.dataSize;
  inputStructure.data8 = kSmcCmdReadBytes;

  result = SmcCall2(kKernelIndexSmc, &inputStructure, &outputStructure, conn_);
  if (result != kIOReturnSuccess) {
    return result;
  }

  memcpy(val.bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

  return kIOReturnSuccess;
}

uint32_t SmcAccessor::ReadIndexCount() {
  SmcVal_t val;

  ReadSmcVal("#KEY", val);
  return string_util::strtoul((const char *)val.bytes, val.dataSize, 10);
}

kern_return_t SmcAccessor::PrintAll() {
  kern_return_t result;
  SmcKeyData_t  inputStructure;
  SmcKeyData_t  outputStructure;

  int           totalKeys, i;
  UInt32Char_t  key;
  SmcVal_t      val;

  totalKeys = ReadIndexCount();
  for (i = 0; i < totalKeys; i++) {
    memset(&inputStructure, 0, sizeof(SmcKeyData_t));
    memset(&outputStructure, 0, sizeof(SmcKeyData_t));
    memset(&val, 0, sizeof(SmcVal_t));

    inputStructure.data8 = kSmcCmdReadIndex;
    inputStructure.data32 = i;

    result = Call(kKernelIndexSmc, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        continue;

    string_util::ultostr(key, 5, outputStructure.key);
    ReadSmcVal(key, val);
    PrintSmcVal(val);
  }

  return kIOReturnSuccess;
}

SmcTemp::SmcTemp(bool isFailSoft)
    : is_fail_soft_(isFailSoft) {
  if (is_fail_soft_) {
    if (mkdir(storage_path_.c_str(), 0777) && errno != EEXIST) {
      std::cerr << "Failed to create directory: " << storage_path_ << std::endl;
    }
  }
}

bool SmcTemp::IsValidTemperature(double temperature, const std::pair<unsigned int, unsigned int>& limits) {
  return temperature > limits.first && temperature < limits.second;
}

bool SmcTemp::StoreValidTemperature(double temperature, std::string file_name) {
  if (!is_fail_soft_) {
    return false;
  }
  std::ofstream out_file(storage_path_ + file_name);

  if (!out_file) {
    std::cerr << "Failed to open the file: " << storage_path_ + file_name << std::endl;
    return false;
  }

  out_file << temperature;
  out_file.close();

  return true;
}

double SmcTemp::CalculateAverageTemperature(const std::vector<std::string>& sensors,
                                     const std::pair<unsigned int, unsigned int>& limits) {
  double temp = 0.0;
  size_t valid_sensor_count = 0;
  for (auto sensor : sensors) {
    auto sensor_value = smc_accessor_.ReadValue(sensor.c_str());
    if (IsValidTemperature(sensor_value, limits)) {
      temp += sensor_value;
      valid_sensor_count++;
    }
  }
  if (valid_sensor_count > 0) {
    temp /= valid_sensor_count;
  }
  return temp;
}

double SmcTemp::GetCpuTemp() {
  double temp = 0.0;
#if defined(ARCH_TYPE_X86_64)
  const std::pair<unsigned int, unsigned int> valid_temperature_limits{0, 110};
  // The reason why I prefer CPU die temperature to CPU proximity temperature:
  // https://github.com/narugit/smctemp/issues/2
  temp = smc_accessor_.ReadValue(kSensorTC0D);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, cpu_file_);
    return temp;
  }
  temp = smc_accessor_.ReadValue(kSensorTC0E);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, cpu_file_);
    return temp;
  }
  temp = smc_accessor_.ReadValue(kSensorTC0F);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, cpu_file_);
    return temp;
  }
  temp = smc_accessor_.ReadValue(kSensorTC0P);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, cpu_file_);
    return temp;
  }
#elif defined(ARCH_TYPE_ARM64)
  std::vector<std::string> sensors;
  std::vector<std::string> aux_sensors;
  const std::pair<unsigned int, unsigned int> valid_temperature_limits{10, 120};

  const std::string cpumodel = getCPUModel();
  if (cpumodel.find("m4") != std::string::npos) {  // Apple M4
    sensors.emplace_back(static_cast<std::string>(kSensorTp01));
    sensors.emplace_back(static_cast<std::string>(kSensorTp09));
    sensors.emplace_back(static_cast<std::string>(kSensorTp0f));
    sensors.emplace_back(static_cast<std::string>(kSensorTp05));
    sensors.emplace_back(static_cast<std::string>(kSensorTp0D));
  } else if (cpumodel.find("m3") != std::string::npos) {  // Apple M3
    // CPU core 1
    sensors.emplace_back(static_cast<std::string>(kSensorTp01));
    // CPU core 2
    sensors.emplace_back(static_cast<std::string>(kSensorTp09));
    // CPU core 3
    sensors.emplace_back(static_cast<std::string>(kSensorTp0f));
    // CPU core 4
    sensors.emplace_back(static_cast<std::string>(kSensorTp0n));
    // CPU core 5
    sensors.emplace_back(static_cast<std::string>(kSensorTp05));
    // CPU core 6
    sensors.emplace_back(static_cast<std::string>(kSensorTp0D));
    // CPU core 7
    sensors.emplace_back(static_cast<std::string>(kSensorTp0j));
    // CPU core 8
    sensors.emplace_back(static_cast<std::string>(kSensorTp0r));
  } else if (cpumodel.find("m2") != std::string::npos) {  // Apple M2
    // CPU efficient cores 1 through 4 on M2 Max 12 Core Chip
    sensors.emplace_back(static_cast<std::string>(kSensorTp1h));
    sensors.emplace_back(static_cast<std::string>(kSensorTp1t));
    sensors.emplace_back(static_cast<std::string>(kSensorTp1p));
    sensors.emplace_back(static_cast<std::string>(kSensorTp1l));

    // CPU core 1
    sensors.emplace_back(static_cast<std::string>(kSensorTp01));
    // CPU core 2
    sensors.emplace_back(static_cast<std::string>(kSensorTp09));
    // CPU core 3
    sensors.emplace_back(static_cast<std::string>(kSensorTp0f));
    // CPU core 4
    sensors.emplace_back(static_cast<std::string>(kSensorTp0n));
    // CPU core 5
    sensors.emplace_back(static_cast<std::string>(kSensorTp05));
    // CPU core 6
    sensors.emplace_back(static_cast<std::string>(kSensorTp0D));
    // CPU core 7
    sensors.emplace_back(static_cast<std::string>(kSensorTp0j));
    // CPU core 8
    sensors.emplace_back(static_cast<std::string>(kSensorTp0r));
  } else if (cpumodel.find("m1") != std::string::npos) {  // Apple M1
    // CPU performance core 1 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp01));
    // CPU performance core 2 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp05));
    // CPU performance core 3 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0D));
    // CPU performance core 4 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0H));
    // CPU performance core 5 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0L));
    // CPU performance core 6 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0P));
    // CPU performance core 7 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0X));
    // CPU performance core 8 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0b));
    // CPU efficient core 1 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp09));
    // CPU efficient core 2 temperature
    sensors.emplace_back(static_cast<std::string>(kSensorTp0T));

    aux_sensors.emplace_back(static_cast<std::string>(kSensorTc0a));
    aux_sensors.emplace_back(static_cast<std::string>(kSensorTc0b));
    aux_sensors.emplace_back(static_cast<std::string>(kSensorTc0x));
    aux_sensors.emplace_back(static_cast<std::string>(kSensorTc0z));
  } else {
    // not supported
    return temp;
  }

  temp = CalculateAverageTemperature(sensors, valid_temperature_limits);
  if (temp > std::numeric_limits<double>::epsilon()) {
    if (IsValidTemperature(temp, valid_temperature_limits)) {
      StoreValidTemperature(temp, cpu_file_);
    }
    return temp;
  }

  temp = CalculateAverageTemperature(aux_sensors, valid_temperature_limits);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, cpu_file_);
  }
#endif

  return temp;
}

double SmcTemp::GetGpuTemp() {
  double temp = 0.0;
#if defined(ARCH_TYPE_X86_64)
  const std::pair<unsigned int, unsigned int> valid_temperature_limits{0, 110};
  temp = smc_accessor_.ReadValue(kSensorTG0D);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, gpu_file_);
    return temp;
  }
  temp = smc_accessor_.ReadValue(kSensorTPCD);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, gpu_file_);
    return temp;
  }
#elif defined(ARCH_TYPE_ARM64)
  std::vector<std::string> sensors;
  const std::pair<unsigned int, unsigned int> valid_temperature_limits{10, 120};
  const std::string cpumodel = getCPUModel();
  if (cpumodel.find("m3") != std::string::npos) {  // Apple M3
    sensors.emplace_back(static_cast<std::string>(kSensorTg0D));  // GPU 1
    sensors.emplace_back(static_cast<std::string>(kSensorTg0P));  // GPU 2
    sensors.emplace_back(static_cast<std::string>(kSensorTg0X));  // GPU 3
    sensors.emplace_back(static_cast<std::string>(kSensorTg0b));  // GPU 4
    sensors.emplace_back(static_cast<std::string>(kSensorTg0j));  // GPU 5
    sensors.emplace_back(static_cast<std::string>(kSensorTg0v));  // GPU 6
  } else if (cpumodel.find("m2") != std::string::npos) {  // Apple M2
    // ref: https://github.com/exelban/stats/blob/6b88eb1f60a0eb5b1a7b51b54f044bf637fd785b/Modules/Sensors/values.swift#L369-L370
    sensors.emplace_back(static_cast<std::string>(kSensorTg0f));  // GPU 1
    sensors.emplace_back(static_cast<std::string>(kSensorTg0j));  // GPU 2
  } else if (cpumodel.find("m1") != std::string::npos) {  // Apple M1
    // ref: https://github.com/exelban/stats/blob/6b88eb1f60a0eb5b1a7b51b54f044bf637fd785b/Modules/Sensors/values.swift#L354-L357
    sensors.emplace_back(static_cast<std::string>(kSensorTg05));  // GPU 1
    sensors.emplace_back(static_cast<std::string>(kSensorTg0D));  // GPU 2
    sensors.emplace_back(static_cast<std::string>(kSensorTg0L));  // GPU 3
    sensors.emplace_back(static_cast<std::string>(kSensorTg0T));  // GPU 4
    // ref: runtime detected on a M1 mac mini
    sensors.emplace_back(static_cast<std::string>(kSensorTg1b));  // GPU 5
    sensors.emplace_back(static_cast<std::string>(kSensorTg4b));  // GPU 6
  } else {
    // not supported
    return temp;
  }
  temp = CalculateAverageTemperature(sensors, valid_temperature_limits);
  if (IsValidTemperature(temp, valid_temperature_limits)) {
    StoreValidTemperature(temp, gpu_file_);
  }
#endif
  return temp;
}

double SmcTemp::GetLastValidCpuTemp() {
  std::string file_path = storage_path_ + cpu_file_;
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Failed to open the file: " << file_path << std::endl;
  }

  double value;
  file >> value;

  if (file.fail()) {
    std::cerr << "Failed to read sensor value from file: " + file_path << std::endl;
  }

  return value;
}

double SmcTemp::GetLastValidGpuTemp() {
  std::string file_path = storage_path_ + gpu_file_;
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Failed to open the file: " << file_path << std::endl;
  }

  double value;
  file >> value;

  if (file.fail()) {
    std::cerr << "Failed to read sensor value from file: " + file_path << std::endl;
  }

  return value;
}
}

