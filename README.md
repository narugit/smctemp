## smctemp
Print CPU and GPU temperatures on macOS.

It works on following macs.
- arm64 (M3 mac)
- arm64 (M2 mac)
- arm64 (M1 mac)
- x86_64

## Acknowledgements
I took the code from [hholtmann/smcFanControl/smc-command](https://github.com/hholtmann/smcFanControl/tree/ad374ffb1dd088a7676719e53dbd2886f8fafdff/smc-command) and modified it to specialize in temperature acquisition.
Some of the sensor values were obtained from the [exelban/stats](https://github.com/exelban/stats) project.

## How to Use
```console
$ git clone https://github.com/narugit/smctemp
$ cd smctemp
$ sudo make install
$ smctemp -c
```

## Usage 
```console
$ smctemp -h
Check Temperature by using Apple System Management Control (Smc) tool 0.3.1
Usage:
./smctemp [options]
    -c         : list CPU temperatures (Celsius)
    -g         : list GPU temperatures (Celsius)
    -h         : help
    -l         : list all keys and values
    -v         : version
    -n         : tries to query the temperature sensors for n times (e.g. -n3) (1 second interval) until a valid value is returned

$ smctemp -c
64.2

$ smctemp -g
36.2
```
