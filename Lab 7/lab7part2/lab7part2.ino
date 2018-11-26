// Assignment 6 - One Way Traffic Controller

//pin declarations
int greenA = 9;             // Green A
int redA = 10;              // Red A
int greenB = 11;            // Green B
int redB = 12;              // Red B
int pedestrianLight = 13;   // Pedestrian Light
int pedestrianAButton = 2;  // Pedestrian A Button
int pedestrianBButton = 3;  // Pedestrian B Button 

// variables
volatile int pedestrianFlag = 0;  // Pedestrian Flag
int state = 1;

void setup(){ 
  // configure pin to input or output
  pinMode(greenA, OUTPUT); 
  pinMode(redA, OUTPUT); 
  pinMode(greenB, OUTPUT); 
  pinMode(redB, OUTPUT);
  pinMode(pedestrianLight, OUTPUT);

  pinMode(pedestrianAButton, INPUT_PULLUP);
  pinMode(pedestrianBButton, INPUT_PULLUP);

  state = 1;
  pedestrianFlag = 0;
  attachInterrupt((pedestrianAButton - 2), pedestrianAInterrupt, RISING);
  attachInterrupt((pedestrianBButton - 2), pedestrianBInterrupt, RISING);
}

void pedestrianAInterrupt(){
  if (state != 4){
    switch(pedestrianFlag){
    case 0:       // no pedestrian crossing currently
      pedestrianFlag = 1;
      break;
    case 2:       // pedestrian crossing request for street B currently
      pedestrianFlag = 3;
      break;
    default:
      break;      // ignore new requests on same street
    }
  }
}

void pedestrianBInterrupt(){
  if (state != 3){
    switch(pedestrianFlag){
    case 0:      // no pedestrian crossing currently
      pedestrianFlag = 2;
      break;
    case 1:      // pedestrian crossing request for street A currently
      pedestrianFlag = 4;
      break;
    default:
      break;     // ignore new requests on same street
    }
  }
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

  setOff(pedestrianLight);

  delay(2000);

  // change state
  switch(pedestrianFlag){
  case 1:                // cross street A
    state = 4;
    pedestrianFlag = 0;
    break;
  case 2:                // cross street B
    state = 3;
    pedestrianFlag = 0;
    break;
  case 3:                // cross street B then A
    state = 3;
    pedestrianFlag = 1;
    break;
  case 4:                // cross street A then B
    state = 4;
    pedestrianFlag = 2;
    break;
  default:               // no pedestrian crossing
    switch(state){
    case 1:
      state = 2;
      break;
    case 2:
      state = 1;
      break;
    case 3:
      state = 2;
      break;
    case 4:
      state = 1;
      break;
    default:
      state = 1;
      break;
    }
    break;
  }
}

void cycle(int green1, int red1, int green2, int red2, int pedestrianLightOn){
  // Street 1 is green
  
  if (pedestrianLightOn == 1)
    setFull(pedestrianLight);

  setFull(green1);
  setOff(red1);

  setFull(red2);
  setOff(green2);

  if (pedestrianLightOn == 1)
    delay(16000);     // delay 16 seconds
  else
    delay(8000);      // delay 8 seconds

  setHalf(green1);    // fade Green 1 for warning
  
  if(pedestrianLightOn == 1)
    delay(4000);      // delay 4 seconds
  else
    delay(2000);      // delay 2 seconds

  // change state
  state = 0;
}

void loop(){
  switch(state){
  case 0:    // delay state
    delayAll();
    break;
  case 1:    // street A green and no pedestrian
    cycle(greenA, redA, greenB, redB, 0);
    break;
  case 2:    // street B green and no pedestrian
    cycle(greenB, redB, greenA, redA, 0);
    break;
  case 3:    // street A green and pedestrian crossing street B
    cycle(greenA, redA, greenB, redB, 1);
    break;
  case 4:    // street B green and pedestrian crossing street A
    cycle(greenB, redB, greenA, redA, 1);
    break;
  default:   // street A green and no pedestrian
    state = 1;
    break;
  }
}
