/*
 Written by Yusef Noor, 22/05/2020.
 
  Allows for automatic or manual control of a motor's speed. Operational mode is manual
  by default. Toggle automatic mode by pressing the SELECT key.

  MANUAL MODE:
  The UP and DOWN keys increase speed in the clockwise and anticlockwise direction
  respectively. Speed number is indicated in row two of the LCD. Maximum speed in 
  either direction is 255. Pressing the LEFT or RIGHT key will make the direction
  of rotation anticlockwise or clockwise respectively.

  AUTO MODE:
  Speed is determined by the temperature of a thermistor and is displayed as a % of
  maximum speed in row two of the LCD. Speed increases linearly from  0 at 15
  degrees celsius to 100% at 35 degrees celsius. The direction of rotation 
  can also be set using the LEFT and RIGHT keys as with MANUAL MODE.
 */



#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // Initialising library with interface pin numbers

#define btnRIGHT  0         // Defining LCD keys
#define btnUP     1
#define btnDOWN   2 
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
int count = 0;              // Creating variable to control speed
int ModeSelect = 0;         // Creating variable to control Mode
int ThermistorPin = A2;     // Indicating analog pin from which thermistor voltage will be read
int DIRECTION = 1;          // Creating variable to control direction in Auto Mode
float R1 = 10000;           // resistance of resistor 
float Vo;                   // Variable to store thermistor voltage
float logR2, R2, T;         // Creating variables used in temperature calculation 

float a1 = 3.354016e-03, b1 = 2.569850e-04, c1 = 2.620131e-06, d1 = 6.383091e-08; // Thermistor temperature calculation constants


int read_LCD_buttons() {
  int key = analogRead(0);              // Mapping each key to the voltage range produced when the key is pressed  
  if (key > 1000) return btnNONE; 
  if (key < 50) return btnRIGHT;
  if (key < 250) return btnUP;
  if (key < 350) return btnDOWN;
  if (key < 450) return btnLEFT;
  if (key < 750) return btnSELECT;
  return btnNONE;
}

void setup() {      
lcd.begin(16,2);      // Initialising LCD dimmensions
pinMode(3, OUTPUT);   // Configuring digital pin 3 and 11 for output since they are connected to the motor inputs through the L293d
pinMode(11, OUTPUT); 

Serial.begin(9600);   // Allowing for obeservation of what is happening using serial monitor

}

void loop() {
 delay(20);                          // Delay chosen to minimise chance of button bounce causing unwanted mode to be selected
  int key = read_LCD_buttons();
  lcd.setCursor(0, 1);
  if (key == btnUP){                  // If UP key is detected in activated position, count increases by 1
  lcd.print(++count);
  lcd.print("                 ");     // Displaying nothing to ensure stray character do not remain on the display after the next print happens
  delay(20);
  }
lcd.setCursor(0,1);                  // If DOWN key is detected in activated position, count decreases by 1
  if (key == btnDOWN){     
  lcd.print(--count);
  lcd.print("                 ");
  delay(65);
  }

  if (count >= 255)                 // Limiting the value of count so that max speed is reached, count can not be increased further
  {
    count = 255;
    lcd.print(count);
    lcd.print("      ");
  }
  if (count <= -255) 
  {
    count = -255;
    lcd.print(count);
    lcd.print("      ");
  }

  if (count > 0)              // Enabling motor to rotate clockwise        
  {
    analogWrite(3, count);    // Digital pin 3 configured to output an average voltage proportional to count so that speed is proportional to count 
    analogWrite(11, LOW);     // Pin 11 set to output 0
  }

  if (count < 0)
  {
    analogWrite(3, LOW);          // Roles of pins 3 and 11 are swapped, when count is negative, to enable anticlockwise rotation instead   
    analogWrite(11, abs(count));
  }

if (key == btnRIGHT)             // When RIGHT key is pressed, value of count is made positive so that motor will rotate clockwise without the magnitude of count changing
{
  DIRECTION = 1;
  count = abs(count);
  lcd.print(count);
  lcd.print("             ");
}
if (key == btnLEFT) 
{
  DIRECTION = -1;
  count = -1*abs(count);        // When LEFT key is pressed, value of count is made negative so that motor will rotate anticlockwise without the magnitude of count changing
  lcd.print(count);
  lcd.print("             ");
}
if (key == btnSELECT)
{
  ModeSelect = ModeSelect + 1;  // Stores number of times SELECT key is detected in activated postition (ideally once per button press)
}
if (ModeSelect % 2 == 0)       // If the number of times SELECT key is pressed is even, Manual mode is displayed on the LCD
{
  lcd.setCursor(0, 0);  
  lcd.print("Manual ");
  lcd.setCursor(0, 1); 
}

else                         // AUTO MODE 
{
  
  lcd.setCursor(0, 0);  
  lcd.print("Auto     ");
  lcd.setCursor(0, 1);

  Vo = analogRead(ThermistorPin);                 // Reading the voltage across the thermistor (in the analog form of a number 0-1023)                       
    R2 = R1 * (1023.0 / (float)Vo - 1.0);         // Transfomring the analog value into its corresponding voltage value and working out resistance
    R2 = R2/R1;
    logR2 = log(R2);
    T = (1 / (a1 + b1*logR2 + c1*logR2*logR2 + d1*logR2*logR2*logR2));   // Formula to work out temperature of thermistor from its resistance
    T = T - 273.15;                                                      // Conv from Kelvin to Celsius 

 if (DIRECTION > 0) {          // Making motor turn clockwise if RIGHT key is pressed
   count = (T-15)*(255/20);    // Creating a linear relationship between speed and temperatue where speed is 0 at 15 celsius and max at 35 celsius
 }

  if (DIRECTION < 0) {        // Making motor turn anticlockwise if LEFT key is pressed
   count = -(T-15)*(255/20); 
 }
                                               
    lcd.setCursor(0,1);
    lcd.print(100*(float)count/255);                                     // Working out current speed as a % of maximum speed 
    lcd.print("% Max Speed");
    lcd.print("                 ");
    delay(65);

}

if (abs(count) > 0 )          // Using serial monitor to allow observation of count, speed and temperature on the computer, where it is saved, instead of the LCD
{

Serial.print("    Temp = ");           
Serial.print(T);

Serial.print("    Count = ");
Serial.print(count);


Serial.print("    Speed(%) = ");
Serial.print(100*(float)count/255);
Serial.print("          ");

delay(0); // Delay here is set to >1000 ms for testing to allow easier reading of serial monitor. For regular function it is set to 0. 
}

}
