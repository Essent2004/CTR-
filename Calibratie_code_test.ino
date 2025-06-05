// ===========================
// CONFIGURATIE & CONSTANTEN
// ===========================

// Stepper motor pin configuratie - Spindels
const int Stepper_spindel1 = 30, DIR1 = 31;
const int Stepper_spindel2 = 40, DIR2 = 41;
const int Stepper_spindel3 = 50, DIR3 = 51;

// Stepper motor pin configuratie - Tubes
const int Stepper_tube1 = 32, DIR_tube1 = 33;
const int Stepper_tube2 = 43, DIR_tube2 = 42;
const int Stepper_tube3 = 52, DIR_tube3 = 53;

// Limietschakelaars
const int LIMIT1 = 2;
const int LIMIT2 = 3;
const int LIMIT3 = 4;
const int LIMIT4 = 5;

// LED pinnen (RGB LED)
const int LED_RED     = 8;
const int LED_GREEN   = 7;
const int LED_RED2    = 11;
const int LED_GREEN2  = 10;
const int LED_BLUE    = 6;
const int LED_BLUE2   = 9;

// Knoppen
const int BUTTON_KALIBRATIE = 12;
const int BUTTON_DEMO = 13;

// Motorstatus
bool stepper1Running  = false;
bool stepper2Running  = false;
bool stepper3Running  = false;
bool tube1Running     = false;
bool tube2Running     = false;
bool tube3Running     = false;
bool calibrating      = false;
bool demoActief       = false;
int demoFase          = 0;

// Kalibratie geheugen
bool prevLimit2 = false, prevLimit3 = false, prevLimit4 = false;
bool prevButtonKalibratie = HIGH;
bool prevButtonDemo = HIGH;

// Timing & pulsen
unsigned long previousMicros1 = 0, previousMicros2 = 0, previousMicros3 = 0;
unsigned long previousMicrosTube1 = 0, previousMicrosTube2 = 0, previousMicrosTube3 = 0;
unsigned long lastLedToggle = 0;

const int PULSE_INTERVAL = 100;                // Pulsinterval voor spindels
const unsigned long TUBE_PULSE_INTERVAL = 10000; // Pulsinterval voor tubes (langzamer)
const int LED_BLINK_INTERVAL = 500;           // LED knipperfrequentie

// Richtinginstellingen (motorrichting)
#define SPINDEL1_DIR_REVERSED true
#define SPINDEL2_DIR_REVERSED true
#define SPINDEL3_DIR_REVERSED true

// Demo tubes
unsigned long demoTimer = 0;
int tubePhase = 0;
bool tubeDirections[3] = {true, true, true};

// ===========================
// SETUP FUNCTIE
// ===========================
void setup() {
  // Instellen van alle pinnen
  pinMode(Stepper_spindel1, OUTPUT); pinMode(DIR1, OUTPUT);
  pinMode(Stepper_spindel2, OUTPUT); pinMode(DIR2, OUTPUT);
  pinMode(Stepper_spindel3, OUTPUT); pinMode(DIR3, OUTPUT);

  pinMode(Stepper_tube1, OUTPUT); pinMode(DIR_tube1, OUTPUT);
  pinMode(Stepper_tube2, OUTPUT); pinMode(DIR_tube2, OUTPUT);
  pinMode(Stepper_tube3, OUTPUT); pinMode(DIR_tube3, OUTPUT);

  pinMode(LIMIT1, INPUT_PULLUP);
  pinMode(LIMIT2, INPUT_PULLUP);
  pinMode(LIMIT3, INPUT_PULLUP);
  pinMode(LIMIT4, INPUT_PULLUP);

  pinMode(LED_RED, OUTPUT); pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED2, OUTPUT); pinMode(LED_GREEN2, OUTPUT);
  pinMode(LED_BLUE2, OUTPUT); pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_KALIBRATIE, INPUT_PULLUP);
  pinMode(BUTTON_DEMO, INPUT_PULLUP);

  // Initialiseer LEDs
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED2, LOW);
  digitalWrite(LED_GREEN2, HIGH);
  digitalWrite(LED_BLUE2, LOW);
  digitalWrite(LED_BLUE, LOW);

  // Initieer seriële communicatie
  Serial.begin(9600);
  Serial.println("Commando's: 1f/1r/1 | 2f/2r/2 | 3f/3r/3 | 4 Kalibratie | 5f/5r/5 | 6f/6r/6 | 7f/7r/7 | 0 Stop");
}

