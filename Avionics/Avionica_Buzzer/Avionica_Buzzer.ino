#include <Adafruit_BMP085.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <SFE_BMP180.h>
#include <Adafruit_L3GD20.h>
#include <Adafruit_L3GD20_U.h>
#include <LSM303.h>

Adafruit_BMP085 bmp;

File file; // file object that is used to read and write data
//const int chipSelect = D8;  // used for ESP8266

const int chipSelect = 10;  // used for Arduino  

LSM303 compass;
char report[80];

float beta = (0.160/1000);  //factor de conversion para magnetómetro. Retorna datos en gauss
float gamma = (0.061/1000); //factor de coneversión aceleracion (AUN POR CONFIRMAR). Rretorna datos en g 
float ax = 0;
float ay = 0;
float az = 0;

SFE_BMP180 pressure; //Altura relativa
unsigned long tiempo;
unsigned long tiempo_inicial =0 ;  
double baseline; // baseline pressure
double P;
double Altura = 0;
double Altura_raw;
double valorFiltrado;
double Temperatura = 0 ;
double ascensoMaximo = 0;
double descensoMaximo = 0;
double ascensoActual = 0;
double descensoActual = 0;
double deltaAltura = 0;
double alturaBase = 0;
bool activoAltura = true;

void setup()
{

 Serial.begin(9600);
 Wire.begin();
 //Inicialización sensores
 bmp.begin();
 compass.init();
 pressure.begin();
 baseline = getPressure();

  pinMode(4,OUTPUT);
  pinMode(3,OUTPUT);
 
//SD Recorder
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

  
}

void loop()
{
    String dataString = "";
    
    //----------------------------------------------------------Analisis altura--------------------------------------------------------
 
  P = getPressure(); // Lectura de presión 
    Altura = pressure.altitude(P, baseline); // altura
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

 //------------------------------------------------------Aceleraciones---------------------------------------------------------------
    
  compass.read();
  ax = compass.a.x * (gamma); //Aceleracion con conversión
  ay = compass.a.y * (gamma);
  az = compass.a.z * (gamma);

   
    Serial.print(bmp.readTemperature());
    Serial.print(",");
    Serial.print(bmp.readPressure());
    Serial.print(",");
    Serial.print(bmp.readAltitude(101500));
    Serial.print(",");
    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);
    Serial.println();

  dataString += String(bmp.readTemperature());
  dataString += ",";
  dataString += String(bmp.readPressure());
  dataString += ",";
  dataString += String(bmp.readAltitude(101500));
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
 if(descensoActual > 1.5 && activoAltura){
   tone(4,200);
   tone(3,200);    
  
  delay(1000);
  
  activoAltura = false;
  }
}

//baseline
double getPressure()
{
  char status;
  double T, P, p0, a;

  // You must first get a temperature measurement to perform a pressure reading.

  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);


    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
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

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          return (P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
