/*
  led_controller.ino (simplified)
  - Simulates an external signal (internal to sketch) to set air quality signal values:
    0=bad, 1=normal, 2=good
  - Controls three LEDs (RED, YELLOW, GREEN) on pins 2,3,4 respectively
  - Assumes common-cathode LEDs (active HIGH)
  - Non-blocking: uses millis() for simulation timing and applies LED changes when
    the signal changes.

  This version removes serial command handling and auto-cycles the signal every
  2 seconds (2000ms). The API function setAirQualitySignal(int) is still available
  for other code/modules to update the signal.

  LED behavior:
    - Signal 0 (bad): RED on, others off
    - Signal 1 (normal): YELLOW on, others off
    - Signal 2 (good): GREEN on, others off
*/

const int PIN_RED = 2;
const int PIN_YELLOW = 3;
const int PIN_GREEN = 4;

// Signal values: -1 unknown/off, 0 bad, 1 normal, 2 good
volatile int airQualitySignal = -1;
int lastAppliedSignal = -2; // different initial value to force first apply

// Simulation (virtual external source)
bool simEnabled = true;
const unsigned long SIM_INTERVAL = 2000UL; // 2 seconds between simulated changes
unsigned long simPrevMillis = 0;
int simIndex = 0; // cycles 0,1,2

void setup()
{
    // Initialize LED pins
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_YELLOW, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);

    // Turn all off initially
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_GREEN, LOW);

    // Start with simulation driving the first state
    simIndex = 0;
    if (simEnabled)
    {
        setAirQualitySignal(simIndex);
    }
}

void loop()
{
    // Simulate incoming internal signal if enabled
    if (simEnabled)
    {
        unsigned long now = millis();
        if (now - simPrevMillis >= SIM_INTERVAL)
        {
            simPrevMillis = now;
            simIndex = (simIndex + 1) % 3; // cycle 0,1,2
            setAirQualitySignal(simIndex);
        }
    }

    // Apply signal if it changed
    if (airQualitySignal != lastAppliedSignal)
    {
        applySignal(airQualitySignal);
        lastAppliedSignal = airQualitySignal;
    }

    // Non-blocking idle; could perform other tasks here
}

// API function other modules can call
void setAirQualitySignal(int s)
{
    if (s < -1 || s > 2)
        return; // ignore invalid
    airQualitySignal = s;
}

// Internal helper: apply the signal to LEDs
void applySignal(int s)
{
    // Turn all off first
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_GREEN, LOW);

    switch (s)
    {
    case 0: // bad -> RED
        digitalWrite(PIN_RED, HIGH);
        break;
    case 1: // normal -> YELLOW
        digitalWrite(PIN_YELLOW, HIGH);
        break;
    case 2: // good -> GREEN
        digitalWrite(PIN_GREEN, HIGH);
        break;
    default:
        // unknown -> all off
        break;
    }
}