// ===========================
// HOOFDFUNCTIES
// ===========================
void loop() {
  unsigned long currentMicros = micros();
  unsigned long currentMillis = millis();

  // LEDs laten knipperen tijdens kalibratie/demo
  if (calibrating && currentMillis - lastLedToggle >= LED_BLINK_INTERVAL) {
    digitalWrite(LED_RED, !digitalRead(LED_RED));
    digitalWrite(LED_GREEN, LOW);
    lastLedToggle = currentMillis;
  }

  if (demoActief && currentMillis - lastLedToggle >= LED_BLINK_INTERVAL) {
    digitalWrite(LED_BLUE2, !digitalRead(LED_BLUE2));
    digitalWrite(LED_GREEN2, LOW);
    digitalWrite(LED_RED2, LOW);
    lastLedToggle = currentMillis;
  }

  // Kalibratie knop indrukken
  bool btnKal = digitalRead(BUTTON_KALIBRATIE);
  if (prevButtonKalibratie == HIGH && btnKal == LOW) {
  if (calibrating) {
    stopAllMotors(); // ook demo wordt dan gestopt
    Serial.println("Kalibratie onderbroken!");
  } else {
    startCalibration();
  }
}

  prevButtonKalibratie = btnKal;

  // Demo knop indrukken
  bool btnDemo = digitalRead(BUTTON_DEMO);
  if (prevButtonDemo == HIGH && btnDemo == LOW) {
    if (!calibrating && !demoActief) {
      demoActief = true;
      startCalibration(); // Demo begint met kalibratie
    } else {
      stopAllMotors();
    }
  }
  prevButtonDemo = btnDemo;

// Seriële input verwerken
if (Serial.available() > 0) {
  String input = Serial.readStringUntil('\n');
  input.trim();

  // STOP commando altijd uitvoeren, ook tijdens kalibratie/demo
  if (input == "0") {
    stopAllMotors();
    return; // Stop verdere verwerking van dit commando
  }

  // Andere commando's alleen als geen kalibratie of demo actief is
  if (!calibrating && !demoActief) {
    if (input == "1f") startSpindelMotor(1, true);
    else if (input == "1r") startSpindelMotor(1, false);
    else if (input == "1") stepper1Running = false;

    else if (input == "2f") startSpindelMotor(2, true);
    else if (input == "2r") startSpindelMotor(2, false);
    else if (input == "2") stepper2Running = false;

    else if (input == "3f") startSpindelMotor(3, true);
    else if (input == "3r") startSpindelMotor(3, false);
    else if (input == "3") stepper3Running = false;

    else if (input == "5f") startTubeMotor(1, true);
    else if (input == "5r") startTubeMotor(1, false);
    else if (input == "5") tube1Running = false;

    else if (input == "6f") startTubeMotor(2, true);
    else if (input == "6r") startTubeMotor(2, false);
    else if (input == "6") tube2Running = false;

    else if (input == "7f") startTubeMotor(3, true);
    else if (input == "7r") startTubeMotor(3, false);
    else if (input == "7") tube3Running = false;

    else if (input == "4") startCalibration();
  }
}
  // Controleer limieten
  checkSpindelLimits();

  // Controleer kalibratie
  if (calibrating) checkCalibration();

  // DEMO logica
  if (demoActief) {
    // Gecontroleerde overgang tussen spindels en tubes
    if (demoFase == 1 && digitalRead(LIMIT1) == HIGH) {
      stepper1Running = false;
      stepper2Running = true;
      digitalWrite(DIR2, SPINDEL2_DIR_REVERSED ? LOW : HIGH);
      demoFase = 2;
    } else if (demoFase == 2 && digitalRead(LIMIT2) == HIGH) {
      stepper2Running = false;
      stepper3Running = true;
      digitalWrite(DIR3, SPINDEL3_DIR_REVERSED ? LOW : HIGH);
      demoFase = 3;
    } else if (demoFase == 3 && digitalRead(LIMIT3) == HIGH) {
      stepper3Running = false;
      demoFase = 4;
      demoTimer = millis();
      tube1Running = tube2Running = tube3Running = true;
      digitalWrite(DIR_tube1, HIGH);
      digitalWrite(DIR_tube2, HIGH);
      digitalWrite(DIR_tube3, HIGH);
    } else if (demoFase == 4) {
      if (currentMillis - demoTimer >= 30000) {
        int current = tubePhase % 3;
        tubeDirections[current] = !tubeDirections[current];
        digitalWrite(
          current == 0 ? DIR_tube1 : current == 1 ? DIR_tube2 : DIR_tube3,
          tubeDirections[current] ? HIGH : LOW
        );
        tubePhase++;
        demoTimer = currentMillis;
      }
    }
  }

  // Pulsen voor draaiende motoren
  if (stepper1Running && currentMicros - previousMicros1 >= PULSE_INTERVAL)
    { digitalWrite(Stepper_spindel1, !digitalRead(Stepper_spindel1)); previousMicros1 = currentMicros; }

  if (stepper2Running && currentMicros - previousMicros2 >= PULSE_INTERVAL)
    { digitalWrite(Stepper_spindel2, !digitalRead(Stepper_spindel2)); previousMicros2 = currentMicros; }

  if (stepper3Running && currentMicros - previousMicros3 >= PULSE_INTERVAL)
    { digitalWrite(Stepper_spindel3, !digitalRead(Stepper_spindel3)); previousMicros3 = currentMicros; }

  if (tube1Running && currentMicros - previousMicrosTube1 >= TUBE_PULSE_INTERVAL)
    { digitalWrite(Stepper_tube1, !digitalRead(Stepper_tube1)); previousMicrosTube1 = currentMicros; }

  if (tube2Running && currentMicros - previousMicrosTube2 >= TUBE_PULSE_INTERVAL)
    { digitalWrite(Stepper_tube2, !digitalRead(Stepper_tube2)); previousMicrosTube2 = currentMicros; }

  if (tube3Running && currentMicros - previousMicrosTube3 >= TUBE_PULSE_INTERVAL)
    { digitalWrite(Stepper_tube3, !digitalRead(Stepper_tube3)); previousMicrosTube3 = currentMicros; }
}

