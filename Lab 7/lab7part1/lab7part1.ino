// Assignment 6 - One Way Traffic Controller

//pin declarations
int greenA = 9;             // Green A
int redA = 10;              // Red A
int greenB = 11;            // Green B
int redB = 12;              // Red B
int maintenanceButton = 2;  // Maintenance Button

// variables
volatile int maintenanceFlag = 0;    // maintenance flag
int state = 1;

void setup(){ 
  // configure pin to input or output
  pinMode(greenA, OUTPUT); 
  pinMode(redA, OUTPUT); 
  pinMode(greenB, OUTPUT); 
  pinMode(redB, OUTPUT);
  pinMode(maintenanceButton, INPUT_PULLUP);

  state = 1;
  maintenanceFlag = 0;
  attachInterrupt((maintenanceButton - 2), maintenanceInterrupt, RISING);
}

void maintenanceInterrupt(){
  maintenanceFlag = (maintenanceFlag == 1)?0:1;
}

void setFull(int pin){ 
  analogWrite(pin, 255); 
}

void setHalf(int pin){ 
  analogWrite(pin, 127); 
}

void setOff(int pin){
  analogWrite(pin, 0);
}

void delayAll(){
  // Street A and B are red
  
  setFull(redA);
  setOff(greenA);

  setFull(redB);
  setOff(greenB);

  delay(2000);      // delay 2 seconds

  // change state
  if (maintenanceFlag == 0){
    if (state == 1)
      state = 2;
    else if (state == 2)
      state = 1;
    else            // default state
      state = 1;
  }
  else
    state = 0;
}

void cycle(int green1, int red1, int green2, int red2){
  // Street 1 is green

  setFull(green1);
  setOff(red1);

  setFull(red2);
  setOff(green2);

  delay(8000);      // delay 8 seconds

  setHalf(green1);  // fade Green 1 for warning
  delay(2000);      // delay 2 seconds

  // change state
  state = 3;
}

void maintenance(){
  // freeze lights to red

  setFull(redA);
  setOff(greenA);

  setFull(redB);
  setOff(greenB);

  // change state
  if (maintenanceFlag == 1)
    state = 0;
  else
    state = 1;
}

void loop(){
  switch(state){
  case 0:    // maintenance state
    maintenance();
    break; 
  case 1:    // street A green
    cycle(greenA, redA, greenB, redB);
    break;
  case 2:    // street B green
    cycle(greenB, redB, greenA, redA);
    break;
  case 3:    // delay state
    delayAll();
    break;
  default:   // street A green
    state = 1;
    break;
  }
}
