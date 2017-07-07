# vsomeip-openni2

A use case demonstrating SOME/IP communication between two Linux embedded boards mimicing an ISO 17215 scenario for camera control and communication. It covers a basic application for controlling parameters and streaming RGB and Depth Image data from the ASUS Xtion Pro camera on a gigabit Ethernet LAN.
The project uses the vsomeip library from GENIVI [vsomeip](https://github.com/GENIVI/vsomeip) and the examples in the wiki along with [openni2_primesense_viewer](https://github.com/ashwin/openni2_primesense_viewer/) 

## Hardware setup:
ASUS Xtion Pro --> Raspberry Pi (Ethernet Camera Unit) --> Ethernet Switch <--- Kontron pITX-E38 board

## License:
GNU General Public License v3.0