// ===========================
// FUNCTIES
// ===========================

// Stop alle motoren en reset status
void stopAllMotors() {
  stepper1Running = stepper2Running = stepper3Running = false;
  tube1Running = tube2Running = tube3Running = false;
  calibrating = demoActief = false;
  demoFase = tubePhase = 0;

  digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED2, LOW); digitalWrite(LED_GREEN2, HIGH);

  Serial.println("Alle motoren gestopt");
}

// Start een spindel in juiste richting
void startSpindelMotor(int motor, bool forward) {
  bool dir = forward ? (SPINDEL1_DIR_REVERSED ? LOW : HIGH) : (SPINDEL1_DIR_REVERSED ? HIGH : LOW);
  if (motor == 1) { stepper1Running = true; digitalWrite(DIR1, dir); }
  if (motor == 2) { stepper2Running = true; digitalWrite(DIR2, dir); }
  if (motor == 3) { stepper3Running = true; digitalWrite(DIR3, dir); }
}

// Start een tube in gewenste richting
void startTubeMotor(int tube, bool forward) {
  if (tube == 1) { tube1Running = true; digitalWrite(DIR_tube1, forward ? HIGH : LOW); }
  if (tube == 2) { tube2Running = true; digitalWrite(DIR_tube2, forward ? HIGH : LOW); }
  if (tube == 3) { tube3Running = true; digitalWrite(DIR_tube3, forward ? HIGH : LOW); }
}

// Beëindigt motor bij bereiken van limiet
void checkSpindelLimits() {
  if (!calibrating && !demoActief) {
    if (stepper1Running && digitalRead(LIMIT1) == HIGH) stepper1Running = false;
    if (stepper2Running && digitalRead(LIMIT2) == HIGH) stepper2Running = false;
    if (stepper3Running && digitalRead(LIMIT3) == HIGH) stepper3Running = false;
  }
}

// Start kalibratieproces
void startCalibration() {
  calibrating = true;
  stepper1Running = stepper2Running = stepper3Running = true;

  digitalWrite(DIR1, SPINDEL1_DIR_REVERSED ? HIGH : LOW);
  digitalWrite(DIR2, SPINDEL2_DIR_REVERSED ? HIGH : LOW);
  digitalWrite(DIR3, SPINDEL3_DIR_REVERSED ? HIGH : LOW);

  prevLimit2 = digitalRead(LIMIT2);
  prevLimit3 = digitalRead(LIMIT3);
  prevLimit4 = digitalRead(LIMIT4);

  Serial.println("Kalibratie gestart...");
}

// Voert controle op limieten uit tijdens kalibratie
void checkCalibration() {
  bool l2 = digitalRead(LIMIT2), l3 = digitalRead(LIMIT3), l4 = digitalRead(LIMIT4);

  if (!prevLimit2 && l2 && stepper1Running) { stepper1Running = false; Serial.println("Spindel 1 gekalibreerd"); }
  if (!prevLimit3 && l3 && stepper2Running) { stepper2Running = false; Serial.println("Spindel 2 gekalibreerd"); }
  if (!prevLimit4 && l4 && stepper3Running) { stepper3Running = false; Serial.println("Spindel 3 gekalibreerd"); }

  prevLimit2 = l2; prevLimit3 = l3; prevLimit4 = l4;

  if (!stepper1Running && !stepper2Running && !stepper3Running) {
    calibrating = false;
    digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);
    Serial.println("Kalibratie voltooid!");

    // Start demo na kalibratie indien actief
    if (demoActief) {
      demoFase = 1;
      stepper1Running = true;
      digitalWrite(DIR1, SPINDEL1_DIR_REVERSED ? LOW : HIGH);
      Serial.println("Demo: Spindel 1 draait vooruit...");
    }
  }
}

// EINDE CODE :P