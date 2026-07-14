# LcdPico: Command Reference
LcdPico uses "SCPI like" command set. Command syntax should be mostly SCPI and IEE488.2 compliant,
to make it easier to control and configure LcdPico units.

LcdPico is a TFT LCD display module for the HP/Agilent/Keysight 34401A digital multimeter. It
sniffs the multimeter's front-panel display bus and mirrors the reading on an external LCD, while
also exposing this command interface over the USB serial console and (on Pico W boards) Telnet
and SSH.


## Command Set
LcdPico supports following commands:

* [*IDN?](#idn)
* [*RST](#rst)
* [CONFigure?](#configure)
* [CONFigure:SAVe](#configuresave)
* [CONFigure:Read?](#configureread)
* [CONFigure:UPLOAD](#configureupload)
* [CONFigure:DELete](#configuredelete)
* [CONFigure:VSENSORS?](#configurevsensors)
* [CONFigure:VSENSORS:SOUrce?](#configurevsensorssource)
* [CONFigure:VSENSORx:NAME](#configurevsensorxname)
* [CONFigure:VSENSORx:NAME?](#configurevsensorxname-1)
* [CONFigure:VSENSORx:SOUrce](#configurevsensorxsource)
* [CONFigure:VSENSORx:SOUrce?](#configurevsensorxsource-1)
* [DEBUG?](#debug)
* [EXIT](#exit)
* [MEASure:Read?](#measureread)
* [MEASure:Main?](#measuremain)
* [MEASure:ANNunciators?](#measureannunciators)
* [MEASure:VSENSORS?](#measurevsensors)
* [MEASure:VSENSORx?](#measurevsensorx)
* [MEASure:VSENSORx:Read?](#measurevsensorxread)
* [MEASure:VSENSORx:TEMP?](#measurevsensorxtemp)
* [MEASure:VSENSORx:HUMidity?](#measurevsensorxhumidity)
* [MEASure:VSENSORx:PREssure?](#measurevsensorxpressure)
* [Read?](#read)
* [SYStem:BACKLight](#systembacklight)
* [SYStem:BACKLight?](#systembacklight-1)
* [SYStem:BOARD?](#systemboard)
* [SYStem:ECHO](#systemecho)
* [SYStem:ECHO?](#systemecho-1)
* [SYStem:ERRor?](#systemerror)
* [SYStem:FLASH?](#systemflash)
* [SYStem:HTTP:SERVer](#systemhttpserver)
* [SYStem:HTTP:SERVer?](#systemhttpserver-1)
* [SYStem:HTTP:PORT](#systemhttpport)
* [SYStem:HTTP:PORT?](#systemhttpport-1)
* [SYStem:HTTP:TLSPORT](#systemhttptlsport)
* [SYStem:HTTP:TLSPORT?](#systemhttptlsport-1)
* [SYStem:HTTP:MASK:VSENSOR](#systemhttpmaskvsensor)
* [SYStem:HTTP:MASK:VSENSOR?](#systemhttpmaskvsensor-1)
* [SYStem:I2C?](#systemi2c)
* [SYStem:I2C:SCAN?](#systemi2cscan)
* [SYStem:I2C:SPEED](#systemi2cspeed)
* [SYStem:I2C:SPEED?](#systemi2cspeed-1)
* [SYStem:LED](#systemled)
* [SYStem:LED?](#systemled-1)
* [SYStem:LFS?](#systemlfs)
* [SYStem:LFS:COPY](#systemlfscopy)
* [SYStem:LFS:DELete](#systemlfsdelete)
* [SYStem:LFS:DIRectory?](#systemlfsdirectory)
* [SYStem:LFS:FORMAT](#systemlfsformat)
* [SYStem:LFS:REName](#systemlfsrename)
* [SYStem:LOG](#systemlog)
* [SYStem:LOG?](#systemlog-1)
* [SYStem:MEMTEST](#systemmemtest)
* [SYStem:MEMory](#systemmemory)
* [SYStem:MEMory?](#systemmemory-1)
* [SYStem:NAME](#systemname)
* [SYStem:NAME?](#systemname-1)
* [SYStem:SSH:SERVer](#systemsshserver)
* [SYStem:SSH:SERVer?](#systemsshserver-1)
* [SYStem:SSH:ACLs](#systemsshacls)
* [SYStem:SSH:ACLs?](#systemsshacls-1)
* [SYStem:SSH:AUTH](#systemsshauth)
* [SYStem:SSH:AUTH?](#systemsshauth-1)
* [SYStem:SSH:PORT](#systemsshport)
* [SYStem:SSH:PORT?](#systemsshport-1)
* [SYStem:SSH:USER](#systemsshuser)
* [SYStem:SSH:USER?](#systemsshuser-1)
* [SYStem:SSH:PASSword](#systemsshpassword)
* [SYStem:SSH:PASSword?](#systemsshpassword-1)
* [SYStem:SSH:KEY?](#systemsshkey)
* [SYStem:SSH:KEY:CREate](#systemsshkeycreate)
* [SYStem:SSH:KEY:DELete](#systemsshkeydelete)
* [SYStem:SSH:KEY:LIST?](#systemsshkeylist)
* [SYStem:SSH:PUBKEY?](#systemsshpubkey)
* [SYStem:SSH:PUBKEY:ADD](#systemsshpubkeyadd)
* [SYStem:SSH:PUBKEY:DELete](#systemsshpubkeydelete)
* [SYStem:SSH:PUBKEY:LIST?](#systemsshpubkeylist)
* [SYStem:SYSLOG](#systemsyslog)
* [SYStem:SYSLOG?](#systemsyslog-1)
* [SYStem:TELNET:SERVer](#systemtelnetserver)
* [SYStem:TELNET:SERVer?](#systemtelnetserver-1)
* [SYStem:TELNET:ACLs](#systemtelnetacls)
* [SYStem:TELNET:ACLs?](#systemtelnetacls-1)
* [SYStem:TELNET:AUTH](#systemtelnetauth)
* [SYStem:TELNET:AUTH?](#systemtelnetauth-1)
* [SYStem:TELNET:PORT](#systemtelnetport)
* [SYStem:TELNET:PORT?](#systemtelnetport-1)
* [SYStem:TELNET:RAWmode](#systemtelnetrawmode)
* [SYStem:TELNET:RAWmode?](#systemtelnetrawmode-1)
* [SYStem:TELNET:USER](#systemtelnetuser)
* [SYStem:TELNET:USER?](#systemtelnetuser-1)
* [SYStem:TELNET:PASSword](#systemtelnetpassword)
* [SYStem:TELNET:PASSword?](#systemtelnetpassword-1)
* [SYStem:TIME](#systemtime)
* [SYStem:TIME?](#systemtime-1)
* [SYStem:TIMEZONE](#systemtimezone)
* [SYStem:TIMEZONE?](#systemtimezone-1)
* [SYStem:TLS:CERT](#systemtlscert)
* [SYStem:TLS:CERT?](#systemtlscert-1)
* [SYStem:TLS:PKEY](#systemtlspkey)
* [SYStem:TLS:PKEY?](#systemtlspkey-1)
* [SYStem:UPGRADE](#systemupgrade)
* [SYStem:UPTIme?](#systemuptime)
* [SYStem:VERsion?](#systemversion)
* [SYStem:VSENSORS?](#systemvsensors)
* [SYStem:WIFI?](#systemwifi)
* [SYStem:WIFI:AUTHmode](#systemwifiauthmode)
* [SYStem:WIFI:AUTHmode?](#systemwifiauthmode-1)
* [SYStem:WIFI:COUntry](#systemwificountry)
* [SYStem:WIFI:COUntry?](#systemwificountry-1)
* [SYStem:WIFI:DNS](#systemwifidns)
* [SYStem:WIFI:DNS?](#systemwifidns-1)
* [SYStem:WIFI:GATEway](#systemwifigateway)
* [SYStem:WIFI:GATEway?](#systemwifigateway-1)
* [SYStem:WIFI:HOSTname](#systemwifihostname)
* [SYStem:WIFI:HOSTname?](#systemwifihostname-1)
* [SYStem:WIFI:INFO?](#systemwifiinfo)
* [SYStem:WIFI:IPaddress](#systemwifiipaddress)
* [SYStem:WIFI:IPaddress?](#systemwifiipaddress-1)
* [SYStem:WIFI:MAC?](#systemwifimac)
* [SYStem:WIFI:MODE](#systemwifimode)
* [SYStem:WIFI:MODE?](#systemwifimode-1)
* [SYStem:WIFI:NETMask](#systemwifinetmask)
* [SYStem:WIFI:NETMask?](#systemwifinetmask-1)
* [SYStem:WIFI:NTP](#systemwifintp)
* [SYStem:WIFI:NTP?](#systemwifintp-1)
* [SYStem:WIFI:NTPClient](#systemwifintpclient)
* [SYStem:WIFI:NTPClient?](#systemwifintpclient-1)
* [SYStem:WIFI:PASSword](#systemwifipassword)
* [SYStem:WIFI:PASSword?](#systemwifipassword-1)
* [SYStem:WIFI:REJOIN](#systemwifirejoin)
* [SYStem:WIFI:SSID](#systemwifissid)
* [SYStem:WIFI:SSID?](#systemwifissid-1)
* [SYStem:WIFI:STATus?](#systemwifistatus)
* [SYStem:WIFI:STATS?](#systemwifistats)
* [SYStem:WIFI:SYSLOG](#systemwifisyslog)
* [SYStem:WIFI:SYSLOG?](#systemwifisyslog-1)
* [SYStem:WIFI:SYSLOGClient](#systemwifisyslogclient)
* [SYStem:WIFI:SYSLOGClient?](#systemwifisyslogclient-1)
* [WHO?](#who)
* [WRIte:VSENSORx](#writevsensorx)


Additionally unit will respond to following standard SCPI commands to provide compatibility in case some program
unconditionally will send these:
* *CLS
* *ESE
* *ESE?
* *ESR?
* *OPC
* *OPC?
* *SRE
* *SRE?
* *STB?
* *TST?
* *WAI


### Common Commands

#### *IDN?
Identify device. This returns string that contains following fields:
```
<manufacturer>,<model number>,<serial number>,<firmware version>
```

Example:
```
*IDN?
TJKO Industries,LCDPICO-34401A,e660c0d1c768a330,1.0
```

#### *RST
Reset unit. This triggers LcdPico to perform (warm) reboot.

```
*RST
```

### CONFigure Commands
Commands for configuring the device settings.

#### CONFigure?
Display current configuration in JSON format.
Same as CONFigure:Read?

Example:
```
CONF?
```

#### CONFigure:SAVe
Save current configuration into flash memory.

Example:
```
CONF:SAVE
```

#### CONFigure:Read?
Display (backup) current configuration in JSON format.

Example:
```
CONF:READ?
```

#### CONFigure:UPLOAD
Upload (previously backed up) configuration in JSON format.
This command waits up to 10 seconds for the configuration to be uploaded.

End of configuration is signified by empty line (after end of configuration).
(To cancel uploading configuration, two empty lines can be sent.)

Example:
```
CONF:UPLOAD
Paste LcdPico configuration in JSON format:
[Received 1234 bytes]

Clearing config...
Loading config...
[   112.487339][0] Config version: lcdpico-config-v1
Configuration successfully loaded.
```

#### CONFigure:DELete
Delete current configuration saved into flash. After unit has been reset it will be using default configuration.

Example:
```
CONF:DEL
*RST
```

#### CONFigure:VSENSORS?
This is same as CONFigure:VSENSORS:SOUrce? command.

#### CONFigure:VSENSORS:SOUrce?
Return virtual sensor (source) configuration information for all
virtual sensors in CSV format.

Format: <vsensor>,<mode>,<parameter1>,<parameter2>,...

Example:
```
CONF:VSENSORS:SOURCE?
vsensor1,manual,0.00,30
vsensor2,manual,0.00,30
vsensor3,i2c,0x48,TMP117
vsensor4,internal,0.00,1.00000
vsensor5,manual,0.00,30
vsensor6,manual,0.00,30
vsensor7,manual,0.00,30
vsensor8,manual,0.00,30
```

### CONFigure:VSENSORx Commands
VSENSORx (where x is the sensor number) commands are used to configure virtual temperature sensors.
There are no physical (thermistor) sensor inputs on this board; all sensors are "virtual" and are
either updated by software (over the command interface), or read from a digital I2C sensor attached
to the board.

Where x is a number from 1 to 8.

Virtual Sensor Mode|Description
------|-----------
manual|Manually updated (by software, via WRIte:VSENSORx) sensor value.
i2c|Reading from digital I2C temperature/humidity/pressure sensor.
internal|Accepted by this command for future use, but not currently read by the firmware (temperature value will not update).

#### CONFigure:VSENSORx:NAME
Set name for virtual temperature sensor.

For example:
```
CONF:VSENSOR1:NAME Ambient
```

#### CONFigure:VSENSORx:NAME?
Query name of a virtual temperature sensor.

For example:
```
CONF:VSENSOR1:NAME?
Ambient
```

#### CONFigure:VSENSORx:SOUrce
Configure source for the virtual temperature sensor.

Source types:

MODE|Description|Parameters
----|-----------|----------
MANUAL|Temperature updated externally (via WRIte:VSENSORx)|default_temperature_C,timeout_s
I2C|Temperature (and possibly humidity/pressure) reading from digital I2C sensor|i2c_address,sensor_type_or_alias
INTERNAL|Reserved for a future onboard temperature reading|temp_offset,temp_coefficient

Note, in "manual" mode if timeout (seconds) is set to zero, then sensor's temperature reading
will never revert back to default value (if no updates are being received). Otherwise, if no
update has been received (via WRIte:VSENSORx) within the timeout, the reading reverts to
default_temperature_C.

Supported I2C sensors:

Sensor Type|Aliases|Possible Addresses|Description|Notes
-----------|-------|------------------|-----------|-----
ADT7410||0x48, 0x49, 0x4a, 0x4b|16bit Digital Temperature Sensor, 0.5C accuracy
AHT1x|||AHT1x Series Temperature and Humidity sensors (AHT10, AHT11 ,...)
AHT2x|||AHT2x Series Temperature and Humidity sensors (AHT20, AHT21 ,...)
AM2320||0x5c|Temperature and Humidity Sensor, 0.5C accuracy|Not found when scanning bus. May not work above 100kHz bus speeds.
AS621x|AS6212, AS6214, AS6218||AS621x Series sensors: AS6212 (0.2C), AS6214 (0.4C), AC6218 (0.8C)
BMP180|||16bit, 0.5C accuracy
BMP280||0x76, 0x77|20bit, 0.5C accuracy
DPS310||0x77, 0x76|24bit, 0.5C accuracy
HDC302x|HDC3022, HDC3021, HDC3020|0x44, 0x45, 0x46, 0x47|HDC302x Series Temperature and Humidity sensors
HTS221||0x5f|Temperature and Humidity Sensor
HTU21D||0x40|Temperature and Humidity Sensor, 0.4C accuracy|Not found when scanning bus (SYS:I2C:SCAN?)
HTU31D||0x40, 0x41|Temperature and Humidity Sensor, 0.3C accuracy
LPSxx|LPS22, LPS25, LPS28, LPS33, LPS35|0x5d, 0x5c|LPSxx Series Temperature and Pressure sensors
MCP9808|||Digital Temperature Sensor, 0.5C accuracy
MPL115A2||0x60|Digital Barometer|Temperature sensor not calibrated.
MPL3115A2||0x60|Temperature and Pressure sensor with Altimetry, 1C accuracy
MS5611||0x76, 0x77|Temperature and Pressure Sensor|Not found when scanning bus (SYS:I2C:SCAN?)
MS8607||0x76 and 0x40|Temperature, Humidity and Pressure Sensor|Not found when scanning bus (SYS:I2C:SCAN?), appears as two separate devices.
PCT2075|||Digital Temperature Sensor, 1C accuracy
SHT3x|SHT30, SHT31, SHT35|0x44, 0x34|SHT3x Series Temperature and Humidity sensors|Not always found when scanning bus (SYS:I2C:SCAN?)
SHT4x|SHT40, SHT41, SHT43, SHT45|0x44|SHT4x Series Temperature and Humidity sensors|Not always found when scanning bus (SYS:I2C:SCAN?)
SHTC3||0x70|Temperature and Humidity sensor, 0.2C accuracy
SI7021||0x40|Temperature and Humidity sensor, 0.4C accuracy
STTS22H||0x38, 0x3c, 0x3e, 0x3f|Temperature Sensor, 0.5C accuracy
TC74|TC74A0|0x48 - 0x4f|Digital Thermal Sensor, 2C accuracy
TMP102||0x48, 0x49, 0x4a, 0x4b|Temperature Sensor, 2C accuracy
TMP117||0x48, 0x49, 0x4a, 0x4b|Temperature Sensor, 0.1C accuracy

Sensor numbering used in this command's parameters:
 - VSENSORS: 1, 2, ...

Defaults:

Default for all virtual sensors is to be in "MANUAL" mode and revert automatically to 0C
if no temperature update has been received within 30 seconds.

VSENSOR|SOURCE
---|------
1|MANUAL,0,30
...|...

Example: Set VSENSOR2 to report temperature that is updated by external program, reverting
to a default value of 99C if no update has been received within 5 seconds.
```
CONF:VSENSOR2:SOURCE manual,99,5
```

Example: Set VSENSOR3 to report temperature from TMP117 (I2C) sensor with address 0x48:
```
CONF:VSENSOR3:SOURCE i2c,0x48,tmp117
```

(to get list of currently active I2C sensor addresses, use: SYS:I2C:SCAN?)


#### CONFigure:VSENSORx:SOUrce?
Query a virtual temperature sensor configuration (temperature reading source).

Command returns response in following format:
```
mode,parameter,parameter,...
```

Example:
```
CONF:VSENSOR2:SOU?
manual,99.0,5
```

#### DEBUG?
Display internal debug counters for the 34401A front-panel bus decoder (SCK/interrupt timing,
byte/message error counts, and touch/LCD interrupt counts). Intended for diagnosing decoding
issues.

Example:
```
DEBUG?
DMM Decoder:
            sck gap: 42 us
        max sck gap: 118 us
           main gap: 620 us
       max main gap: 1450 us
            any gap: 42 us
        max any gap: 1450 us
         fifo level: 2
     max fifo level: 18
 byte overrun count: 0
 mid byte gap count: 0
 buf overflow count: 0
      bad msg count: 0
  last mid byte gap: 0
       last bad msg: 0
         last reset: 1234567
           last int: 4567890
          last main: 4567000
           last any: 4567890

Interrupts:
            DMM_INT: 128442
            DMM_SCK: 512044
          DMM_RESET: 812
            LCM_INT: 30211
            CTP_INT: 0

```

#### EXIT
Disconnect remote connection. This will disconnect active telnet or SSH connection.

```
EXIT
```


### MEASure Commands
These commands are for reading the current 34401A display content and virtual sensor readings.

#### MEASure:Read?
Return the last valid (numeric) reading captured from the multimeter's front-panel display.
(This is same as: Read?)

Example:
```
MEAS:READ?
+1.234567E+00
```

#### MEASure:Main?
Return the current raw content of the multimeter's front-panel display. Unlike MEASure:Read?,
this reflects whatever is currently shown (including transient text messages), not only
confirmed numeric readings.

Example:
```
MEAS:MAIN?
+1.234567E+00
```

#### MEASure:ANNunciators?
Return comma separated list of currently active annunciators (status indicators) shown on the
multimeter's display.

Possible annunciators: *, Adrs, Rmt, Man, Trig, Hold, Mem, Ratio, Math, ERROR, Rear, Shift, Diode,
Continuity, 4-Wire

Example:
```
MEAS:ANN?
Rmt,Trig,4-Wire
```

#### MEASure:VSENSORS?
Return all measurements for all virtual sensors.

Format: vsensor,"name",temperature_C,humidity_%,pressure_hPa

Example:
```
MEAS:VSENSORS?
vsensor1,"Ambient",24.4,0.0,0
vsensor2,"vsensor2",0.0,0.0,0
vsensor3,"vsensor3",24.8,43.0,997
vsensor4,"vsensor4",0.0,0.0,0
vsensor5,"vsensor5",0.0,0.0,0
vsensor6,"vsensor6",0.0,0.0,0
vsensor7,"vsensor7",0.0,0.0,0
vsensor8,"vsensor8",0.0,0.0,0
```

#### MEASure:VSENSORx?
Return current temperature (C) measured by the virtual sensor.

This is same as: MEASure:VSENSORx:Read? / MEASure:VSENSORx:TEMP?

Example:
```
MEAS:VSENSOR1?
24.4
```

#### MEASure:VSENSORx:Read?
Return current temperature (C) measured by the virtual sensor.

This is same as: MEASure:VSENSORx?

Example:
```
MEAS:VSENSOR1:R?
24.4
```

#### MEASure:VSENSORx:TEMP?
Return current temperature (C) measured by the virtual sensor.

This is same as: MEASure:VSENSORx?

Example:
```
MEAS:VSENSOR1:TEMP?
24.4
```

#### MEASure:VSENSORx:HUMidity?
Return current humidity (%) measured by the virtual sensor (only applicable to I2C sensors
that support humidity measurements).

Example:
```
MEAS:VSENSOR3:HUM?
43.0
```

#### MEASure:VSENSORx:PREssure?
Return current pressure (hPa) measured by the virtual sensor (only applicable to I2C sensors
that support pressure measurements).

Example:
```
MEAS:VSENSOR3:PRE?
997
```


### Read Commands

#### Read?
Return the last valid (numeric) reading captured from the multimeter's front-panel display.
(This is same as: MEASure:Read?)

Example:
```
Read?
+1.234567E+00
```


### SYStem Commands

#### SYStem:BACKLight
Set LCD display backlight brightness (%).

Range: 0 - 100

Example:
```
SYS:BACKL 80
```

#### SYStem:BACKLight?
Query current LCD display backlight brightness (%).

Example:
```
SYS:BACKL?
80
```

#### SYStem:BOARD?
Display information about hardware model and the Pico module attached.

Example:
```
SYS:BOARD?
Hardware Model: LCD-PICO-34401A
         Board: Pico W
           MCU: RP2040-B2 @ 200MHz
           RAM: 264KB
         Flash: 2048KB @ 100MHz
 Serial Number: e6614c311b7ca431
```

#### SYStem:ECHO
Enable or disable local echo on the console.
This can be useful if interactively programming LcdPico.

Value|Status
-----|------
0|Local Echo disabled.
1|Local Echo enabled.

Default: 0

Example: enable local echo
```
SYS:ECHO 1
```

Example: disable local echo
```
SYS:ECHO 0
```

#### SYStem:ECHO?
Display local echo status:

Example:
```
SYS:ECHO?
0
```

#### SYStem:ERRor?
Display status from last command.

Example:
```
SYS:ERR?
0,"No Error"
```

#### SYStem:FLASH?
Returns information about Pico flash memory usage.

Example:
```
SYS:FLASH?
Flash memory size:                     2097152
Binary size:                           683520
LittleFS size:                         262144
Unused flash memory:                   1151488
```

#### SYStem:HTTP:SERVer
Enable or disable built-in HTTP server that displays status information
about the unit.

Default: ON

Example (disable HTTP server):
```
SYS:HTTP:SERVER OFF
```

#### SYStem:HTTP:SERVer?
Query current status of HTTP server.

Example:
```
SYS:HTTP:SERVER?
ON
```

#### SYStem:HTTP:PORT
Set web server HTTP port.

Default: 0  (use default HTTP port tcp/80)

Example:
```
SYS:HTTP:PORT 8080
```

#### SYStem:HTTP:PORT?
Query currently configured HTTP port.

Example:
```
SYS:HTTP:PORT?
0
```

#### SYStem:HTTP:TLSPORT
Set web server HTTPS port.

Default: 0  (use default HTTPS port tcp/443)

Example:
```
SYS:HTTP:TLSPORT 8443
```

#### SYStem:HTTP:TLSPORT?
Query currently configured HTTPS port.

Example:
```
SYS:HTTP:TLSPORT?
0
```

#### SYStem:HTTP:MASK:VSENSOR
Configure which virtual sensors should be displayed on HTTP server.

Sensors can be specified as comma separated list (1,3,7) or as range (1-3)
or as combination of both.

Default: 1-8    (display all virtual sensors)

Example:
```
SYS:HTTP:MASK:VSENSOR 1-4
```

#### SYStem:HTTP:MASK:VSENSOR?
Query which virtual sensors are configured to be displayed on HTTP server.

Example:
```
SYS:HTTP:MASK:VSENSOR?
1-4
```

#### SYStem:I2C?
Returns status if I2C bus is active (available) currently.

Example:
```
SYS:I2C?
ON
```

#### SYStem:I2C:SCAN?
Scan I2C Bus for active devices.
This returns addresses of any devices found on I2C bus.

Example:
```
SYS:I2C:SCAN?
Scanning I2C Bus... 0x48, 0x76
Device(s) found: 2
```

#### SYStem:I2C:SPEED
Set speed that I2C bus operates at.
Note, change won't take effect until unit is rebooted.

Speed range: 10000 - 3400000
(speeds over 1000000 may not work reliably)

Default: 1000000  (1000 kHz or 1000 kbit/s)

Example:
```
SYS:I2C:SPEED 1000000
CONF:SAVE
```

#### SYStem:I2C:SPEED?
Return currently configured I2C bus speed (Hz or bit/s).

Example:
```
SYS:I2C:SPEED?
1000000
```

#### SYStem:LED
Set system indicator LED operating mode.

Range: 0 - 2

Default: 0

Example:
```
SYS:LED 1
```

#### SYStem:LED?
Query current system indicator LED operating mode.

Example:
```
SYS:LED?
0
```

#### SYStem:LFS?
Display information about the LittleFS filesystem in the flash memory.

Example:
```
SYS:LFS?
Filesystem size:                       262144
Filesystem used:                       24576
Filesystem free:                       237568
Number of files:                       3
Number of subdirectories:              0
```

#### SYStem:LFS:COPY
Copy a file on the flash filesystem (littlefs).
If destination file already exists copy will fail.

Parameters: <sourcefile> <destinationfile>

Example:
```
SYS:LFS:COPY lcdpico.cfg lcdpico-backup.cfg
```

#### SYStem:LFS:DELete
Delete file from the flash filesystem (littlefs).

Parameters: <filename>

Example (delete TLS certificate and private key):
```
SYS:LFS:DEL cert.pem
SYS:LFS:DEL key.pem
```

#### SYStem:LFS:DIRectory?
List contents of the flash filesystem (littlefs).

Example:
```
SYS:LFS:DIR?
Directory: /

.                                                       <DIR>
..                                                      <DIR>
cert.pem                                                 1286
lcdpico.cfg                                              4012
key.pem                                                  1709

```

#### SYStem:LFS:FORMAT
Format flash filesystem. This will erase current configuration (including any TLS certificates saved in flash).

Example (format filesystem and save current configuration):
```
SYS:LFS:FORMAT
CONF:SAVE
```

#### SYStem:LFS:REName
Rename a file on the flash filesystem (littlefs).

Parameters: <oldname> <newname>

Example:
```
SYS:LFS:REN lcdpico-backup.cfg lcdpico.cfg
```

#### SYStem:LOG
Set the system logging level. This controls the level of logging to the console.

Default: WARNING

Log Levels:

Level|Name
-----|----
0|EMERG
1|ALERT
2|CRIT
3|ERR
4|WARNING
5|NOTICE
6|INFO
7|DEBUG

Example: Enable verbose debug output
```
SYS:LOG DEBUG
```

#### SYStem:LOG?
Display current system logging level.

Example:
```
SYS:LOG?
NOTICE
```

#### SYStem:MEMTEST
Run a (destructive) RAM memory test. This overwrites all currently available heap memory
with test patterns to verify the RP2040/RP2350 RAM is functioning correctly.

Example:
```
SYS:MEMTEST
```

#### SYStem:MEMory
Test how much available (heap) memory system currently has.
This does a simple test to try to determine what is the largest
block of heap memory that is currently available, as well as
try allocating as many small blocks of memory as possible to determine
roughly the total available heap memory.

This command takes an optional 'blocksize' parameter (512 - 8192) to specify
the memory block size to use in the test. Default is 1024 bytes.

Example:
```
SYS:MEM 512
Largest available memory block:        114688 bytes
Total available memory:                111104 bytes
```

#### SYStem:MEMory?
Returns information about heap and stack size, as well as information
about current (heap) memory usage as returned by _mallinfo()_ system call.

Example:
```
SYS:MEM?
Core0 stack size:                      8192
Core1 stack size:                      4096
Heap size:                             136604
mallinfo:
Total non-mmapped bytes (arena):       136604
# of free chunks (ordblks):            2
# of free fastbin blocks (smblks):     0
# of mapped regions (hblks):           0
Bytes in mapped regions (hblkhd):      0
Max. total allocated space (usmblks):  0
Free bytes held in fastbins (fsmblks): 0
Total allocated space (uordblks):      21044
Total free space (fordblks):           115560
Topmost releasable block (keepcost):   114808
```

#### SYStem:NAME
Set system name.

Example:
```
SYS:NAME lcdpico1
```

#### SYStem:NAME?
Query current system name.

Example:
```
SYS:NAME?
lcdpico1
```

### SSH Server Commands
(Available on Pico W boards, when firmware is built with SSH server support.)

#### SYStem:SSH:SERVer
Control whether SSH server is enabled or not.
After making change configuration needs to be saved and unit reset.

Default: OFF

Example:
```
SYS:SSH:SERV ON
```

#### SYStem:SSH:SERVer?
Query current SSH server status.

Example:
```
SYS:SSH:SERV?
ON
```

#### SYStem:SSH:ACLs
Configure Access Control List for the SSH server. This can be used to restrict which
hosts/networks are allowed to connect to the SSH server.

Parameters: <ip_or_network>/<prefix>,...  (up to 4 entries)

Example:
```
SYS:SSH:ACLS 192.168.1.0/24
```

#### SYStem:SSH:ACLs?
Display currently configured SSH server ACLs.

Example:
```
SYS:SSH:ACLS?
192.168.1.0/24
```

#### SYStem:SSH:AUTH
Control whether SSH server requires authentication (username/password or public key).

Default: ON

Example:
```
SYS:SSH:AUTH ON
```

#### SYStem:SSH:AUTH?
Query current SSH server authentication setting.

Example:
```
SYS:SSH:AUTH?
ON
```

#### SYStem:SSH:PORT
Set SSH server (TCP) port.

Default: 0   (use default SSH port tcp/22)

Example:
```
SYS:SSH:PORT 2222
```

#### SYStem:SSH:PORT?
Query currently configured SSH server port.

Example:
```
SYS:SSH:PORT?
0
```

#### SYStem:SSH:USER
Set SSH username.

Example:
```
SYS:SSH:USER admin
```

#### SYStem:SSH:USER?
Query currently configured SSH username.

Example:
```
SYS:SSH:USER?
admin
```

#### SYStem:SSH:PASSword
Set SSH user password.

Example:
```
SYS:SSH:PASS mypassword
```

#### SYStem:SSH:PASSword?
Display currently configured SSH user password hash.

When no password is set, password authentication is disabled.

Example:
```
SYS:SSH:PASS?
$6$QvD5AkWSuydeH/EB$UsYA0cymsCRSse78fN4bMb5q0hM5B7YUNSFd3zJfMDbTG7DOH8iuMufVjsvqBOxR9YCJYSHno4CFeOhLtTGLx.
```

#### SYStem:SSH:KEY?
Display list of server private keys. Currently there is no support to
extract/export private keys. This command only displays SHA256 checksums for
each key.

Example:
```
SYS:SSH:KEY?
ecdsa                     121 SHA256:bDd9a2FrWRCyGxwgNZzzejEh+0ivXkww6nbM+4cIfSg
ed25519                    82 SHA256:/si832zNTkrxn0MHh8dhwfa6KlDAPcTRHsoqZbeVaSM
```

#### SYStem:SSH:KEY:CREate
Create SSH Server Private Key. Generated key is saved on the filesystem (littlefs) and
is used by the SSH server as its server private key.

Command takes key type as parameter.

Currently supported key types:

 - ECDSA
 - ED25519
 - ALL (create all supported keys)

Example:
```
SYS:SSH:KEY:CREATE ED25519
Generating ed25519 private key...
OK
```

#### SYStem:SSH:KEY:DELete
Delete SSH Server Private key. Key is deleted from the internal filesystem (littlefs).
When rotating server keys, old key must be first removed, before new one can be created.

Command takes key type as parameter.

Example:
```
SYS:SSH:KEY:DEL ECDSA
Deleting ecdsa private key...
Private key deleted.
```

#### SYStem:SSH:KEY:LIST?
Display list of server private keys.
This is same as command SYS:SSH:KEY?

#### SYStem:SSH:PUBKEY?
Display list of user name and public key pairs that can be used to login
using public key authentication.

Example:
```
SYS:SSH:PUBKEY?
1: admin ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOOWu9+yEevReQUXEDgEMAIrs1DERTwZeSRRhIqioC6x admin@ws1
```

#### SYStem:SSH:PUBKEY:ADD
Add SSH public key for a user to authenticate with.
Command takes two parameters "username", and "public key" (in OpenSSH format).
Currently up to 4 public keys can be added.

Parameters: <username> <ssh publickey>

Example:
```
SYS:SSH:PUBKEY ADD admin ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOOWu9+yEevReQUXEDgEMAIrs1DERTwZeSRRhIqioC6x admin@ws1
SSH Public key added: slot 1: ssh-ed25519 (admin)
```

#### SYStem:SSH:PUBKEY:DELete
Delete SSH Public key.

Parameters: <key number>

(Command SYS:SSH:PUBKEY? can be used to find out key numbers.)

Example:
```
SYS:SSH:PUBKEY:DEL 1
SSH Public key deleted: slot 1: admin:ssh-ed25519 (admin@ws1)
```

#### SYStem:SSH:PUBKEY:LIST?
Display list of user name and public key pairs.
This is same as SYS:SSH:PUBKEY?

#### SYStem:SYSLOG
Set the syslog logging level. This controls the level of logging sent to a remote
syslog server.

Default: ERR

Log Levels:

Level|Name
-----|----
0|EMERG
1|ALERT
2|CRIT
3|ERR
4|WARNING
5|NOTICE
6|INFO
7|DEBUG

Example: Enable logging of NOTICE (and lower level) messages:
```
SYS:SYSLOG NOTICE
```

#### SYStem:SYSLOG?
Display current syslog logging level.

Example:
```
SYS:SYSLOG?
ERR
```

### Telnet Server Commands
(Available on Pico W boards, when firmware is built with Telnet server support.)

#### SYStem:TELNET:SERVer
Control whether Telnet server is enabled or not.
After making change configuration needs to be saved and unit reset.

Default: OFF

Example:
```
SYS:TELNET:SERV ON
```

#### SYStem:TELNET:SERVer?
Query current Telnet server status.

Example:
```
SYS:TELNET:SERV?
ON
```

#### SYStem:TELNET:ACLs
Configure Access Control List for the Telnet server.

Parameters: <ip_or_network>/<prefix>,...  (up to 4 entries)

Example:
```
SYS:TELNET:ACLS 192.168.1.0/24
```

#### SYStem:TELNET:ACLs?
Display currently configured Telnet server ACLs.

Example:
```
SYS:TELNET:ACLS?
192.168.1.0/24
```

#### SYStem:TELNET:AUTH
Control whether Telnet server requires authentication (username/password).

Default: ON

Example:
```
SYS:TELNET:AUTH ON
```

#### SYStem:TELNET:AUTH?
Query current Telnet server authentication setting.

Example:
```
SYS:TELNET:AUTH?
ON
```

#### SYStem:TELNET:PORT
Set Telnet server (TCP) port.

Default: 0   (use default Telnet port tcp/23)

Example:
```
SYS:TELNET:PORT 2323
```

#### SYStem:TELNET:PORT?
Query currently configured Telnet server port.

Example:
```
SYS:TELNET:PORT?
0
```

#### SYStem:TELNET:RAWmode
Control whether Telnet server operates in raw mode (no telnet protocol negotiation).

Default: OFF

Example:
```
SYS:TELNET:RAW ON
```

#### SYStem:TELNET:RAWmode?
Query current Telnet server raw mode setting.

Example:
```
SYS:TELNET:RAW?
OFF
```

#### SYStem:TELNET:USER
Set Telnet username.

Example:
```
SYS:TELNET:USER admin
```

#### SYStem:TELNET:USER?
Query currently configured Telnet username.

Example:
```
SYS:TELNET:USER?
admin
```

#### SYStem:TELNET:PASSword
Set Telnet user password.

Example:
```
SYS:TELNET:PASS mypassword
```

#### SYStem:TELNET:PASSword?
Display currently configured telnet user password hash.

Example:
```
SYS:TELNET:PASS?
$6$QvD5AkWSuydeH/EB$UsYA0cymsCRSse78fN4bMb5q0hM5B7YUNSFd3zJfMDbTG7DOH8iuMufVjsvqBOxR9YCJYSHno4CFeOhLtTGLx.
```

#### SYStem:TIME
Set system Real-Time Clock (RTC) time.

This command expects time in following format:
  YYYY-MM-DD HH:MM:SS

Example:
```
SYS:TIME 2026-07-13 18:55:42
```

#### SYStem:TIME?
Return current Real-Time Clock (RTC) time.
This is only available if using Pico W and it has successfully
gotten time from a NTP server, or the RTC has been initialized using
SYStem:TIME command.

Command returns nothing if RTC has not been initialized.

Example:
```
SYS:TIME?
2026-07-13 18:55:42
```

#### SYStem:TIMEZONE
Set POSIX timezone to use when getting time from a NTP server.
If DHCP server does not supply POSIX Timezone (DHCP Option 100), then this
command can be used to specify local timezone.

This command takes a POSIX timezone string as argument (or, if argument is blank,
then it clears the existing timezone setting).

Example (set Pacific time as local timezone):
```
SYS:TIMEZONE PST8PDT7,M3.2.0/2,M11.1.0/02:00:00
```

Example (clear timezone setting):
```
SYS:TIMEZONE
```

#### SYStem:TIMEZONE?
Return current POSIX timezone setting.

Command returns nothing if no timezone has been set.

Example:
```
SYS:TIMEZONE?
PST8PDT7,M3.2.0/2,M11.1.0/02:00:00
```

### TLS Commands
(Available when firmware is built with TLS support, used for the HTTPS server.)

#### SYStem:TLS:CERT
Upload or delete TLS certificate for the HTTP server.
Note, both certificate and private key must be installed before HTTPS server will
activate (when system is restarted next time).

When run without arguments this will prompt to paste TLS (X.509) certificate
in PEM format. When run with "DELETE" argument currently installed certificate
will be deleted.

Example (upload/paste certificate):
```
SYS:TLS:CERT
Paste certificate in PEM format:

```

Example (delete existing certificate from flash memory):
```
SYS:TLS:CERT DELETE
```

#### SYStem:TLS:CERT?
Display currently installed certificate.

Example:
```
SYS:TLS:CERT?
```

#### SYStem:TLS:PKEY
Upload or delete (TLS Certificate) Private key for the HTTP server.
Note, both certificate and private key must be installed before HTTPS server will
activate (when system is restarted next time).

When run without arguments this will prompt to paste private key
in PEM format. When run with "DELETE" argument currently installed private key
will be deleted.

Example (upload/paste private key):
```
SYS:TLS:PKEY
Paste private key in PEM format:

```

Example (delete existing private key from flash memory):
```
SYS:TLS:PKEY DELETE
```

#### SYStem:TLS:PKEY?
Display currently installed private key.

Example:
```
SYS:TLS:PKEY?
```

#### SYStem:UPGRADE
Reboot unit to USB (BOOTSEL) mode for firmware upgrade.
This command triggers LcdPico to reboot and enter USB "mode", where
new firmware can simply be copied to the USB drive that appears.
After the file has been copied, LcdPico will automatically reboot to the new
firmware image.

Example:
```
SYS:UPGRADE
```

#### SYStem:UPTIme?
Return time elapsed since unit was last rebooted (soft reset).
Command also returns total time unit has been on since last cold reset
and number of times unit has been soft reset since.

Example:
```
SYS:UPTIME?
10:58:40 up 0 days, 02:32
since cold boot 1 days, 11:39 (soft reset count: 5)
```

#### SYStem:VERsion?
Display software version and copyright information.

Example:
```
SYS:VER?
Lcdpico-34401A v1.0 (Jul 13 2026; Release; SDK v2.1.0; pico_w)

<credits and license information>

littlefs: 2.9
cJSON: 1.7.18
libb64: 2.0
wolfSSH: 1.4.18
wolfSSL: 5.7.4
```

#### SYStem:VSENSORS?
Display number of virtual (temperature) sensors available.

Example:
```
SYS:VSENSORS?
8
```

### WiFi Commands
(Available on Pico W boards.)

#### SYStem:WIFI?
Check if the unit supports WiFi networking.
This should be used to determine if any other "WIFI"
commands will be available.

Return values:

0 = No WiFi support (Pico).
1 = WiFi supported (Pico W).

Example:
```
SYS:WIFI?
1
```

#### SYStem:WIFI:AUTHmode
Set Wi-Fi Authentication mode.

Following modes are currently supported:

Mode|Description|Notes
----|-----------|-----
default|Use system default|Currently default is WPA2
WPA3_WPA2|Use WPA3/WPA2 (mixed) mode|
WPA3|Use WPA3 only|
WPA2|Use WPA2 only|
WPA2_WPA|Use WPA2/WPA (mixed) mode|Not recommended.
WPA|Use WPA only|Not recommended.
OPEN|Use "Open" mode|No authentication.

Example:
```
SYS:WIFI:AUTH WPA3_WPA2
```

#### SYStem:WIFI:AUTHmode?
Return currently configured Authentication mode for the WiFi interface.

Example:
```
SYS:WIFI:AUTH?
default
```

#### SYStem:WIFI:COUntry
Set Wi-Fi Country code. By default, the country setting for the wireless adapter is unset.
This means the driver will use a default world-wide safe setting, which can mean that some
channels are unavailable.

Country codes are two letter (ISO 3166) codes. For example, Finland = FI, Great Britain = GB,
United States of America = US, ...

Example:
```
SYS:WIFI:COUNTRY US
```

#### SYStem:WIFI:COUntry?
Return currently configured country code for the Wi-Fi interface.

Example:
```
SYS:WIFI:COUNTRY?
US
```

#### SYStem:WIFI:DNS
Configure (static) DNS Server IP addresses.

Note, set to "0.0.0.0" to use servers provided by DHCP.

Example (configure two DNS servers):
```
SYS:WIFI:DNS 8.8.8.8,8.8.4.4
```

#### SYStem:WIFI:DNS?
Display currently configured (static) DNS Server IP addresses.

If DHCP is active, it may configure DNS servers. To see currently
active DNS servers see SYS:WIFI:INFO?

Example:
```
SYS:WIFI:DNS?
8.8.8.8, 8.8.4.4
```

#### SYStem:WIFI:GATEway
Set statically configured default gateway (router).

Example:
```
SYS:WIFI:GATEWAY 192.168.1.1
```

#### SYStem:WIFI:GATEway?
Display currently configured default gateway (router).

Example:
```
SYS:WIFI:GATEWAY?
192.168.1.1
```

#### SYStem:WIFI:HOSTname
Set system hostname. This will be used with DHCP and Syslog.
If hostname is not defined then system will default to generating
hostname as follows:
  LcdPico-xxxxxxxxxxxxxxxx

(where "xxxxxxxxxxxxxxxx" is the LcdPico serial number)

Example:
```
SYS:WIFI:HOSTNAME lcdpico1
```

#### SYStem:WIFI:HOSTname?
Return currently configured system hostname.

Example:
```
SYS:WIFI:HOSTNAME?
lcdpico1
```

#### SYStem:WIFI:INFO?
Display information about the current WiFi network state.

Example:
```
SYS:WIFI:INFO?
 Network Link: Up
  WiFi Status: Link Up (2 days, 00:11:42 since last change)
  MAC Address: 28:cd:c1:xx:xx:xx
  DHCP Client: Enabled
DHCP Hostname: LcdPico-e6614c31xxxxxxxxx

   IP Address: 192.168.1.170
      Netmask: 255.255.255.0
      Gateway: 192.168.1.1

  DNS Servers: 8.8.8.8, 8.8.4.4
  NTP Servers: 192.168.1.10
```

#### SYStem:WIFI:IPaddress
Set statically configured IP address.

Set address to "0.0.0.0" to enable DHCP.

Example:
```
SYS:WIFI:IP 192.168.1.42
```

#### SYStem:WIFI:IPaddress?
Display currently configured (static) IP address.
If no static address is configured, DHCP will be used.

Example:
```
SYS:WIFI:IP?
0.0.0.0
```

#### SYStem:WIFI:MAC?
Display WiFi adapter MAC (Ethernet) address.

Example:
```
SYS:WIFI:MAC?
28:cd:c1:01:02:03
```

#### SYStem:WIFI:MODE
Set WiFi connection mode. Normally this setting is not needed with modern APs.

However, if LcdPico is failing to connect to a WiFi network, this could be
due to old firmware on the AP (upgrading to latest firmware typically helps).
If firmware update did not help or there is no updated firmware available, setting
connection mode to synchronous can help (however this could cause LcdPico to "hang"
for up to 60 seconds during boot up).

Mode|Description
------|-----------
0|Asynchronous connection mode (default)
1|Synchronous connection mode

Default: 0

Example:
```
SYS:WIFI:MODE 1
```

#### SYStem:WIFI:MODE?
Display currently configured WiFi connection mode.

Example:
```
SYS:WIFI:MODE?
0
```

#### SYStem:WIFI:NETMask
Set statically configured netmask.

Example:
```
SYS:WIFI:NETMASK 255.255.255.0
```

#### SYStem:WIFI:NETMask?
Display currently configured (static) netmask.

Example:
```
SYS:WIFI:NETMASK?
255.255.255.0
```

#### SYStem:WIFI:NTP
Configure IP address(es) for NTP server(s) to use (up to 2).

Set to "0.0.0.0" to use server provided by DHCP.

Example:
```
SYS:WIFI:NTP 192.168.1.10,192.168.1.11
```

#### SYStem:WIFI:NTP?
Display currently configured NTP server(s).

Note, "0.0.0.0" means to use DHCP.

Example:
```
SYS:WIFI:NTP?
192.168.1.10
```

#### SYStem:WIFI:NTPClient
Enable or disable NTP client that synchronizes clock to network time server time.

Default: ON

Example (disable NTP client):
```
SYS:WIFI:NTPC OFF
```

#### SYStem:WIFI:NTPClient?
Query current status of NTP client.

Example:
```
SYS:WIFI:NTPC?
ON
```

#### SYStem:WIFI:PASSword
Set Wi-Fi (PSK) password/passphrase.

Example:
```
SYS:WIFI:PASS mynetworkpassword
```

#### SYStem:WIFI:PASSword?
Display currently configured Wi-Fi (PSK) password/passphrase.

Example:
```
SYS:WIFI:PASS?
mynetworkpassword
```

#### SYStem:WIFI:REJOIN
Trigger attempt to rejoin the WiFi network. If system is currently joined to a network,
this causes system to leave the current network first.

System will automatically attempt to rejoin the WiFi network if connection fails,
so this command should normally not be needed.

Example:
```
SYS:WIFI:REJOIN
```

#### SYStem:WIFI:SSID
Set Wi-Fi network SSID. LcdPico will automatically try joining this network.

Example:
```
SYS:WIFI:SSID mynetwork
```

#### SYStem:WIFI:SSID?
Display currently configured Wi-Fi network SSID.

Example:
```
SYS:WIFI:SSID?
mynetwork
```

#### SYStem:WIFI:STATus?
Display WiFi Link status.

Return value: link_status,current_ip,current_netmask,current_gateway,link_status_code

Link Status:

Link Status|Description
-----|-----------
Link Down|Not connected to a WiFi network.
Joining|Connecting to WiFi.
No IP|Connected to WiFi, but no IP address.
Link Up|Connected to WiFi with an IP address.
Link Fail|Connection failed.
Network Fail|No matching SSID found (could be out of range, or down).
Auth Fail|Authentication failed (wrong password)

Example:
```
SYS:WIFI:STAT?
Link Up,192.168.1.42,255.255.255.0,192.168.1.1,3
```

#### SYStem:WIFI:STATS?
Display TCP/IP stack (LwIP) statistics.

Example:
```
SYS:WIFI:STATS?
```

#### SYStem:WIFI:SYSLOG
Configure IP address for Syslog server to use.

Set to "0.0.0.0" to use server provided by DHCP.

Example:
```
SYS:WIFI:SYSLOG 192.168.1.20
```

#### SYStem:WIFI:SYSLOG?
Display currently configured Syslog server.

Note, "0.0.0.0" means to use DHCP.

Example:
```
SYS:WIFI:SYSLOG?
192.168.1.20
```

#### SYStem:WIFI:SYSLOGClient
Enable or disable Syslog client that relays log messages to a remote (syslog) server.

Default: ON

Example (disable Syslog client):
```
SYS:WIFI:SYSLOGC OFF
```

#### SYStem:WIFI:SYSLOGClient?
Query current status of Syslog client.

Example:
```
SYS:WIFI:SYSLOGC?
ON
```

#### WHO?
Display remote connection information. This displays information about currently
connected telnet and SSH sessions.

Example:
```
WHO?
admin    telnet   192.168.1.10:59422 (Connected)
```

### WRIte Commands

#### WRIte:VSENSORx
Update the temperature reading for a virtual sensor that is configured in "MANUAL" mode
(see CONFigure:VSENSORx:SOUrce). This is intended to be used by external software to feed
a temperature reading (e.g. host CPU temperature) into LcdPico.

If the sensor is not in "MANUAL" mode, this command returns an error.

Example:
```
WRITE:VSENSOR1 42.5
```
