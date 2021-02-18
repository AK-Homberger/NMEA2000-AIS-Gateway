# NMEA2000 AIS Gateway

This repository shows how to build a gateway to read NMEA0183 AIS messages and transform them to NMEA2000.

The purpose is to use existing cheap RS232 AIS receiver together with devices (e.g. MFD) that support only NMEA2000.
Many small Chart Plotters only support NMEA2000 an do not have an additional RS232 or RS422 input.

The used ESP32 reads AIVDM messages from UART2 (connected to GPIO 16).

The solution is re-using the code from [this](https://github.com/aduvenhage/ais-decoder) GitHub repository. Its under [MIT-License](AIS decoder is under MIT license: https://github.com/aduvenhage/ais-decoder/blob/master/LICENSE).

It was necessary to change the code to make it usable with Arduino IDE and the ESP32. Ther reason for the changes was the diferent versions of C-Compiler versions (clang#14 to clang#11). It was quite an effort to do the backport to Arduino clang#11 version, because of missing functions.

Currently following AIS message types are supported: 1-3, 5, 18, 24A, 24B

To does:
- Message type 14 for AIS safety related brodcast messages (to support AIS MOB devices)
- ETA calculation for message 5 translation

To use the gateway the following librariis have to be installed:


