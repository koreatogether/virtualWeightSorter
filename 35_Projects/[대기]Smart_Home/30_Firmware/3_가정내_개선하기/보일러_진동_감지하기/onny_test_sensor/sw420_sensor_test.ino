/*

board : arduino nano board
sensor : sw420 vibration sensor
onyly reading signal : digital pin 2

*/

int sensorPin = 2; // select the input pin for the sensor
int sensorState = 0; // variable for reading the sensor status

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
    // declare sensor as input:
    pinMode(sensorPin, INPUT);
}

void loop() {
  // read the state of the sensor:
  sensorState = digitalRead(sensorPin);

  // check if the sensor is HIGH:
  if (sensorState == HIGH) {
    // turn LED on:
    Serial.println("sensor is HIGH");
  } else {
    // turn LED off:
    Serial.println("sensor is LOW");
  }
}


