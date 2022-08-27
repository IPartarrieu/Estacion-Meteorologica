#include <DS1307RTC.h>

#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <SPI.h> // para módulo de tarjeta SD
#include <SD.h> // para tarjeta SD
#include <TimeLib.h> // para designar la hora de los datos


//RTC_DS1307 RTC;

File myFile;

LiquidCrystal_I2C lcd(0x27,20,4);
String mensaje = "Primera Estacion APP";
int longitud_texto = mensaje.length();
int posicion;

#include "SparkFunMPL3115A2.h"
#include "SparkFunHTU21D.h"

// pin digital
const byte WSPEED = 3;

// pin analogo
const byte WDIR = A0;

// variables globales
long lastSecond; // el contador millis para ver cuando pasa un segundo
byte seconds; // cuando pasan 60 aumenta un minuto
byte seconds_2m; // mantiene pista de la velocidad del viento y dirección promedio al paso de 2 minutos
byte minutes; 
byte minutes_10m; 

long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

//necesitamos registrar:
//velocidad/Dirección del viento (no almacenado)
//Refaga/dirección persistente en el día (no almacenada)
//velocidad/dirección promedio cada 2 minutos
//velocidad/dirección cada 10 minutos

byte windspdavg[120]; //120 bytes para mantener pista de 2 min de promedio

#define WIND_DIR_AVG_SIZE 120
int winddiravg[WIND_DIR_AVG_SIZE];
float windgust_10m[10];
int windgustdirection_10m[10];

//valores meteorológicos esperados
int winddir = 0;
float windspeedmph = 0;
float windgustmph = 0;
int windgustdir = 0;
float windspdmph_avg2m = 0;
int winddir_avg2m = 0;
float windgustmph_10m = 0;
int windgustdir_10m = 0;

void wspeedIRQ()
// activado por el imán del anemómetro (2 pulsos por rotación)
{
  if (millis() - lastWindIRQ > 10) // ignora fallos de rebote de los interruptores de menos de 10 ms (lectura máxima de 142MPH) tras el cierre del interruptor de láminas
  {
    lastWindIRQ = millis(); //agarra el tiempo actual
    windClicks++; //hay 1.492MPH por cada 1 click por segundo
  }

}


//uint32_t syncProvider()
//{
//  return RTC.now().unixtime();
//}

