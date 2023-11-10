# zwave-flashing-tool
This is a simple flashing tool for 5gen zwave SoCs like the ZM5101A.

```{bash}
matti@rocinante ~/Bastelkram/z-wave/zwave-flashing-tool/build$ ./zft -h                                                                                                                                                        255 ↵  ✹main 
Usage: ./zft -d <device> -f <file> -o <file> -n <file> -t <timeout>
        -d <device>    Serial device
        -f <file>      Input hex file
        -o <file>      Output hex file
        -n <file>      Input NVR file
        -m <file>      Output NVR file
        -t <timeout>   Serial receive timeout
        -v <level>     Log level 0..4

```
## Building
Clone this repository and change into the top level directory.
Afterwards you are able to build the zft binary with the regular cmake commands.
```{bash}
mkdir build
cd build
cmake ..
make
```
