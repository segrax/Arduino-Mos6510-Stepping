/**
 * MOS6510 CPU Stepping
 *
 * (c) 2014 Robert Crossfield
 **/

int pinClock = 04;
int pinReady = 22;
int pinReset = 23;
int pinRW    = 48;

int pinsAddress[16] = {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39};
int pinsData[8]     = {40, 41, 42, 43, 44, 45, 46, 47};

int previousAddress = 0;
bool previousRW = false;

String commandInput = "";
boolean commandComplete = false;

const unsigned char Memory[ 0x20 ] = { 0x00, 0x00, 0x78, 0x4c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x78, 0x4C, 0x09, 0x00, 0xA9, 0x04 };

void setup() {
  commandInput.reserve(200);
  
  Serial.begin(9600);
  
  // Timer0 frequency set to 62500 Hz
  //TCCR0B = TCCR0B & 0b11111000 | 0x01;
  
  pinMode(pinClock, OUTPUT);
  pinMode(pinReady, OUTPUT);
  pinMode(pinReset, OUTPUT);
  pinMode(pinRW,    INPUT);
    
  for( int pin = 0; pin < 8; ++pin )
    pinMode(pinsData[pin], OUTPUT);

  setData(0x00);
  
  digitalWrite( pinReady, LOW );
  digitalWrite( pinReset, HIGH );
  
  analogWrite( pinClock, 127 );  // 50% on, 50% off
}

/**
 * Handle serial port event
 **/
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();

    commandInput += inChar;
    if (inChar == '\n')
      commandComplete = true;
  }
}

/**
 * Get the address pins
 **/
unsigned short getAddress() {
  unsigned short Address = 0;
  
  for( int pin = 0; pin < 16; ++pin )
    pinMode(pinsAddress[pin], INPUT);
    
  for( int pin = 0; pin < 16; ++pin ) {
    
      if( digitalRead( pinsAddress[pin] ) == HIGH )
        Address |= (1 << pin);
  }
  
  return Address;
}

/**
 * Set the data line
 **/
void setData( unsigned char pData ) {
  
    for( int pin = 0; pin < 8; ++pin ) {
      if( pData & 1 )
        digitalWrite( pinsData[pin], HIGH );
      else
        digitalWrite( pinsData[pin], LOW );
        
        pData >>= 1;
    }
}

/** 
 * Reset The CPU
 **/
void Reset() {
    digitalWrite( pinReset, LOW );
    delay(10);
    digitalWrite( pinReset, HIGH );
}

/**
 * Single Step
 */ 
void Step() {
  digitalWrite( pinReady, HIGH );
  delay(1);
  digitalWrite( pinReady, LOW );
}

/**
 * Get the READ/WRITE Pin status
 *
 * @return True if READING, False if WRITTING
 **/ 
bool GetRW() {
  if( digitalRead( pinRW ) == HIGH )
    return true;
  
  return false;  
}

/**
 * Main Loop
 **/
void loop(){
  
  // Get current address pins
  unsigned short Address = getAddress();
  
  // Get current Read/Write Status
  bool RW = GetRW();
  
  // Has the read/write pin changed since the last loop
  if( RW != previousRW ) {
    if( RW == true )
      Serial.print("READ  ");
    else
      Serial.print("WRITE ");
      previousRW = RW;
  }
  
  // Has the address changed since the last loop
  if( Address != previousAddress ) {
    Serial.print("Address: 0x");
    Serial.print( Address, HEX );

    previousAddress = Address;
  
    if( RW == true ) {
      unsigned char Data;
      
      switch( Address ) {
        case 0xFFFC:
          Data = 0x02;
          break;
          
        case 0xFFFD:
          Data = 0x00;
          break;
          
        default:
          Data = Memory[ Address ];
          break;
      }
      Serial.print(" Data: ");
      Serial.print(Data, HEX );
      
      setData( Data ); 
    }
    Serial.println("");
  }
  delay(10);
  
  // Check for commands from serial port
   if( commandComplete == true ) {
    commandComplete = false;
    
    if( commandInput == "r\n" )
      Reset();
    if( commandInput == "z\n" )
        Step();
        
    commandInput = "";
  }
}

