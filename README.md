# Diploma Thesis LWM2M Library
## BUT - Brno University of Technology

[![university](https://img.shields.io/badge/university-Brno%20University%20of%20Technology-red.svg)](https://www.vutbr.cz/en/)
[![faculty](https://img.shields.io/badge/faculty-Faculty%20of%20Electrical%20Engineering%20and%20Communication-blue.svg)](https://www.fekt.vutbr.cz/)

This is the repository for a final stretch of diploma thesis for BUT Telecommunications study programme.
Topic of the thesis is Laboratory demonstrator for LPWA utilizing LWM2M protocol. Library is written in C++ with respect to the modern CPU constrains, therefore features such as polymorphism, dynamic casting or templates are not used.

Provided code can be viewed as a working proof of concept of the LWM2M implementation. <strong>In no means can this library be used in production 
devices</strong> since it contains a lot of compromises and shorcuts that had to be made to meet the deadline for the thesis.
Library was developed and tested against open-source LWM2M server Leshan. 
Functionality of the library was tested on 4 transmission technologies. Ethernet, WiFi, NB-IoT and LTE-Cat-M. <br>
Every piece of code in this library is original written. No piece of code has been copied from external sources.

### Capabilities
Full standalone library with no dependency on other systems or SDKs.
Library supports full modularity - application is not constrained to use of certain technologies.<br>
Library implements LWM2M v1.0 standard with most of its features including :
- Automatic registration to the server.
- Automatic update scheduling based on internal timing structure.
- Device Management and Information reporting interfaces
- Observe functionality with min & max attributes.
- Plain text and JSON formats.
- Automated responses to requests.
- Dynamic lifetime configuration.

### Prerequisites
With library's full modularity, certain functions and procedures have to be programmed separately, outside of the library. These fuctions include:
- Custom send and receive functions for given module/interface.
- Custom reboot function.
- Timer function that calls timing advance function to keep track of time within the library.
- Frequent calls of the loop function.
