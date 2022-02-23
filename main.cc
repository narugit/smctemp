#include <iostream>

#include "smctemp.h"

void usage(char* prog) {
  std::cout << "Check Temperature by using Apple System Management Control (Smc) tool " << smctemp::kVersion << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << prog << " [options]" << std::endl;
  std::cout << "    -c         : list CPU temperatures" << std::endl;
  std::cout << "    -h         : help" << std::endl;
  std::cout << "    -l         : list all keys and values" << std::endl;
  std::cout << "    -v         : version" << std::endl;
}

int main(int argc, char *argv[]) {
  int c;
  extern char   *optarg;

  kern_return_t result;
  int           op = smctemp::kOpNone;
  smctemp::UInt32Char_t  key = { 0 };
  smctemp::SmcVal_t      val;

  while ((c = getopt(argc, argv, "clvh")) != -1) {
    switch(c) {
      case 'c':
        op = smctemp::kOpReadCpuTemp;
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
    case smctemp::kOpReadCpuTemp:
      smc_temp.PrintCpuTemp();
      break;
  }

  return 0;
}

