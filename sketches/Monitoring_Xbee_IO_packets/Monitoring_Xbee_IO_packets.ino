
/*
 * A simple program for monitoring receive packets under 64 bit addressing.
 * The packets arrive on a serially connected (local) Xbee from a wirelessly connected (remote) Xbee.
 * 
 * The local Xbee is connected via pins 10 and 11
 * 
 */


#include <SoftwareSerial.h>


SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

}

void loop() { // run over and over
  if (mySerial.available()) {
    byte start_delim=mySerial.read();
    if( start_delim == 0x7e){
      Serial.println("Start found. ");
      byte len[2];
      if( mySerial.readBytes(len,sizeof(len))==sizeof(len)){
        Serial.print("Length = ");
        int length = (len[0]<<8) + len[1];
        Serial.println(length);
        byte frameData[length];

        Serial.print("sizeof(frameData) = ");
        Serial.println(sizeof(frameData));
        if( mySerial.readBytes(frameData,sizeof(frameData))==sizeof(frameData)){
          int j=0;
          byte api = frameData[j++];

          for( int k=0; k< sizeof(frameData); k++){
            Serial.println(frameData[k], HEX);
          }
          
          Serial.print("API = ");
          Serial.println(api,HEX);
          if( api==0x82){
            Serial.println("RX (Receive) Packet: 64-bit address I/O");
            //0x82 for RX (Receive) Packet: 64-bit address I/O
            Serial.print("Address = ");
            for(int i= 0; i< 8; i++){
              byte ch = frameData[j++];
              Serial.print(ch, HEX);
            }
            Serial.println();
            byte rssi = frameData[j++];
            Serial.print("RSSI = ");
            Serial.println(rssi, HEX);
            byte options = frameData[j++];
            Serial.print("Options = ");
            Serial.println(options, HEX);

            byte numSamples = frameData[j++];
            Serial.print("Number of samples  = ");
            Serial.println(numSamples, HEX);

            byte channelIndicator1 = frameData[j++];
            Serial.print("Channel indicator 1 = ");
            Serial.println(channelIndicator1  , BIN);
            byte channelIndicator2 = frameData[j++];
            Serial.print("Channel indicator 2 = ");
            Serial.println(channelIndicator2, BIN);
            if( ((channelIndicator1 & 0x1) == 0x1) || (channelIndicator2 != 0x0)){
              Serial.println("A DIO line is enabled");
              byte dio1 = frameData[j++];
              Serial.print("dio1 = ");
              Serial.println(dio1, HEX);
              byte dio2 = frameData[j++];
              Serial.print("dio2 = ");
              Serial.println(dio2, HEX);
            }
            for(int adc=0; adc< 7; adc++){
              byte checkCode = 0x1<<(adc+1);
              if( (channelIndicator1 & checkCode) == checkCode){
                 Serial.print("ADC A");
                 Serial.print(adc);
                 Serial.print(" is enabled  = ");
                 byte msb = frameData[j++];
                 byte lsb = frameData[j++];
                 int val = (msb<<8) + lsb;
                 Serial.println( val, DEC);
              }
            }
            Serial.print("CheckSum = ");
            Serial.println(mySerial.read(), HEX);
            
         }
      }
    }
   }
  }
}
