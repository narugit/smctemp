## smctemp
Print CPU and GPU temperatures on macOS.

It works on following macs.
- arm64 (M4 mac)
- arm64 (M3 mac)
- arm64 (M2 mac)
- arm64 (M1 mac)
- x86_64

## Acknowledgements
I took the code from [hholtmann/smcFanControl/smc-command](https://github.com/hholtmann/smcFanControl/tree/ad374ffb1dd088a7676719e53dbd2886f8fafdff/smc-command) and modified it to specialize in temperature acquisition.
Some of the sensor values were obtained from the [exelban/stats](https://github.com/exelban/stats) project.

## Installation
### Brew (Recommended)
```bash
brew tap narugit/tap
brew install narugit/tap/smctemp
```

### Manual
```bash
git clone https://github.com/narugit/smctemp
cd smctemp
sudo make install
```

## Usage 
```console
$ smctemp -h
Check Temperature by using Apple System Management Control (Smc) tool
Usage:
./smctemp [options]
    -c         : list CPU temperatures (Celsius)
    -g         : list GPU temperatures (Celsius)
    -C         : list individual CPU core temperatures (Celsius)
    -G         : list individual GPU core temperatures (Celsius)
    -h         : help
    -i         : set interval in milliseconds (e.g. -i25, valid range is 20-1000, default: 1000)
    -l         : list all keys and values
    -f         : fail-soft mode. Shows last valid value if current sensor read fails.
    -v         : version
    -n         : tries to query the temperature sensors for n times (e.g. -n3) until a valid value is returned

$ smctemp -c
64.2

$ smctemp -g
36.2

$ smctemp -C
P-Core 1: 62.4°C
P-Core 2: 63.8°C
P-Core 3: 65.2°C
P-Core 4: 64.8°C
E-Core 1: 58.6°C
E-Core 2: 59.1°C
```

## Note for M2 Mac Users
On M2 Macs, sensor values may be unstable as described in the following issue:
- https://github.com/narugit/smctemp/pull/14
- https://github.com/narugit/smctemp/issues/32

For M2 Macs, using the `-n`, `-i`, and `-f` options can help obtain more stable sensor values.
Try tuning these options to get better results.

The recommended settings are `-i25 -n180 -f` (See also https://github.com/narugit/smctemp/pull/34/files#r1721025001)
