# zwave-flashing-tool
This is a simple flashing tool for 5gen zwave SoCs like the ZM5101A.

```{bash}
matti@rocinante ~/Bastelkram/z-wave/zwave-flashing-tool/build$ ./zft -h 
Usage: ./build/zft -d <device> -f <file> -o <file> -n <file> -m <file> -p <file> -j <file> -e -s -t <timeout> -v <level>
        -d <device>    Serial device
        -f <file>      Input hex file
        -o <file>      Output hex file
        -n <file>      Input NVR file
        -m <file>      Output NVR file
        -p <file>      Preset input NVR file (json)
        -j <file>      Preset output NVR file (json)
        -s             Update NVR with S2 keypair
        -e             Erase flash
        -t <timeout>   Serial receive timeout
        -v <level>     Log level 0..4

```
## Building
Clone this repository and change into the top level directory.
```{bash}
git clone https://github.com/matthiasb85/zwave-flashing-tool.git
cd zwave-flashing-tool
```
Afterwards you are able to build the zft binary with the regular cmake commands.
```{bash}
mkdir build
cd build
cmake ..
make
```
