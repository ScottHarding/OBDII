This Arduino code emulates an OBDII-compliant Engine Control Unit (ECU). It's not a code reader (like most projects), 
but rather is an OBDII code "generator" in the sense that it emulates the computer inside of most modern cars. It will
generate real or simulated parameters such as engine coolant temperature. It also generates Diagnotic Trouble Codes (DTCs), which
again can either be simulated or generated automatically.

This is forked from https://www.instructables.com/Arduino-OBD2-Simulator.

Dependencies: Arduino CAN library. Hardware requirements: An Arduino connected to a CANBUS.