void setup()
{
//  setTime(15,15,00,10,07,2022);
Serial.begin(9600);
  //RTC.begin()
  //setSyncProvider(syncProvider);
  //if(timeStatus() != timeSet)
  //  Serial.println("Unable to sync with the RTC");
  //else
  //  Serial.println("RTC has set the system time");
  while (!Serial){
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(8)){
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  
  Serial.println("Primera Estación");

  pinMode(WSPEED, INPUT_PULLUP);
  attachInterrupt(1, wspeedIRQ, FALLING);
  interrupts();

  Serial.println("Consultora de Recurso Eólico: APP");

  //display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(mensaje);
  
  lcd.setCursor(0,1);
  lcd.print("D:");
  lcd.setCursor(0,2);
  lcd.print("Dx2:");
  lcd.setCursor(0,3);
  lcd.print("Dr10:");

  lcd.setCursor(10,1);
  lcd.print("R:");
  lcd.setCursor(10,2);
  lcd.print("Rx2:");
  lcd.setCursor(10,3);
  lcd.print("Rr10:");

  while (!Serial) ;
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
    SD.remove("PRUEBA.txt");
File dataFile = SD.open("PRUEBA.txt" , FILE_WRITE);
  if (dataFile) {
    dataFile.print("FECHA");dataFile.print("    ");dataFile.print("HORA");
    dataFile.print("    ");dataFile.print("MAG");dataFile.print("    ");
    dataFile.print("DIR");dataFile.print("    ");dataFile.print("RAF");
    dataFile.print("    ");dataFile.print("DIR_RAF"); dataFile.close();
  }
}
void loop() 
{
  /*if (Serial.available()) {
   time_t t = processSyncMessage();
   if (t != 0){
    RTC.set(t);   // set the RTC and system time to the received value
    setTime(t);
    }
   }
   digitalClockDisplat();
*/ 

tmElements_t tm; //Esto se utiliza para poder programar la hora
  if (RTC.read(tm)) {
    }

  File dataFile = SD.open("PRUEBA.txt" , FILE_WRITE);
  if (dataFile) {
    dataFile.println(" ");
    dataFile.print(tm.Day);dataFile.print("/");dataFile.print(tm.Month);
    dataFile.print("/");dataFile.print(tmYearToCalendar(tm.Year));dataFile.print("    ");
    dataFile.print(tm.Hour);dataFile.print(":");dataFile.print(tm.Minute);
    dataFile.print(":");dataFile.print(tm.Second);dataFile.print("    ");
    dataFile.print(windspdmph_avg2m);dataFile.print("    ");dataFile.print(winddir_avg2m);dataFile.print("    ");
    dataFile.print(windgustmph);dataFile.print("    ");dataFile.print(windgustdir);
    dataFile.close();
 }  else {
    // Serial.println("ERROR");
  }

dataFile = SD.open("PRUEBA.txt");
  if (dataFile) {
    //Serial.println("PRUEBA.txt:");
    //read from the file until there's nothing else in it:
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    //close the file:
    dataFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening 1.txt");
  }
  Serial.println("");



  
  lcd.setCursor(2,1);
  lcd.print(winddir);
  lcd.setCursor(4,2);
  lcd.print(winddir_avg2m);
  lcd.setCursor(5,3);
  lcd.print(windgustdir_10m);
  
  lcd.setCursor(12,1);
  lcd.print(windspeedmph);
  lcd.setCursor(14,2);
  lcd.print(windspdmph_avg2m);
  lcd.setCursor(15,3);
  lcd.print(windgustmph_10m);
  
  delay(500);

  
  lcd.setCursor(2,1);
  lcd.print("   ");
  lcd.setCursor(4,2);
  lcd.print("   ");
  lcd.setCursor(5,3);
  lcd.print("   ");

  lcd.setCursor(12,1);
  lcd.print("     ");
  lcd.setCursor(14,2);
  lcd.print("     ");
  lcd.setCursor(15,3);
  lcd.print("     ");
  delay(500);
  

  
 if(millis() - lastSecond >= 1000)
 {
  lastSecond += 1000;
  
  // toma una velocidad y dirección media cada segundo por 2 min
  if(++seconds_2m > 119) seconds_2m = 0;
  
  float currentSpeed = get_wind_speed();
  windspeedmph = currentSpeed;
  int currentDirection = get_wind_direction();
  windspdavg[seconds_2m] = (int)currentSpeed;
  winddiravg[seconds_2m] = currentDirection;

  if(currentSpeed > windgust_10m[minutes_10m])
  {
    windgust_10m[minutes_10m] = currentSpeed;
    windgustdirection_10m[minutes_10m] = currentDirection;
  }

  if(currentSpeed > windgustmph)
  {
    windgustmph = currentSpeed;
    windgustdir = currentDirection;
  }

  if(++seconds > 59)
  {
    seconds = 0;

    if(++minutes > 59) minutes = 0;
    if(++minutes_10m > 9) minutes_10m = 0;

    windgust_10m[minutes_10m] = 0;
  }
  printWeather();
 }
 delay(100);
}

void calcWeather()
{
  winddir = get_wind_direction();

    //Calc windspeed
    //windspeedmph = get_wind_speed(); //This is calculated in the main loop on line 179

    //Calc windgustmph
    //Calc windgustdir
    //These are calculated in the main loop

  float temp = 0;
  for(int i = 0 ; i < 120 ; i++)
    temp += windspdavg[i];
  temp /= 120.0;
  windspdmph_avg2m = temp;

  //Calc winddir_avg2m, Wind Direction
  //You can't just take the average. Google "mean of circular quantities" for more info
  //We will use the Mitsuta method because it doesn't require trig functions
  //And because it sounds cool.
  //Based on: http://abelian.org/vlf/bearings.html
  //Based on: http://stackoverflow.com/questions/1813483/averaging-angles-again

  long sum = winddiravg[0];
  int D = winddiravg[0];
  for(int i = 1 ; i < WIND_DIR_AVG_SIZE ; i++)
  {
    int delta = winddiravg[i] - D;

    if(delta < -180)
      D += delta + 360;
    else if(delta > 180)
      D += delta -360;
    else
      D += delta;
    sum += D;
     
  }
  winddir_avg2m = sum /WIND_DIR_AVG_SIZE;
  if(winddir_avg2m >= 360) winddir_avg2m -= 360;
  if(winddir_avg2m < 0) winddir_avg2m += 360;

  //buscamos el mayor valor de rafaga en los últimos de 10 min
  windgustmph_10m = 0;
  windgustdir_10m = 0;

  //paso cada 10 min
  for(int i = 0; i < 10 ; i++)
  {
    if(windgust_10m[i] > windgustmph_10m)
    {
      windgustmph_10m = windgust_10m[i];
      windgustdir_10m = windgustdirection_10m[i];
    }
  } 
}

//retorna rapidez instantanea de viento
float get_wind_speed()
{
  float deltaTime = millis() - lastWindCheck;
  deltaTime /= 1000.0;

  float windSpeed = (float)windClicks / deltaTime;
  windClicks = 0; //reseteo y nuevas mediciones
  lastWindCheck = millis();

  windSpeed *= 1.492;
  windSpeed /= 2.237;

  /* Serial.println();
     Serial.print("Windspeed:");
     Serial.println(windSpeed);*/

  return(windSpeed);
  
}

//leemos la dirección del viento, retornamos en grados
int get_wind_direction()
{
  unsigned int adc;
  adc = analogRead(WDIR);

  if (adc < 380) return (113);
  if (adc < 393) return (68);
  if (adc < 414) return (90);
  if (adc < 456) return (158);
  if (adc < 508) return (135);
  if (adc < 551) return (203);
  if (adc < 615) return (180);
  if (adc < 680) return (23);
  if (adc < 746) return (45);
  if (adc < 801) return (248);
  if (adc < 833) return (225);
  if (adc < 878) return (338);
  if (adc < 913) return (0);
  if (adc < 940) return (293);
  if (adc < 967) return (315);
  if (adc < 990) return (270);
  return (-1); // error, disconnected?
  
}

void printWeather()
{
  calcWeather();

  Serial.println();
  Serial.print("Dir[º] = ");
  Serial.print(winddir);
  Serial.print(" | Int[m/s] = ");
  Serial.print(windspeedmph, 1);
  Serial.print(" | Dir_r[º] = ");
  Serial.print(windgustdir);
  Serial.print(" | Int_r[m/s] = ");
  Serial.print(windgustmph, 1);
  Serial.print(" | Dir_x(2 min)[º] = ");
  Serial.print(winddir_avg2m);
  Serial.print(" | Int_x(2 min)[m/s] = ");
  Serial.print(windspdmph_avg2m, 1);
  Serial.print(" | Dir_r(10 min)[º] = ");
  Serial.print(windgustdir_10m);
  Serial.print(" | Int_r(10 min)[m/s] = ");
  Serial.println(windgustmph_10m, 1);
  
}

#define TIME_HEADER "T"

unsigned long processSyncMessage(){
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // 01/01/2013

  if(Serial.find(TIME_HEADER)){
    pctime = Serial.parseInt();
    return pctime;
    if(pctime < DEFAULT_TIME){
      pctime = 0L;
    }
  }
  return pctime;
}
