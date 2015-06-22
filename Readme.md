# UP42 - A Walkera UP02 Quadcopter firmware flasher clone

UP42 is a multi platform (Windows 32/64, Linux 32/64)
command line utility with the following features:
 
* Walkera firmware en-/decryption
* Serial port DTR can be used to force receiver into boot loader mode
* Configurable profiles in simple '.ini' file style config file
* fast

## Command line usage
The UP42 flasher understands these command line arguments:

###-h,--help
Show help/usage informations.

###-V,--version
Displays version information.

###-v,--verbose   
Enables verbose mode. If used multiple times (e.g. -v -v -v or -vvv) different debug informations will be shown.

###-c,--config
Specifies which config file to use. Defaults to *binary*.ini. If binary is named up42, up42.ini is used as default config file name.

###-I,--info
Connects to a Walkera receiver board and displays it's information string. Does not flash firmware.

###-P,--profile
Defines which profile should be used from the config file.

###-q,--quiet
Be quiet. Dont output any messages except data.


###-i,--input
Input file to send. If '-' stdin will be used.

###-o,--output
Output file after encryption with key. If '-' stdout will be used.
If no key specified outputFile == inputFile. If no output file is specified
and a key is present a temporary output file is created which is deleted after
flashing.

###-k,--key
Key to XOR-encrypt inputFile with. Can be either:
* a hex string starting with '0x'
* the name of a file which content is used as key
* if not hex or file the character string itself

If omitted no encryption is performed on input file.

###-p,--port
Name of the serial port to send the encrypted output file via XModem to. If omitted nothing will be sent.

###-b,--baud
Baude rate to use. Defaults to 9600.

###-1,-2
1 or 2 stop bits.

###-7,-8
7 or 8 data bits.

###-N,-E,-O
No, Even or Odd parity.

###-D,--nodtr
Disable DTR control.

##Configuration
A UP42 config file is a simple '.ini' file where each section represents a so called profile.

This is a sample file for two Walkera receiver models. A Hoten-X RX2635H and a QR-Ladybird RX2634H:
```
[RX2635H]
name=RX2635H
description=Walkera Hoten-X
key=0x8E500166526E7BD7EEC85C7AC05AD792
input=MyFirmware.bin
output=MyFirmware.fw
port=COM1
; Walkera normally uses 38400,N,8,1 baud
baud=38400
; 7 or 8 data bits
dataBits=8
; 0=one stopbit, 1=one and a half stop bit, 2=two stopbits
stopBits=0
; 0=no, 1=odd, 2=even, 3=mark, 4=space
parity=0


[RX2634H]
name=RX2634H
description=Walkera QR-LadyBird
key=0xD0943F8C297615D82040E32745D848AD
```
In this sample the profile **RX2635H** defines values for the command line parameters
(in order) -k, -i, -o, -p, -b, -8, -N.
So you can simply call
`up42 -P RX2635H` to:
* encrypt the file *MyFirmware.bin* with the key *0x8E500166526E7BD7EEC85C7AC05AD792*
* write the encrypted content to file *MyFirmware.fw*
* flash the receiver using XMODEM via port COM1, 38400 baud, no parity, 8 databits and 1 stopbit

After flashing the receiver the bootloader mode is ended and the new firmware is started.


##Compiling

###Windows
Building on Windows can be done in two ways:
1. building with TCC (Tiny C compiler)
If you dont have Visual Studio available or dont want install it wasting
some gigabytes of disk space, UP42 can be compiled using TCC from http://bellard.org/tcc/.
Download and install the 32 or 64 bit version and change the *build.cmd* file 
with the path to TCC. You will also need GNU make for Windows from 
http://gnuwin32.sourceforge.net/packages/make.htm 

2. building using Microsoft Visual Studio.
For building UP42 with Visual Studio there is a solution file (.sln) included.
Open it with Visual Studio and compile it either for 32 or 64 bit.

###Linux
To build under Linux you can use the included `build.sh` shell script which uses make and gcc.

 
##Connecting the board / the flash cable
UP42 needs a serial connection to the receiver board. Walkera receivers work with low
voltage (3.3V) wheras a PC serial port uses 12 Volts. So directly connecting the receiver
to the PC is not a good idea. So a level shifter like the [MAX3232 from Maxim](http://www.maximintegrated.com/en/products/interface/transceivers/MAX3232.html#popuppdf) must be used.

Such a cable was already shown at http://www.min.at/prinz/?x=entry:entry140107-181200. UP42 can be used with
this cable but you have to manually enter bootloader mode by reseting (powering on) the receiver
while UP42 waits for the board to become ready (about ~ 10 seconds).

This can be automated by UP42 by using the serial port DTR line to automatically reset the board
during the wait time. Thus making firmware updates completely automatic. 

More infos can be found at: http://www.min.at/prinz/?x=entry:entry150622-125650


