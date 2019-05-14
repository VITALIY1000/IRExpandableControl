#include <IRremote.h>
#define RECEIVER_PIN 2
#define OUTPUT_PIN 12
#define BUTTON_CHANGE_ACTIVATE_SIGNAL_PIN 7
#define FIRST_INDICATOR_PIN 9
#define ERROR_INDICATOR_PIN 3
#define SUCCESS_INDICATOR_PIN 4
#define processIndicatorsNumber 3
#define maxSignalReceivingDelay 100000

IRrecv irrecv(RECEIVER_PIN);
decode_results results;

long ActivateSignal = 0;

/* tmp variables: */
boolean isOutputTurnedOn = false;
boolean isReceiverDelayOn = false;
boolean isChangeActivateSignalModeActive = false;
short signalCounter = 0;
long tmpSignal = 0;

/* millis variables */
long receiverDelay;



void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(ERROR_INDICATOR_PIN, OUTPUT);
  pinMode(SUCCESS_INDICATOR_PIN, OUTPUT);
  pinMode(BUTTON_CHANGE_ACTIVATE_SIGNAL_PIN, INPUT_PULLUP);

  int i = 0;
  while (i < processIndicatorsNumber) {
    pinMode(i + FIRST_INDICATOR_PIN, OUTPUT);
    i++;
  }
  
}

void loop() {
  if (isReceiverDelayOn) {
    // задержка приема следующего сигнала
    // (что-бы не было ложных срабатываний)
    if (millis() - receiverDelay > 50) {
      isReceiverDelayOn = false;
      irrecv.resume();
    }
  } else {
    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);

      //обрабатываем полученый сигнал
      resultProcessing(results.value);

      isReceiverDelayOn = true;
      receiverDelay = millis();
    } else if (Serial.available() > 0) {
      long parsedInt = Serial.parseInt();
      Serial.println(parsedInt, HEX);

      //обрабатываем полученый сигнал
      resultProcessing(parsedInt);

      isReceiverDelayOn = true;
      receiverDelay = millis();
    }
  }
  
  if (digitalRead(BUTTON_CHANGE_ACTIVATE_SIGNAL_PIN) == LOW) {
    isChangeActivateSignalModeActive = true;
  }
  
}



void resultProcessing(int receivedSignal) {
  if (isChangeActivateSignalModeActive) {
    if (signalCounter == 0) {
      tmpSignal = receivedSignal;
      digitalWrite(FIRST_INDICATOR_PIN, HIGH);
      signalCounter++;
      
    } else {
      if (signalCounter == processIndicatorsNumber - 1) {
        digitalWrite(FIRST_INDICATOR_PIN + signalCounter, HIGH);
        delay(1000);
        finish(false);
      } else {
        if (receivedSignal == tmpSignal) {
          digitalWrite(FIRST_INDICATOR_PIN + signalCounter, HIGH);
          signalCounter++;
    
        } else {
          finish(true);
        }
      }
    }
  } else {
    if (receivedSignal == ActivateSignal) {
      if (isOutputTurnedOn) {
        digitalWrite(OUTPUT_PIN, LOW);
      } else {
        digitalWrite(OUTPUT_PIN, HIGH);
      }
      isOutputTurnedOn = !isOutputTurnedOn;
    }
  }
}



void finish(boolean error) {
  if(error) {
    digitalWrite(ERROR_INDICATOR_PIN, HIGH);
    delay(1000);
    digitalWrite(ERROR_INDICATOR_PIN, LOW);
  } else {
    digitalWrite(SUCCESS_INDICATOR_PIN, HIGH);
    delay(1000);
    digitalWrite(SUCCESS_INDICATOR_PIN, LOW);

    ActivateSignal = tmpSignal;
  }
  
  isChangeActivateSignalModeActive = false;
  signalCounter = 0;
  
  int i = 0;
  while (i < processIndicatorsNumber) {
    digitalWrite(i + FIRST_INDICATOR_PIN, LOW);
    i++;
  }
}
