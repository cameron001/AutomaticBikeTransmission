#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <TinyGPSPlus.h>


//SCREEN STUFF FOR DISPLAY
const int rs = 52, en = 51, d4 = 35, d5 = 34, d6 = 33, d7 = 32;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int rx = 12;
const int tx = 11;
SoftwareSerial ss(rx,tx);


typedef struct task {
 int state;
 unsigned long period;
 unsigned long elapsedTime;
 int (*TickFct)(int);


} task;


const unsigned short tasksNum = 6;
task tasks[tasksNum];


int up = 0;
int down = 0;
int gear = 0;
int shiftup = 0;
int shiftdown = 0;
int speed = 0;
double acceleration = 0;
int accelJump = 0;
int shiftupQ=0;
int shiftdownQ=0;
int demoMode = 0;


enum GEAR_STATES { gearstart, one, two, three, four, five, six };
int GEAR_TICK(int state1) {
 switch (state1) {
   case gearstart:
     shiftupQ = 0;
     shiftdownQ = 0;
     state1 = one;
     break;
   case one:
     state1 = one;
     if(speed >= 10 || (up == 1 && down != 1)){
       // shiftdown = 0;
       // shiftup = 1;
       shiftdownQ = 0;
       if(accelJump){
         if(demoMode){
           speed +=10;
         }
         shiftupQ += 2; 
         state1 = three;       
       }
       else{
         ++shiftupQ;
         up = 0;
         down = 0;
         state1 = two;
       }
     }
     else{
       state1 = one;
     }
     break;
   case two:
     if(speed >= 20 || (up == 1 && down != 1)){
       shiftdownQ = 0;
       if(accelJump){
         if(demoMode){
           speed +=10;
         }
         shiftupQ += 2; 
         state1 = four;       
       }
       else{
         up = 0;
         down = 0;
         // shiftdown = 0;
         // shiftup = 1;
         ++shiftupQ;
         state1 = three;
       }
     }
     else if(speed < 10 || (down == 1 && up != 1)){
       shiftupQ = 0;
       up = 0;
       down = 0;
       // shiftup = 0;
       // shiftdown = 1;
       ++shiftdownQ;
       state1 = one;
     }
     else{
       state1 = two;
     }
     break;
   case three:
     if(speed >= 30 || (up == 1 && down != 1)){
       shiftdownQ = 0;
       if(accelJump){
         if(demoMode){
           speed +=10;
         }
         shiftupQ += 2; 
         state1 = five;       
       }
       else{
         up = 0;
         down = 0;
         // shiftdown = 0;
         // shiftup = 1;
         ++shiftupQ;
         state1 = four;
       }
     }
     else if(speed < 20 || (down == 1 && up != 1)){
       shiftupQ = 0;
       up = 0;
       down = 0;
       // shiftup = 0;
       // shiftdown = 1;
       ++shiftdownQ;
       state1 = two;
     }
     else{
       state1 = three;
     }
     break;
   case four:
     if(speed >= 40 || (up == 1 && down != 1)){
       shiftdownQ = 0;
       if(accelJump){
         if(demoMode){
           speed +=10;
         }
         shiftupQ += 2; 
         state1 = six;       
       }
       else{
         up = 0;
         down = 0;
         // shiftdown = 0;
         // shiftup = 1;
         ++shiftupQ;
         state1 = five;
       }
     }
     else if(speed < 30 || (down == 1 && up != 1)){
       shiftupQ = 0;
       up = 0;
       down = 0;
       // shiftup = 0;
       // shiftdown = 1;
       ++shiftdownQ;
       state1 = three;
     }
     else{
       state1 = four;
     }
     break;
   case five:
     if(speed >= 50 || (up == 1 && down != 1)){
       shiftdownQ = 0;
       up = 0;
       down = 0;
       // shiftdown = 0;
       // shiftup = 1;
       ++shiftupQ;
       state1 = six;
     }
     else if(speed < 40 || (down == 1 && up != 1)){
       shiftupQ = 0;
       up = 0;
       down = 0;
       shiftup = 0;
       shiftdown = 1;
       ++shiftdownQ;
       state1 = four;
     }
     else{
       state1 = five;
     }
     break;
   case six:
     if(speed < 50 || (down == 1 && up != 1)){
       shiftupQ = 0;
       up = 0;
       down = 0;
       shiftup = 0;
       shiftdown = 1;
       ++shiftdownQ;
       state1 = five;
     }
     else{
       state1 = six;
     }
     break;


   default:
     break;
 }
 switch (state1) {
   case gearstart:
     break;
   case one:
     gear = 1;
     //Serial.println(gear);
     break;
   case two:
     gear = 2;
     //Serial.println(gear);
     break;
   case three:
     gear = 3;
     //Serial.println(gear);
     break;
   case four:
     gear = 4;
     //Serial.println(gear);
     break;
   case five:
     gear = 5;
     //Serial.println(gear);
     break;
   case six:
     gear = 6;
     //Serial.println(gear);
     break;
   default:
     break;
 }
 return state1;
}




