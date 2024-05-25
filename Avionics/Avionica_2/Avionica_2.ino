#include <DHT_U.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <SFE_BMP180.h>
#include <Adafruit_L3GD20.h>
#include <Adafruit_L3GD20_U.h>
#include <LSM303.h>


File file; // file object that is used to read and write data

const int chipSelect = 10;  // used for Arduino
SFE_BMP180 bmp180;
LSM303 compass;

float Po = 1023.25;
float gama = (0.061 / 1000);
float ax = 0;
float ay = 0;
float az = 0;
float pressure;
float H = 0;

//float pitch = 0;
//float yaw = 0;
//float  q0 = 0;
//float  q1 = 0;
//float  q2 = 0;
//float  q3 = 0;

unsigned long tiempo;
unsigned long tiempo_inicial =0 ;  
double P;
double Altura = 0;
double ascensoMaximo = 0;
double descensoMaximo = 0;
double ascensoActual = 0;
double descensoActual = 0;
double deltaAltura = 0;
double alturaBase = 0;

void setup()
{

  Serial.begin(9600);
  pinMode(4,OUTPUT);
  pinMode(3,OUTPUT);

  while (!Serial) {
  }

  if (!SD.begin(chipSelect)) { // Initialize SD card
    Serial.println("Could not initialize SD card."); // if return value is false, something went wrong.
  }
 
  if (SD.exists("file.txt")) { // if "file.txt" exists, fill will be deleted
    Serial.println("File exists.");
    if (SD.remove("file.txt") == true) { //If file is removed, we print a message
      Serial.println("Successfully removed file.");
    } else {
      Serial.println("Could not removed file.");
    }
  }

  
  bmp180.begin(); 
  compass.init();
  compass.enableDefault();;
  bmp180.begin();
  pressure = getPressure();
  
}

void loop()
{

  //----------------------------------------------------------Analisis altura--------------------------------------------------------
 
  P = getPressure(); // Lectura de presión 
    Altura = bmp180.altitude(P, Po);
    tiempo = millis()-tiempo_inicial;

  

    if (alturaBase == 0 ) {
      alturaBase = Altura;
      ascensoMaximo = Altura;
      descensoMaximo = Altura;
    }
    if (Altura > ascensoMaximo) {
      ascensoMaximo = Altura;
    }
    if (Altura < descensoMaximo) {
      descensoMaximo = Altura;
    }
    ascensoActual = Altura - descensoMaximo;
    descensoActual = ascensoMaximo - Altura;
    deltaAltura = ascensoMaximo - descensoMaximo;
//------------------------------------------------------------------------------------------------------------------
    String dataString = "";
    double T; 
    compass.read();
    T = getTemp();
    ax = compass.a.x * (gama); //Aceleracion con conversión
    ay = compass.a.y * (gama);
    az = compass.a.z * (gama);
    float raw = getPressure(); // replace with your method
    pressure = (7 * pressure + raw) / 8;  // or  15*  /16    dividing by a power of 2 works a bit faster.

   
    Serial.print(T);
    Serial.print(",");
    Serial.print(pressure);
    Serial.print(",");
    Serial.print(Altura);
    Serial.print(",");
    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);
    Serial.println();

  dataString += String(T);
  dataString += ",";
  dataString += String(pressure);
  dataString += ",";
  dataString += String(Altura);
  dataString += ",";
  dataString += String(ax);
  dataString += ",";
  dataString += String(ay);
  dataString += ",";
  dataString += String(az);
  dataString += ",";

  File dataFile = SD.open("file.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  }
  
  else {
    Serial.println("error opening file.txt");
  }
    
  delay(1000);

   //ACTIVAR SISTEMA DESPLIEGUE CON IMU 
 if(descensoActual > 1.5 ){
   tone(4,200);
   tone(3,200);    
  
  delay(1000);
 }
}

double getPressure()
{
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = bmp180.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = bmp180.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = bmp180.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = bmp180.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}

double getTemp()
{
  char status;
  double T;
  
  status = bmp180.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);
    
    status = bmp180.getTemperature(T);
    if (status != 0)
    {
     return(T);
        }
        else Serial.println("error retrieving temperature measurement\n");
      }
  else Serial.println("error starting temperature measurement\n");
}
