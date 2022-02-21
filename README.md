## Acknowledgements
I took the code from [hholtmann/smcFanControl/smc-command](https://github.com/hholtmann/smcFanControl/tree/ad374ffb1dd088a7676719e53dbd2886f8fafdff/smc-command) and modified it to specialize in temperature acquisition.

## Usage 
```console
$ ./smctemp -h
Get temperature by using Apple System Management Control (SMC) tool 0.01
Usage:
./smctemp [options]
    -h         : help
    -c         : list CPU temperatures
    -l         : list all keys and values
    -v         : version

$ ./smctemp -c
Tp01,44
Tp05,44
Tp09,43
Tp0D,44
Tp0H,44
Tp0L,43
Tp0P,43
Tp0T,44
Tp0X,45
Tp0b,42
```


## Temperature Sensors
### CPU Sensors
- Tp09
- Tp0T
- Tp01
- Tp05
- Tp0D
- Tp0H
- Tp0L
- Tp0P
- Tp0X
- Tp0b

