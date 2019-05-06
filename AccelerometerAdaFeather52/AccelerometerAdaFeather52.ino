/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <bluefruit.h>

#define xAcc  A3
#define yAcc  A4
#define zAcc   A5

#define BLUE_PIN  3
#define GREEN_PIN 4
#define RED_PIN   2

/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

// String to send in the throughput test
//#define TEST_STRING     "01234567899876543210"
//const int TEST_STRLEN = strlen(TEST_STRING);
//
//// Number of total data sent ( 1024 times the test string)
//#define TOTAL_BYTES     (1024 * strlen(TEST_STRING))

BLEDis bledis;
BLEUart bleuart;
BLEBas  blebas;  // battery

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
    //LEDs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  pinMode(xAcc, INPUT);
  pinMode(yAcc, INPUT);
  pinMode(zAcc, INPUT);
  
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Throughput Example");
  Serial.println("------------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this manually via PIN 19
  Bluefruit.autoConnLed(true);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("_TimsBluefruit52");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();
  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);
  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Set up and start advertising
  startAdv();
}

void startAdv(void)
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // There is no room for Name in Advertising packet
  // Use Scan response for Name
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle)
{
  (void) conn_handle;
  Serial.println("Connected");
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  

  while (Bluefruit.connected() && bleuart.notifyEnabled())
  {
    
    int xRead = analogRead(xAcc);
    int yRead = analogRead(yAcc);
    int zRead = analogRead(zAcc);

    //light the led according to side
    mapAccToSide(xRead, yRead, zRead);

    Serial.print("Sending ");
    
    Serial.print(xRead);
    Serial.print(", ");
    Serial.print(yRead);
    Serial.print(", ");
    Serial.println(zRead);
    
    uint16_t x  = xRead;
    uint16_t y  = yRead;
    uint16_t z  = zRead;
    String accString = String(xRead) + ',' + String(yRead) + ',' + String(zRead);
    char delimit = ',';
    bleuart.print(accString);
   
    byte ch = bleuart.read();
    //Serial.println((String)ch);
    if (int(ch) != 255)
    {
      Serial.println("Data received");
      LedOff();
      delay(500);
    }
//    bleuart.print(x);
//    bleuart.print(delimit);
//    bleuart.print(y);
//    bleuart.print(delimit);
//    bleuart.println(z);
    
    
  }
}

/**************************************************************************/
/*!
    @brief  Get user input from Serial
*/
/**************************************************************************/
char* getUserInput(void)
{
  static char inputs[64+1];
  memset(inputs, 0, sizeof(inputs));

  // wait until data is available
  while( Serial.available() == 0 ) { delay(1); }

  uint8_t count=0;
  do
  {
    count += Serial.readBytes(inputs+count, 64);
  } while( (count < 64) && Serial.available() );

  return inputs;
}

/**************************************************************************/
/*!
    @LED FUNCTS
*/
/**************************************************************************/
void mapAccToSide(int x, int y, int z)
{
  //side 0 up
  // X = 470   Y = 478   Z = 571 
  if ((x > 400 && x < 500) && (y > 400 && y < 500) && (z > 520))
  {
    Serial.println("GREEN SIDE UP");
    GreenLedOn();
  }
  // tilt to left
  //  X = 484   Y = 568   Z = 488
  else if ((x > 400 && x < 500) && (y > 500) && (z > 420 && z < 520))
  {
    Serial.println("BLUE SIDE UP");
    BlueLedOn();
  }

  //  // tilt to right 
  //  X = 470   Y = 371   Z = 477 
    else if ((x > 400 && x < 500) && (y < 400) && (z > 420 && z < 520))
  {
    Serial.println("RED SIDE UP");
    RedLedOn();
  }

  //  // tilt forward
  //  X = 567   Y = 473   Z = 478 
  else if ((x > 500) && (y > 400 && y < 500) && (z > 420 && z < 520))
  {
    Serial.println("Purple SIDE UP");
    PurpleLedOn();
  }
  //  //tilt back
  //  X = 375   Y = 470   Z = 478 
  else if ((x < 500) && (y > 400 && y < 500) && (z > 420 && z < 520))
  {
    Serial.println("Yellow SIDE UP");
    YellowLedOn();
  }
  //  //upside down
  //  X = 469   Y = 469   Z = 377 
  else if ((x > 400 && x < 500) && (y > 400 && y < 500) && (z < 420))
  {
    Serial.println("White SIDE UP");
    WhiteLedOn();
  }
  
  else{
    LedOff();
  }
}

void colorRGB(int red, int green, int blue){
  analogWrite(RED_PIN,constrain(red,0,255));
  analogWrite(GREEN_PIN,constrain(green,0,255));
  analogWrite(BLUE_PIN,constrain(blue,0,255));
}

void WhiteLedOn() {
  colorRGB(255, 255, 255);
}
void BlueLedOn() {
  colorRGB(0, 0, 255);
}
void GreenLedOn() {
  colorRGB(0, 255, 0);
}
void RedLedOn(){
  colorRGB(255, 0, 0);
}
void PurpleLedOn(){
  colorRGB(255, 0, 255);
}
void YellowLedOn(){
  colorRGB(255, 255, 0);
}
void LedOff(){
  colorRGB(0, 0, 0);
}
