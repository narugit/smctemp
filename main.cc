#include <charconv>
#include <iomanip>
#include <iostream>

#include "smctemp.h"

void usage(char* prog) {
  std::cout << "Check Temperature by using Apple System Management Control (Smc) tool " << smctemp::kVersion << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << prog << " [options]" << std::endl;
  std::cout << "    -c         : list CPU temperatures (Celsius)" << std::endl;
  std::cout << "    -g         : list GPU temperatures (Celsius)" << std::endl;
  std::cout << "    -h         : help" << std::endl;
  std::cout << "    -i         : set interval in milliseconds (e.g. -i25, valid range is 20-1000, default: 1000)"
    << std::endl;
  std::cout << "    -l         : list all keys and values" << std::endl;
  std::cout << "    -f         : fail-soft mode. Shows last valid value if current sensor read fails." << std::endl;
  std::cout << "    -v         : version" << std::endl;
  std::cout << "    -n         : tries to query the temperature sensors for n times (e.g. -n3)";
  std::cout << " (1 second interval) until a valid value is returned" << std::endl;
}

int main(int argc, char *argv[]) {
  int c;
  unsigned int attempts = 1;
  unsigned int interval_ms = 1'000;

  kern_return_t result;
  int op = smctemp::kOpNone;
  bool isFailSoft = false;

  while ((c = getopt(argc, argv, "clvfhn:gi:")) != -1) {
    switch(c) {
      case 'c':
        op = smctemp::kOpReadCpuTemp;
        break;
      case 'g':
        op = smctemp::kOpReadGpuTemp;
        break;
      case 'i':
        if (optarg) {
          unsigned int temp_interval;
          auto [ptr, ec] = std::from_chars(optarg, optarg + strlen(optarg), temp_interval);
          if (ec != std::errc() || temp_interval < 20 || temp_interval > 1000) {
            std::cerr << "Invalid argument provided for -i (integer between 20 and 1000 is required)" << std::endl;
            return 1;
          }
          interval_ms = temp_interval;
        }
        break;
      case 'n':
        if (optarg) {
          auto [ptr, ec] = std::from_chars(optarg, optarg + strlen(optarg), attempts);
          if (ec != std::errc()) {
            std::cerr << "Invalid argument provided for -n (integer is required)" << std::endl;
            return 1;
          }
        }
        break;
      case 'l':
        op = smctemp::kOpList;
        break;
      case 'f':
        isFailSoft = true;
        break;
      case 'v':
        std::cout << smctemp::kVersion << std::endl;
        return 0;
        break;
      case 'h':
      case '?':
        op = smctemp::kOpNone;
        break;
    }
  }

  if (op == smctemp::kOpNone) {
    usage(argv[0]);
    return 1;
  }

  smctemp::SmcAccessor smc_accessor = smctemp::SmcAccessor();
  smctemp::SmcTemp smc_temp = smctemp::SmcTemp(isFailSoft);

  switch(op) {
    case smctemp::kOpList:
      result = smc_accessor.PrintAll();
      if (result != kIOReturnSuccess) {
        std::ios_base::fmtflags ef(std::cerr.flags());
        std::cerr << "Error: SmcPrintAll() = "
          << std::hex << result << std::endl;
        std::cerr.flags(ef);
      }
      break;
    case smctemp::kOpReadGpuTemp:
    case smctemp::kOpReadCpuTemp:
      double temp = 0.0;
      const std::pair<unsigned int, unsigned int> valid_temperature_limits{10, 120};
      while (attempts > 0) {
        if (op == smctemp::kOpReadCpuTemp) {
          temp = smc_temp.GetCpuTemp();
        } else if (op == smctemp::kOpReadGpuTemp) {
          temp = smc_temp.GetGpuTemp();
        }
        if (smc_temp.IsValidTemperature(temp, valid_temperature_limits)) {
          break;
        } else {
          usleep(interval_ms * 1'000);
          attempts--;
        }
      }
      if (isFailSoft) {
        if (!smc_temp.IsValidTemperature(temp, valid_temperature_limits)) {
          if (op == smctemp::kOpReadCpuTemp) {
            temp = smc_temp.GetLastValidCpuTemp();
          } else if (op == smctemp::kOpReadGpuTemp) {
            temp = smc_temp.GetLastValidGpuTemp();
          }
        }
      }
      std::cout << std::fixed << std::setprecision(1) << temp << std::endl;
      if (temp == 0.0) {
        std::cerr << "Could not get valid sensor value. Please use `-n` option and `-i` option." << std::endl;
        std::cerr << "In M2 Mac, it would be work fine with `-i25 -n180 -f` options.`" << std::endl;
        return 1;
      }
      break;
  }

  return 0;
}

