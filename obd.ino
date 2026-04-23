#include "mcp_can.h"
#include "obd.h"
/*
 * @file: obd.ino
 * @author: Scott Harding
 * @date: 2026-04-23
 * @description: forked from https://www.instructables.com/Arduino-OBD2-Simulator/
 * mviljoen2
 */

// Global vars
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);
unsigned char current_mode_num = 1;
unsigned char size_of_parm[NUM_MODES];

void setup() {
  init_DTCStatus(false, 6, "spark");
  size_of_parm[0] = sizeof(mode1_parms);
  size_of_parm[1] = sizeof(mode2_parms);
  size_of_parm[2] = sizeof(mode3_parms);
  size_of_parm[3] = sizeof(mode4_parms);
  size_of_parm[4] = sizeof(mode5_parms);
  size_of_parm[5] = sizeof(mode6_parms);
  size_of_parm[6] = sizeof(mode7_parms);
  size_of_parm[7] = sizeof(mode8_parms);
  size_of_parm[8] = sizeof(mode9_parms);
  size_of_parm[9] = sizeof(mode10_parms);
  //init all modes with parameters
  for(int i=0; i<NUM_MODES ; i++){
     mode[i].parameter = parameters[i];
  }
  Serial.begin(9600);
  while (
      !(CAN_OK == CAN.begin(CAN_500KBPS,
                            MCP_8MHz))) { // bit rate and set crystall frequency
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");
}

void loop() { check_CAN(); }

void check_CAN() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    unsigned char len = 0;
    unsigned char buf[8];
    CAN.readMsgBuf(&len, buf);
    uint32_t canId = CAN.getCanId();
    unsigned char payload_size =
        buf[0] & 0x0f;                  // lower nibble but only give 0-7 bytes
    unsigned char code = buf[0] & 0xf0; // upper nibble
    unsigned char mode_number = buf[1];
    unsigned char pid = buf[2];
    

    // send remaining frames for multi-frame VIN
    if (code == 0x30) {
      if (current_mode_num == 9 && pid == 0x02) {
        CAN.sendMsgBuf(0x7E8, 0, 8, VIN_FRAME1);
        CAN.sendMsgBuf(0x7E8, 0, 8, VIN_FRAME2);
      } else if (current_mode_num == 9 && pid == 0x0A) {
        CAN.sendMsgBuf(0x7E8, 0, 8, ECU_NAME_FRAME1);
        CAN.sendMsgBuf(0x7E8, 0, 8, ECU_NAME_FRAME2);
      } else if (current_mode_num == 3) {
        CAN.sendMsgBuf(0x7E8, 0, 8, GET_DTC_FRAME1);
        CAN.sendMsgBuf(0x7E8, 0, 8, GET_DTC_FRAME2);
      }
      current_mode_num = 1;
    } else {
      //used for multi-frame messages
      current_mode_num = mode_number;
      // search for the specific data struct associated with given PID
      unsigned int index;
      //get mode 
      Mode m = mode[mode_number - 1];
      //find the index of the parameter
      for (index = 0; index < size_of_parm[mode_number-1]/sizeof(mode[mode_number-1].parameter[0]) ; index++) {
        if (m.parameter[index].pid == pid) {
          break;
        }
      }
      //return bit stream of PID 
      CAN.sendMsgBuf(0x7E8, 0, 8, m.parameter[index].bit_stream);
    }
  }
}

void init_DTCStatus(bool MIL_on, unsigned char numDTCs, String ignition_type) {
  // total number of bytes of data, including prefix
  STATUS[0] = 0x06;
  // Mode = 0x40 | 01
  STATUS[1] = 0x41;
  // PID=1
  STATUS[2] = 0x01;
  // Byte A6-0bits 6:0 of byte 2 set number of DTCs
  STATUS[3] = numDTCs;
  // Byte A7  MSB of byte 2 state of MIL
  STATUS[3] |= MIL_on << 7;
  // Byte B7 reserved (0) 
  STATUS[4] = 0x00;
  STATUS[4] &= ~(1 << 7);
  // Byte B6-B4 completenes of common tests
  STATUS[4] |= (1 << 6);
  STATUS[4] |= (1 << 5);
  STATUS[4] |= (1 << 4);
  // Byte B3 ignition type
  // B3 = 1 if compression
  if (ignition_type == "compression") {
    STATUS[4] |= (1 << 3);
  }
  // Byte B2-B0 availability of common tests
  STATUS[4] |= (1 << 2);
  STATUS[4] |= (1 << 1);
  STATUS[4] |= (1 << 0);
  STATUS[5] = 0xff;
  STATUS[6] = 0xff;
  STATUS[7] = 0xff;
}
