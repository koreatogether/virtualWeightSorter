#include <MQUnifiedsensor.h>

// Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define pin A0                // Analog input 0 of your arduino
#define type "MQ-7"           // MQ7
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ7CleanAir 27.5 // RS / R0 = 27.5 ppm

// Declare Sensor
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup()
{
    Serial.begin(9600);
    MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
    MQ7.setA(99.042);           // Configure the equation to calculate CO concentration
    MQ7.setB(-1.518);           // Configure the equation to calculate CO concentration
    MQ7.init();

    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++)
    {
        MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += MQ7.calibrate(RatioMQ7CleanAir);
        Serial.print(".");
    }
    MQ7.setR0(calcR0 / 10);
    Serial.println("  done!");

    if (isinf(calcR0))
    {
        Serial.println("Warning: Connection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
        while (1)
            ;
    }
    if (calcR0 == 0)
    {
        Serial.println("Warning: Connection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
        while (1)
            ;
    }
    MQ7.serialDebug(true);
}

void loop()
{
    MQ7.update();      // Update data, the arduino will read the voltage from the analog pin
    MQ7.readSensor();  // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
    MQ7.serialDebug(); // Will print the table on the serial port
    delay(1000);
}
