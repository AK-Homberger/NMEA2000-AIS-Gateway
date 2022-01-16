# NMEA0183 to NMEA2000 AIS Gateway

This repository shows how to build a gateway to read NMEA0183 AIS messages and transform them to NMEA2000 PGNs.

If you need the opposite direction, then this might be of interest for you: https://github.com/ronzeiller/NMEA0183-AIS

The purpose is to use an existing NMEA0183 AIS receiver together with devices (e.g. MFD) that support only NMEA2000.
Many new chart plotters only support NMEA2000 and do not have an additional RS232 or RS422 input.

I had to change and add some AIS related [PGN structures](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/bea3ec6255678d2c806bf09e5bebd815074605ea/MyAISToN2k/NMEA0183AIStoNMEA2000.h#L73) in addition to the NMEA2000 library to make this work also with Raymarine MFDs.

![AIS1](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/AIS1.jpg)
![AIS2](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/AIS2.JPG)

The used ESP32 reads AIVDM messages from UART2 (connected to GPIO 16, baud rate 38400). Forwarding to NMEA2000 is done via a CAN bus transceiver.

The solution is re-using AIS decoder code from [this](https://github.com/aduvenhage/ais-decoder) GitHub repository. It is published under [MIT-License](https://github.com/aduvenhage/ais-decoder/blob/master/LICENSE).

It was necessary to change the code, to make it usable with the Arduino IDE and the ESP32. The reason for the changes was the different C compiler versions (clang#14 to clang#11). It was quite an effort to do the backport to Arduino clang#11 version, because of missing functions.

Currently, following AIS message types are supported: 1-3, 5, 14, 18, 19, 24A, 24B

Detailled information regarding AIS messages can be found [here](https://gpsd.gitlab.io/gpsd/AIVDM.html).

Only AIVDM messages (other ships) are decoded by default. If you also want to decode and forward own ship messages (AIVDO) comment/uncomment the appropriate lines (code change requires Arduino IDE):

```
// Select (comment/uncomment) if you want to decode only other ship (AIVDM) or also own ship (AIVDO) messages
// if (!NMEA0183Msg.IsMessageCode("VDM") && !NMEA0183Msg.IsMessageCode("VDO")) return;   // Not a AIVDM/AIVDO message, return
if (!NMEA0183Msg.IsMessageCode("VDM")) return;   // Not a AIVDM message, return
```
# Software Install
To install the program on the ESP32 you do have two options:

1. Install the binary file with the **NodeMCU PyFlasher** tool
2. Compile and upload the program with the **Arduino IDE**

## NodeMCU PyFlasher

The easiest way to upload the binary file is to use the [NodeMCU PyFlasher tool](https://github.com/marcelstoer/nodemcu-pyflasher) tool.

Download the appropriate version for your operating system. Then download the binary file **[MyAISToN2k.ino.esp32.bin](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/MyAISToN2k.ino.esp32.bin)** to the PC.

Then start the NodeMCU PyFlaher program and select the downloaded [binary](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/MyAISToN2k.ino.esp32.bin) and set the following options including the correct COM port:

![Esptool](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/NodeMCUPyFlasher.png)

Then press "Flash NodeMCU". On some ESP32 it might be nessessary to press the "BOOT" button to start uploading.

## Arduino IDE

To install the [software](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/tree/main/MyAISToN2k) with the Arduino IDE you have to install the IDE and the ESP32 board support.
Then you have to install three libraries.

To use the gateway the following libraries have to be installed (as ZIP file):
- [NMEA2000](https://github.com/ttlappalainen/NMEA2000)
- [NMEA2000_esp32](https://github.com/ttlappalainen/NMEA2000_esp32)
- [NMEA0183](https://github.com/ttlappalainen/NMEA0183)

Jus click on the green "Code" button and select "Download as ZIP".

Then download the [repository](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/archive/refs/heads/main.zip) as ZIP.file and upack it.
The Arduino code is in the "MyAISToN2k" folder.

How to install and use the Arduino IDE is explained in the [NMEA2000 workshop](https://github.com/AK-Homberger/NMEA2000-Workshop).

# Hardware
The schematics and the PCB from the [WLAN gateway](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32) can be used for the gateway. Just ignore the not needed parts.

![schematics](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/KiCAD/ESP32WifiAisTempVolt2/ESP32WifiAisTempVolt2.png)

![PCB](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/KiCAD/ESP32WifiAisTempVolt2/ESP32WifiAisTempVolt2-PCB.png)

The board can be ordered at Aisler.net: https://aisler.net/p/DNXXRLFU

**!!! Be careful with placing the the ESP32 on the PCB. The USB connector has to be on the right side !!!**

## Remove the 120 ohm resistor from the transceiver
For unknown reasons, many CAN bus transceivers for the ESP32 have a 120 Ohm resistor built into them. The resistor does not belong to the devices at the ends of the stub lines, but to the ends of the backbone cable.

Whether the transceiver contains a 120 ohm resistor can be determined either by looking at the [circuit diagram](https://github.com/AK-Homberger/NMEA2000-Workshop/blob/main/Docs/SN65HVD230%20CAN%20Board_SCH.pdf) or by measuring with the multimeter.

A knife tip is suitable for removing the SMD resistor. Use it to scratch or pry off the resistance. With the transceiver shown here, place the tip of the knife in front of the resistor (between the chip and the resistor) and support the knife against the chip. Then lever the tip against the resistance and use it to scratch / loosen / break the resistance. Whichever happens first.

![Transceiver](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/CAN-Transceiver.jpg)

It then looks like the picture. Then measure to be on the safe side. Without a 120 ohm resistor, the multimeter shows approx. 75 kOhm.

## Parts:
You only need these parts for the gateway:

- U1 ESP32 [Link](https://www.amazon.de/AZDelivery-NodeMCU-Development-Nachfolgermodell-ESP8266/dp/B071P98VTG/ref=sxts_sxwds-bia-wc-drs3_0?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&cv_ct_cx=ESP32&dchild=1&keywords=ESP32) 
- J5 SN65HVD230 [Link](https://eckstein-shop.de/Waveshare-SN65HVD230-CAN-Board-33V-ESD-protection)
- J2 D24V10F5 [Link](https://eckstein-shop.de/Pololu-5V-1A-Step-Down-Spannungsregler-D24V10F5)
- R4 Resistor 4,7 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-4-7-kohm-1-4w-4-7k-p1425.html?&nbc=1)
- R5 Resistor 33 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-33-kohm-1-4w-33k-p1412.html?&nbc=1)
- Q2 Transistor BC547 [Link](https://www.reichelt.de/de/en/small-signal-transistors-npn-to-92-45-v-rnd-bc547-p223356.html?&nbc=1)
- D1 Diode 1N4001 [Link](https://www.reichelt.com/de/en/rectifier-diode-do41-50-v-1-a-1n-4001-p1723.html?&nbc=1)
- D2 Diode 1N4148 [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-p1730.html?search=1n4148)
- J1, J3 Connector 2-pin [Link](https://www.reichelt.de/de/en/2-pin-terminal-strip-spacing-5-08-akl-101-02-p36605.html?&nbc=1)


# Updates:
- 13.01.22 Version 0.7: Select wether AIVDO messages (own ship) are to be decoded also in addition to AIVDM.
- 13.01.22 Version 0.6: Corrected time PGNs for ETA calculation and changed AIS PGN handling for Raymarine MFDs.
- 01.03.21 Version 0.5: Send message type 19 as PGN129040 (instead of 18 and 24a/24b PGNs).
- 20.02.21 Version 0.4: Added message type 14 support.
- 19.02.21 Version 0.3: Added message type 19 support.
- 19.02.21 Version 0.2: Added ETA calculation for message 5.
- 18.02.21 Version 0.1: Initial version.