int backsteps[8][4] {
 {0,0,0,1},
 {0,0,1,1},
 {0,0,1,0},
 {0,1,1,0},
 {0,1,0,0},
 {1,1,0,0},
 {1,0,0,0},
 {1,0,0,1},
};
int forsteps[8][4] {
 {1,0,0,0},
 {1,1,0,0},
 {0,1,0,0},
 {0,1,1,0},
 {0,0,1,0},
 {0,0,1,1},
 {0,0,0,1},
 {1,0,0,1},
};


int i = 0;
int counter = 0;
int hold = 0;
int sig[4] = {2, 3, 4, 5};
enum SHIFTER_STATES { shiftstart, upshift, downshift, wait};
int SHIFTER_TICK( int state2 ){
 switch(state2){
   case shiftstart:
     state2 = wait;
     break;
   case wait:


     if(shiftupQ > 0/*shiftup == 1 && shiftdown != 1*/ ){
       --shiftupQ;
       hold = 1;
       shiftup = 0;
       shiftdown = 0;
       state2 = upshift;
     }
     else if(shiftdownQ > 0/*shiftup != 1 && shiftdown == 1*/ ){
       --shiftdownQ;
       hold = 1;
       shiftup = 0;
       shiftdown = 0;
       state2 = downshift;
     }
     else{
       hold = 0;
       shiftdownQ = 0;
       shiftupQ = 0;
       state2 = wait;
     }
     break;
   case upshift:
     if(counter < 450){
       if(i < 8){
         ++i;
       }
       else{
         i = 0;
         //state2 = wait;
       }
       ++counter;
       state2 = upshift;
     }
     else{
       counter = 0;
       hold = 0;
       state2 = wait;
     }


     break;
   case downshift:
     if(counter < 450){
       if(i < 8){
         ++i;
       }
       else{
         i = 0;
         //state2 = wait;
       }
       ++counter;
       state2 = downshift;
     }
     else{
       counter = 0;
       hold = 0;
       state2 = wait;
     }
     break;
   default:
     break;
 }


 switch(state2){
   case shiftstart:
     break;
   case upshift:
     for (int j=0; j < 4; j++) {
       if (forsteps[i][j] == 1) {
         digitalWrite(sig[j], HIGH);
       }
       else {
         digitalWrite(sig[j], LOW);
       }
     }
     break;


   case downshift:
     for (int j=0; j < 4; j++) {
       if (backsteps[i][j] == 1) {
         digitalWrite(sig[j], HIGH);
       }
       else {
         digitalWrite(sig[j], LOW);
       }
     }


     break;
   case wait:
     break;
   default:
     break;
 }
 return state2;
}
//int demoMode = 0;
int joyx = 500;
enum INPUT_STATES { inputstart, getinputmanual, waitinput };
int INPUT_TICK( int state3 ){
 switch(state3){
   case inputstart:
     state3 = waitinput;
     break;
   case waitinput:
     if(digitalRead(9) == 0){
       demoMode = 1;
       state3 = getinputmanual;
     }
     else{
       state3 = waitinput;
     }     
     break;   
   case getinputmanual:
     // up = 0;
     // down = 0;
     if(digitalRead(8) == 0){
       demoMode = 0;
       state3 = waitinput;
     }
     else{
       state3 = getinputmanual;
     }
    
     break;
  
  
   default:
     break;
 }


 switch(state3){
   case inputstart:
     break;
   case waitinput:
     Serial.println("WAIT");
     break;


   case getinputmanual:
     Serial.println("MANUAL");
     int tempup = digitalRead(7);
     // Serial.print("UP:");
     // Serial.println(up);
     int tempdown = digitalRead(6);
     up = !tempup;
     down = !tempdown;


     int joy = analogRead(A10);
     joyx = analogRead(A9);
     //Serial.println(joy);    
     if(joy <= 100 && speed > 0){
       speed = speed-1;
     }
     else if(joy >= 900 && speed < 65){
       speed = speed+1;
     }
     break;
  


   default:
     break;
 }
 return state3;
}


//GPS
int tempCounterG = 0;
int satCount = 0;
TinyGPSPlus gps;
enum GPS_STATES { gpsstart, getgps, manualspeed };
int GPS_TICK( int state4 ){
 switch(state4){
   case gpsstart:
     state4 = getgps;
     break;
   case getgps:
     //Serial.print("..");
     //++tempCounterG;
     if(demoMode == 1){
       speed = 0;
       state4 = manualspeed;
     }
     else{
       state4 = getgps;
     }
    
     break;
   case manualspeed:
     if(demoMode == 0){
       speed = 0;
       state4 = getgps;
     }
     else{
       state4 = manualspeed;
     }
     break;
  
   default:
     break;
 }


 switch(state4){
   case gpsstart:
     break;
   case getgps:
     while (ss.available() > 0) {
       if (gps.encode(ss.read())) {
         if (gps.speed.isValid()) {
           speed = gps.speed.mph();
           satCount = gps.satellites.value();
         }
       }
     }
     break;
   case manualspeed:
     while (ss.available() > 0) {
       if (gps.encode(ss.read())) {
         if (gps.location.isValid()) {
           Serial.print("Location: ");
           Serial.print(gps.location.lat(), 6);
           Serial.print(", ");
           Serial.println(gps.location.lng(), 6);
           satCount = gps.satellites.value();
         }
       }
     }


     break;
   default:
     break;
 }
 return state4;
}


