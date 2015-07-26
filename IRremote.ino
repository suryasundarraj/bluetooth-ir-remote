/*Code for the Product 
* IR Bluetooth Remote Record and Play
****************************ELINT LABZ******************************
***************************www.elintlabz.in*************************
************************Created by SURYA SUNDAR RAJ D***************
*
*In this Project we can store upto 128 Remote Buttons to the EEPROM
and also access them using a Bluetooth Terminal of any Smart Phone.
*
*/

//Import the Libraries Required which are available for ARDUINO

#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <IRremote.h>

//Declaring Variables
int RECV_PIN = 11;                //Initialize the PIN for TSOP
unsigned long  codevalue;
char code[7];                        //Array where the received codes are saved
int length;

IRrecv irrecv(RECV_PIN);          //Objects Created for the Project
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(9600);            //Initialize the Baud Rate 
  irrecv.enableIRIn();           // Start the receiver
  pinMode(7,INPUT);
  pinMode(9, INPUT);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    Serial.println(codeValue);
    codeLen = results->bits;
   }
}

//This Function will Trasmit the Code which is saved in the EEPROM
void sendCode(int repeat) 
{
  if (codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(codevalue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codevalue, HEX);
    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codevalue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codevalue = codevalue & ~(1 << (codeLen - 1));
    codevalue = codevalue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codevalue, HEX);
      irsend.sendRC5(codevalue, codeLen);
    } 
    else {
      irsend.sendRC6(codevalue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

int lastButtonState;

void loop() {
  if(Serial.available()>0)
  {
    for(int i = 0 ;i<=6;i++)
    {
      code[i] = Serial.read();
      delay(100);
    }
    command();
  }
  if(9 == HIGH)
  {
    for(int i; i<=512; i++)
    {
      EEPROM.writeLong(i,0);
    }
    Serial.println("Doneee");
  }
}

//Checks the Command Received
void command()
{
  if(code[0] == 'R')
  {
    calc();
    delay(10);
    save();
    delay(1000);
    delay(2000);
  }
  else if(code[0] == 'A')
  {
    acc();
    delay(2000);
  }
  else if(code[0] == 'C')
  {
    clr();
    delay(1000);
  }
}

void convert()
{
   code[2] = code[2] - '0';
   code[3] = code[3] - '0';
   code[4] = code[4] - '0';
   length = (code[2]*100) + (code[3]*10) + code[4];
}

int n;

void calc()
{
  convert();
  delay(10);
  if(length == 1)
  {
    n = 0;
  }
  else
  {
    n = (length-1)*4;
  }
}

void sendcode()
{
  butread(); 
  if (irrecv.decode(&results)) 
  {
    storeCode(&results);
  }
}


void sendrem()
{
    Serial.println(codevalue, HEX);
//  digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState == 1);
//  digitalWrite(STATUS_PIN, LOW);
    delay(50);
}


void butread()
{
  if (lastButtonState == HIGH || irrecv.decode(&results)) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
    storeCode(&results);
    irrecv.resume(); // resume receiver
  }
  delay(500);
}

//Records the Code to the EEPROM
void save()
{
 // Serial.print("Recording Button  ");
  Serial.print(l);
//  Serial.print(" :\t");
  sendcode();
  EEPROM.writeLong(n,0);
  delay(100);
  EEPROM.writeLong(n,codeValue);
 // digitalWrite(STATUS_PIN, LOW);
  irrecv.resume();
} 

//Access the Stored Codes from the EEPROM
void acc()
{
  calc();
  delay(10);
  codevalue = EEPROM.readLong(n);
  sendrem();
}

//Clears the EEPROM
void clr()
{
  for(int i = 0; i<=512; i++)
  {
    EEPROM.write(i,0);
  }
  delay(500);
  Serial.println("Cleared EEPROM Successfully");
}

//End of the Program
