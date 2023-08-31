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
  std::cout << "    -l         : list all keys and values" << std::endl;
  std::cout << "    -v         : version" << std::endl;
  std::cout << "    -n         : tries to query the temperature sensors for n times (e.g. -n3)";
  std::cout << " (1 second interval) until a valid value is returned" << std::endl;
}

int main(int argc, char *argv[]) {
  int c;
  unsigned int attempts = 1;

  kern_return_t result;
  int           op = smctemp::kOpNone;
  smctemp::UInt32Char_t  key = { 0 };
  smctemp::SmcVal_t      val;

  while ((c = getopt(argc, argv, "clvhn:g")) != -1) {
    switch(c) {
      case 'c':
        op = smctemp::kOpReadCpuTemp;
        break;
      case 'g':
        op = smctemp::kOpReadGpuTemp;
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
  smctemp::SmcTemp smc_temp = smctemp::SmcTemp();

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
      while (attempts > 0) {
        if (op == smctemp::kOpReadCpuTemp) {
          temp = smc_temp.GetCpuTemp();
        } else if (op == smctemp::kOpReadGpuTemp) {
          temp = smc_temp.GetGpuTemp();
        }
        if (temp > 0.0) {
          break;
        } else {
          usleep(1'000'000);
          attempts--;
        }
      }
      std::cout << std::fixed << std::setprecision(1) << temp;
      break;
  }

  return 0;
}

