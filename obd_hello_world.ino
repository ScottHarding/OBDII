#include "mcp_can.h"
#include "obd.h"

// Global vars
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);
unsigned char size_of_parm[NUM_MODES];

void setup() {
  //need this due to pointer "decaying" in C++
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

void loop() { 
   check_CAN(); 
}

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
