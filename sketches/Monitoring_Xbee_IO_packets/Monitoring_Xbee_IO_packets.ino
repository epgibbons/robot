
/*
 * A simple program for monitoring receive packets under 64 bit addressing.
 * The packets arrive on a serially connected (local) Xbee from a wirelessly connected (remote) Xbee.
 * 
 * The local Xbee is connected via pins 10 and 11
 * 
 */


#include <SoftwareSerial.h>

#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTBIN(x)     Serial.print (x, BIN)
 #define DEBUG_PRINTHEX(x)     Serial.print (x, HEX)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
 #define DEBUG_PRINTDECLN(x)  Serial.println (x,DEC)
 #define DEBUG_PRINTBINLN(x)  Serial.println (x,BIN)
 #define DEBUG_PRINTHEXLN(x)  Serial.println (x,HEX)
#else
 #define DEBUG_PRINT(x)   
 #define DEBUG_PRINTDEC(x) 
 #define DEBUG_PRINTBIN(x)     
 #define DEBUG_PRINTHEX(x) 
 #define DEBUG_PRINTLN(x) 
 #define DEBUG_PRINTDECLN(x) 
 #define DEBUG_PRINTBINLN(x) 
 #define DEBUG_PRINTHEXLN(x)
#endif

SoftwareSerial mySerial(10, 11); // RX, TX

struct dio{
  byte enabled;
  byte val;
};
struct aio{
  byte enabled;
  int val;
};

struct data{
  byte valid;
  struct dio dios[8];
  struct aio aios[6];  
};

struct data getData();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Arduino built in serial line connected");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  while (!mySerial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Local Xbee serial line connected");

}

void loop() { // run over and over
  struct data d;
  d = getData();
  if( d.valid){
    for( int i=0; i<8; i++){
      if( d.dios[i].enabled)
        Serial.println(d.dios[i].val);
      else
        DEBUG_PRINTLN("Not");
      
    }
    for( int i=0; i<6; i++){
      if( d.aios[i].enabled)
        Serial.println(d.aios[i].val);
      else
        DEBUG_PRINTLN("Not");
    }

  }
}

struct data getData(){
  struct data d ={ 0,{{0,0}}};
  DEBUG_PRINTLN("getData");
  if (mySerial.available()) {
    byte start_delim=mySerial.read();
    if( start_delim == 0x7e){
      DEBUG_PRINTLN("Start (0x7E) found. ");
      byte len[2];
      if( mySerial.readBytes(len,sizeof(len))==sizeof(len)){
        DEBUG_PRINT("Length = ");
        int length = (len[0]<<8) + len[1];
        DEBUG_PRINTLN(length);
        byte frameData[length];

        if( mySerial.readBytes(frameData,sizeof(frameData))==sizeof(frameData)){
          int j=0;
          byte api = frameData[j++];

          DEBUG_PRINT("API = ");
          DEBUG_PRINTHEXLN(api);
          if( api==0x82){
            DEBUG_PRINTLN("RX (Receive) Packet: 64-bit address I/O");
            //0x82 for RX (Receive) Packet: 64-bit address I/O
            DEBUG_PRINT("Address = ");
            for(int i= 0; i< 8; i++){
              byte ch = frameData[j++];
              DEBUG_PRINTHEX(ch);
            }
            DEBUG_PRINTLN();
            byte rssi = frameData[j++];
            DEBUG_PRINT("RSSI = ");
            DEBUG_PRINTHEXLN(rssi);
            byte options = frameData[j++];
            DEBUG_PRINT("Options = ");
            DEBUG_PRINTHEXLN(options);

            byte numSamples = frameData[j++];
            DEBUG_PRINT("Number of samples  = ");
            DEBUG_PRINTHEXLN(numSamples);

            byte channelIndicator1 = frameData[j++];
            DEBUG_PRINT("Channel indicator 1 = ");
            DEBUG_PRINTBINLN(channelIndicator1 );
            byte channelIndicator2 = frameData[j++];
            DEBUG_PRINT("Channel indicator 2 = ");
            DEBUG_PRINTBINLN(channelIndicator2);
            if( (channelIndicator1 & 0x1) || (channelIndicator2 != 0x0)){
              byte dio1 = frameData[j++];
              byte dio2 = frameData[j++];
              for( int dio=0; dio < 8; dio++){

                if( channelIndicator2 & (0x1<<dio) ){
                  Serial.print("DIO");
                  Serial.print(dio);                  
                  d.dios[dio].enabled=1;
                  d.dios[dio].val= (dio2 & 0x1<<dio) != 0;
                  Serial.print(" is enabled = ");
                  Serial.println( (dio2 & 0x1<<dio) != 0);
                }
                else {
                  DEBUG_PRINT("DIO");
                  DEBUG_PRINT(dio);                  
                  DEBUG_PRINTLN(" is NOT enabled");
                }
                
              }
              if( channelIndicator1 & 0x1){
                Serial.print("DIO8 is enabled = ");
                Serial.println( dio1 & 0x1);
              }
              else {
                DEBUG_PRINTLN("DIO8 is NOT enabled");

              }
            }
            for(int adc=0; adc< 7; adc++){
              byte checkCode = 0x1<<(adc+1);
              if( channelIndicator1 & checkCode){
                 Serial.print("ADC A");
                 Serial.print(adc);
                 Serial.print(" is enabled  = ");
                 byte msb = frameData[j++];
                 byte lsb = frameData[j++];
                 int val = (msb<<8) + lsb;
                 d.aios[adc].enabled=1;
                 d.aios[adc].val= val;
                 Serial.println( val, DEC);
              }
              else {
                DEBUG_PRINT("ADC A");
                DEBUG_PRINT(adc);
                 DEBUG_PRINTLN(" is NOT enabled");
              }
            }
            byte check = mySerial.read();
            byte verify=0;
            for( int i=0; i< sizeof(frameData); i++){
              verify += frameData[i];
            }
            if( check + verify != 0xFF){
              DEBUG_PRINTLN("Check sum is correct");
              d.valid=1;
            }
            else {
              DEBUG_PRINTLN("Check sum is incorrect");
            }
         }
      }
    }
   }
  }
  return d;
}
