//Developer: Madhukant
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define BUFF_SIZE 40
char buffer[BUFF_SIZE];
char data[BUFF_SIZE];
// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     48
#define TFT_RST    47  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     49

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <EEPROM.h>

#include <Servo.h>

Servo servo;

#include <motor.h>

Motor left(29,28,44);
Motor right(30,31,45);

  int meanspeed=190;
  int leftspeed=meanspeed;
  int rightspeed=meanspeed;

#define pi 3.14

#define trigPin 3
#define echoPin 2

void setup()
{
  Serial.begin(57600);
  Serial3.begin(57600);
  delay(500);
  Wire.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo.attach(7);
  servo.write(90);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setCursor(0,0);
  tft.setTextColor(ST7735_WHITE,ST7735_BLACK);
  
  left.stopMotor();
  right.stopMotor();
}
float givendir=0.00;
int dist()
{
  long duration, distance=70;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
    if(distance>70)
    return 70;
  return distance;
}
int distance=0;
int mindistance=20;
//****************************new mpu functions*************************************
int start=0;
float convert(char data[],int n){
  int sign=0,start=0;
  float number=0.00;
  int i;
  if(data[0]=='0')
    sign=0;
  else if(data[0]=='-')
    sign=-1;
  else sign =1;
  if(sign==-1){
      start=1;
  }
  else if(sign==1){
     start=0;
  }
  int part=0;
  int psize=0;
  int factor=1;
   while(data[psize]!='.'){
        psize++;
      }
    
   for(i=psize-1;i>=start;i--){
       part=part+factor*(data[i]-48);
       factor*=10;
   }
   number=part;
   //printf("%f",number);
   start=psize+1;
   psize++;
   float dec=0.0;
   float fac=0.1;
   for(i=psize;i<=n;i++){
       dec=dec+fac*(data[i]-48);
       fac/=10;
   }
   number=number+dec;
   return sign*number;
}
float imuread;
float MPUread(){
  while(1){
int i = 0,k=0;
while(Serial3.available()) {
buffer[i++] = Serial3.read();
data[i-1]=buffer[i-1];
//Serial.print("#");
if(buffer[i-1]==',')
start=0;
//if(((buffer[i-1]<='9'&&buffer[i-1]>='0')||buffer[i-1]=='.'||buffer[i-1]=='-'||buffer[i-1]=='\n')&&i>18)
if(start==1)
{ data[k]=buffer[i-1];
  k++;
}
//Serial.print(buffer[i-1]);
if(buffer[i-1]=='=')
start=1;

if(i == BUFF_SIZE) break;
}
for(i=0;i<k;i++){
  //Serial.print(data[i]);
}
imuread=convert(data,k);
/*if(imuread>=3.14||imuread<=-3.14)
  return MPUread();*/
return imuread;
}
}
//****************************angle function**************************************
int arr[100],n=25,angle;
int flag=0;

float getangle(int arr[],int n,float botdir){  
  float curbotdir=MPUread()*pi/180.0; 
  if(curbotdir>3.14)
    return getangle(arr,n,botdir);
  float rad=0.00;
  int groups[5]={0,0,0,0,0};
  int i,k=0;
  int maxdist=0,maxi=0;
  for(i=0;i<5;i++)
  {
    for(k=0;k<5;k++)
    {
      groups[i]=groups[i]+arr[k+i*5];
    }
    if (groups[i]>maxdist)
    {
      maxdist=groups[i];
      maxi=i;
    }
  }
  tft.setCursor(0,80);
  tft.print(arr[0]);
  tft.print(" ");
  for(i=1;i<n;i++){
    tft.print(arr[i]);
    tft.print(" ");
 }  
 float mpuread=MPUread();
   if(arr[12]>20&&arr[13]>20&&arr[11]>20)
    rad=mpuread*pi/180.0;
  else  
    { if(((mpuread+(2-maxi)*30)*pi/180)>pi)
        rad =pi-(mpuread-180+(2-maxi)*30)*pi/180;
      else if(((mpuread+(2-maxi)*30)*pi/180)<-3.14)
        rad=((mpuread+180+(2-maxi)*30)*pi/180+pi);  
      else 
        rad=(mpuread+(2-maxi)*30)*pi/180;
      
    }
    
  return rad;  
}
//***********************angle function end**************

void loop()
{  
    left.moveMotor(leftspeed);
    right.moveMotor(rightspeed);
    tft.setCursor(60,0);
    tft.print("LS - ");
    tft.print(leftspeed);
    tft.print("RS - ");
    tft.print(rightspeed);
  //if (MPU.read()) { 
   // float botdir=MPU.m_dmpEulerPose[2];
   float botdir=MPUread()*pi/180.0;
    tft.setCursor(0,0);
    //Serial.println(botdir);
    tft.print(botdir); //Serial.println(","); //direction +pi =oppposite pins toward south = 0.0 
    //Serial.println();
    if(abs(botdir-givendir)<=0.2)
    {
      leftspeed=meanspeed;
      rightspeed=meanspeed;
    }
    else
    {
      /*
       * for -ve turn left i.e. bot is in right direction
       * for +ve turn right i.e. bot is in left direction
       */
       if((botdir-givendir)<0&&abs(botdir-givendir)>0.2)  //turn left
       {
          leftspeed=meanspeed+30;
          rightspeed=meanspeed-30;
       }
       else if((botdir-givendir)>0&&abs(botdir-givendir)>0.2)                     //turn right
       {

          leftspeed=meanspeed-30;
          rightspeed=meanspeed+30;
       }
       
    }
    A:
    distance=dist();
    delay(15);
    if(abs(dist()-distance)<3)
      { distance=dist();
        tft.setCursor(80,60);
        tft.print("0");
      }
    else 
      { tft.setCursor(80,60);
        tft.print("1");
        goto A; 
      } 
    tft.setCursor(50,30);
    tft.print("#distance- ");
    tft.print(distance);
    if(distance<=mindistance){
      left.stopMotor();
      right.stopMotor();
  // if(flag==0){
  int i=0;
  if(dist()>20)
    i=6;
  angle=20;
 // if(i!=6)
  for(i=0;i<n;i++){
    servo.write(angle);
    delay(100);
    arr[i]=dist();
    //angle=angle+(120/n);
    angle=angle+6;
      //Serial.print(arr[i]);
      //Serial.print(" ");
  }
  servo.write(90);
 float rad;
    rad=getangle(arr,n,botdir);
    //*******************************************************
     while(1){
       imuread=MPUread()*pi/180.0;
       tft.setCursor(0,0);
    tft.print(imuread);
    if(abs(imuread-(rad))<=0.1){
   left.stopMotor();
   right.stopMotor();
   //return;
   break;
    }
    int reverse =0;
      if((imuread-rad)>pi/2||(imuread-rad)<(-1*pi/2))
        reverse=1;
      else if((imuread-(rad))>0&&abs(imuread-(rad))>=0.1&&reverse==0){      //turn left
         right.moveMotor(100);
         left.moveMotor(-100);
         delay(70);
         right.stopMotor();
         left.stopMotor();
         tft.setCursor(0,50);
         tft.print(rad);
      }
      else if(abs(imuread-(rad))>=0.1){           //turn right
        left.moveMotor(100);
        right.moveMotor(-100);
         delay(70);
         left.stopMotor();
         right.stopMotor();
         tft.setCursor(0,50);
         tft.print(rad);
      }

  //}
  
  }
 /* left.stopMotor();
  right.stopMotor();*/
    //*******************************************************
    }
    else 
    {
      servo.write(90);
    }
  
  //*****************************************
 
 
  
}