enum ACCEL_STATES { accelstart, getaccel, manacc};
int ACCEL_TICK( int state5 ){
 switch(state5){
   case accelstart:
     state5 = getaccel;
     break;
   case getaccel:
     if(demoMode == 0){
       state5 = getaccel;
     }
     else if(demoMode == 1){
       accelJump = 0;
       state5 = manacc;
     }
     break;
   case manacc:
     if(demoMode == 1){
       state5 = manacc;
     }
     else if(demoMode == 0){
       accelJump = 0;
       state5 = getaccel;
     }
     break;
  
   default:
     break;
 }


 switch(state5){
   case accelstart:
     break;
   case getaccel:
     Wire.beginTransmission(0x68);
     Wire.write(0x3B); 
     Wire.endTransmission(false);
     Wire.requestFrom(0x68,12,true); 
     acceleration=Wire.read()<<8|Wire.read();
     //Serial.println(acceleration);  
     if(acceleration > 3500){
       accelJump = 1;
     }
     break;
  
   case manacc:
     Wire.beginTransmission(0x68);
     Wire.write(0x3B); 
     Wire.endTransmission(false);
     Wire.requestFrom(0x68,12,true); 
     acceleration=Wire.read()<<8|Wire.read();
     //Serial.print("accelerometer: ");
     //Serial.println(acceleration);
     if(joyx >= 900){
       accelJump = 1;
     }
     else if(joyx <= 100){
       accelJump = 0;
     }
     Serial.print("DEMO accel: ");
     Serial.println(accelJump);


     break;
  


   default:
     break;
 }
 return state5;
}


enum DISP_STATES { dispstart, display };
int DISP_TICK( int state7 ){
 switch(state7){
   case dispstart:
     lcd.clear();
     state7 = display;
     break;
   case display:
     lcd.clear();
     state7 = display;
     break;
  
  
   default:
     break;
 }


 switch(state7){
   case dispstart:
     break;
   case display:
     lcd.setCursor(0,0);
     lcd.print("Gear:");
     lcd.print(gear);
     lcd.print(" ");
     lcd.print("Acc:");
     lcd.print(acceleration);
     lcd.setCursor(0,1);
     lcd.print("Speed: ");
     lcd.print(speed);
     lcd.print(" ");
     lcd.print("Sat:");
     lcd.print(satCount);
    
     break;
  


   default:
     break;
 }
 return state7;
}



void setup() {


 pinMode(2, OUTPUT);
 pinMode(3, OUTPUT);
 pinMode(4, OUTPUT);
 pinMode(5, OUTPUT);
 pinMode(6, INPUT_PULLUP);
 pinMode(7, INPUT_PULLUP);
 pinMode(8, INPUT_PULLUP);
 pinMode(9, INPUT_PULLUP);


 Wire.begin();
 Wire.beginTransmission(0x68);
 Wire.write(0x6B); 
 Wire.write(0);   
 Wire.endTransmission(true);


 unsigned char i = 0;
 tasks[i].state = gearstart;
 tasks[i].period = 200;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &GEAR_TICK;
 ++i;
 tasks[i].state = shiftstart;
 tasks[i].period = 1;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &SHIFTER_TICK;
 ++i;
 tasks[i].state = inputstart;
 tasks[i].period = 150;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &INPUT_TICK;
 ++i;
 tasks[i].state = gpsstart;
 tasks[i].period = 50;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &GPS_TICK;
 ++i;
 tasks[i].state = dispstart;
 tasks[i].period = 300;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &DISP_TICK;
 ++i;
 tasks[i].state = accelstart;
 tasks[i].period = 200;
 tasks[i].elapsedTime = 0;
 tasks[i].TickFct = &ACCEL_TICK;




 lcd.begin(16, 2);
 //lcd.clear();
 //lcd.setCursor(0,0);
 analogWrite(10,150);
 lcd.print("TESTING");
  Serial.begin(9600);
 ss.begin(9600);
  //Serial1.begin(9600);
 }


void loop() {
 unsigned char i;
 for (i = 0; i < tasksNum; ++i) {
   if ( (millis() - tasks[i].elapsedTime) >= tasks[i].period) {
     tasks[i].state = tasks[i].TickFct(tasks[i].state);
     tasks[i].elapsedTime = millis();
   }
 }
}
