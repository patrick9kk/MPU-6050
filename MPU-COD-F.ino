//
//  ESP 32 + SD + MPU
 
//               ______________ 
//            EN|              |23     
//            36|              |22 MPU (22)
//            39|              |1 
//            34|   ________   |3 
//            35|  |        |  |21 MPU (SDA)
//            32|  |        |  |19 SD (MISO)
//            33|  |        |  |18 
//            25|  |        |  |5 SD (SCK)
//            26|  |        |  |17 
//            27|  |        |  |16 
//            14|  |        |  |4 SD (SC)
//            12|  |        |  |2 
//   (MOSI)SD 13|  |        |  |15
//           GND|  |_  USB _|  |GND SD (GND) MPU (GND)
//           vin|______________|3.3V SD (VCC) MPU(VCC)

#include <SPI.h>
#include <mySD.h>
#include <TinyGPS++.h>
//#include "heltec.h"
//MPU
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// definir um nome para manipilar melhor 
Adafruit_MPU6050 mpu;

#define MICROSD_PIN_CHIP_SELECT   4
#define MICROSD_PIN_MOSI          13
#define MICROSD_PIN_MISO          19
#define MICROSD_PIN_SCK           5

#define BAND    915E6

#define LOG_FILE_PREFIX "gpslog" // Nome do arquivo.
#define MAX_LOG_FILES 100 // Número máximo de gpslog que pode ser feito
#define LOG_FILE_SUFFIX "csv" // Sufixo do arquivo
char logFileName[13]; // Char string para o nome do arquivo
// Data to be logged:
#define LOG_COLUMN_COUNT 7
char * log_col_names[LOG_COLUMN_COUNT] = {
  "acelX", "acelY", "acelZ", "gyroX", "gyroY", "gyroZ", "temp"
};



void setup()
{
  Serial.begin(115200);
  //Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, false /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  //Heltec.begin(false, true, false, true, BAND);
  

  //SD
  Serial.println("Configurando SD Card");
  if (!SD.begin(MICROSD_PIN_CHIP_SELECT, MICROSD_PIN_MOSI, MICROSD_PIN_MISO, MICROSD_PIN_SCK)) {
    Serial.println("Falha na inicialização");
  }

  updateFileName(); // Toda vez que inicia, cria um novo arquivo gpslog(x+1).csv
  printHeader(); // Coloca o cabeçalho no arquivo novo
  // --- Fim da inicialização do SD ---

  //MPU
  Serial.println("Adafruit MPU6050 test!");

  // inicializar MPU 
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("MPU6050 Found!");
  
  // faixa de medição do acelerômetro
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  // faixa de medição do giroscópio
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  // faixa de medição do giroscópio 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  //Fim da configuração do MPU
}

void loop()
{
  logGPSData();
}

byte logGPSData()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Abre o arquivo

  //MPU
  /* Modo que vai ser visualizado o dados */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  //Configuração do acelerômetro
  
  
  if (logFile)
  { 
    logFile.print(a.acceleration.x);
    logFile.print('\t');
    logFile.print(a.acceleration.y);
    logFile.print('\t');
    logFile.print(a.acceleration.z);
    logFile.print('\t');
    logFile.print(g.gyro.x);
    logFile.print('\t');
    logFile.print(g.gyro.y);
    logFile.print('\t');
    logFile.print(g.gyro.z);
    logFile.print('\t');
    logFile.print(temp.temperature);
    Serial.println(a.acceleration.x);
    Serial.println(a.acceleration.y);
    Serial.println(a.acceleration.z);
    Serial.println(g.gyro.x);
    Serial.println(g.gyro.y);
    Serial.println(g.gyro.z);
    Serial.println(temp.temperature);
    Serial.print('\n');
    logFile.println();
    logFile.close();
 
    return 1; // Retorna em caso de true
  }
 
  return 0; // Se falhar em abrir o arquivo, retorna false
}

// printHeader() - coloca o cabeçalho no arquivo
void printHeader()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file
 
  if (logFile) // If the log file opened, print our column names to the file
  {
    int i = 0;
    for (; i < LOG_COLUMN_COUNT; i++)
    {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // If it's anything but the last column
        logFile.print('\t'); // print a comma
      else // If it's the last column
        logFile.println(); // print a new line
    }
    logFile.close(); // close the file
  }
}
 
// updateFileName() - Procura os nomes dos arquivos no SD Card e cria um novo incrementando +1 no nome;
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // Limpa logFileName string
    // Set logFileName to "gpslogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // Se o arquivo não existe
    {
      break; // Quebra o loop ao achar o index
    }
    else // Caso contrário...
    {
      Serial.print(logFileName);
      Serial.println(" existe"); // Print a debug statement
    }
  }
  Serial.print("Nome do arquivo: ");
  Serial.println(logFileName); // Debug print the file name
}
