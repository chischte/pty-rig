void RunMainTestCycle() {

  if (clearanceNextStep && nextStepTimer.timedOut()) {
    static byte subStep = 0;

    switch (cycleStep) {
    case VIBRIEREN: // PLOMBE FALLENLASSEN
      eepromCounter.getValue(cycleDurationTime);
      cycleDurationTimer.setTime(eepromCounter.getValue(cycleDurationTime) * 1000);
      ZylRevolverschieber.stroke(250, 300);    //(push time,release time)
      if (ZylRevolverschieber.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case KLEMMEN: // PLOMBEN IM RUTSCH-SCHACHT FIXIEREN
      errorBlink = !sealAvailable;
      if (sealAvailable) {
        ZylGummihalter.set(1); // Plomben fixieren
      } else { // MAGAZIN LEER!
        machineRunning = false;
        ZylGummihalter.set(0); // zum Befüllen zurückziehen
        break;
      }
      ZylMagnetarm.set(0);
      clearanceNextStep = false;
      nextStepTimer.setTime(300);
      cycleStep++;
      break;

    case FALLENLASSEN: // PLOMBE FALLENLASSEN
      if (subStep == (0)) {
        ZylFalltuerschieber.stroke(150, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep++;
        }
      }
      if (subStep == 1 || subStep == 2) {
        ZylFalltuerschieber.stroke(40, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep++;
        }
      }
      if (subStep == 3) {
        ZylFalltuerschieber.stroke(300, 40);    //(push time,release time)
        if (ZylFalltuerschieber.stroke_completed()) {
          subStep = 0;
          clearanceNextStep = false;
          cycleStep++;
        }
      }
      break;

    case MAGNETARM_AUSFAHREN:
      // PLOMBE ZUM ZANGENPAKET FAHREN

      // ZUERST SICHERSTELLEN DASS FALLTÜRE GESCHLOSSEN IST:
      if (subStep == 0) {
        ZylSchild.set(0);
        ZylFalltuerschieber.set(0);
        nextStepTimer.setTime(300);
        subStep++;
        break;
      }
      if (subStep == 1) {
        ZylMagnetarm.set(1);
        ToolReset();    //reset tool "Wippenhebel ziehen"
        ZylGummihalter.set(0); // Plombenfixieren lösen
        nextStepTimer.setTime(600);
        clearanceNextStep = false;
        subStep = 0;
        cycleStep++;
      }
      break;

    case BAND_UNTEN:
      // UNTERES BAND VORSCHIEBEN
      ZylSchild.set(1);
      ZylGummihalter.set(0);    //Plomben für nächsten Zyklus können nachrutschen
      if (lowerStrapAvailable) {
        MotFeedUnten.stroke(eepromCounter.getValue(lowerFeedtime), 400);
        if (MotFeedUnten.stroke_completed()) {
          lowerStrapBlockCounter = 0;
          clearanceNextStep = false;
          cycleStep++;
        }
      } else {
        MotFeedUnten.abort_stroke();
        lowerStrapBlockCounter++;
        if (lowerStrapBlockCounter == 2) {
          machineRunning = false;
          errorBlink = true;
        }
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case BAND_OBEN:
      // OBERES BAND VORSCHIEBEN
      ZylMesser.set(0); // Messer muss zurückgezogen sein
      if (upperStrapAvailable) {
        MotFeedOben.stroke(eepromCounter.getValue(upperFeedtime), 400);
        if (MotFeedOben.stroke_completed()) {
          upperStrapBlockCounter = 0;
          clearanceNextStep = false;
          cycleStep++;
        }
      } else {
        MotFeedOben.abort_stroke();
        upperStrapBlockCounter++;
        if (upperStrapBlockCounter == 2) {
          machineRunning = false;
          errorBlink = true;
        }
        nextStepTimer.setTime(500);
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case ZURUECKFAHREN:
      // MAGNETARM ZURÜCKZIEHEN
      if (ZylMagnetarm.stroke_completed()) {
        //Serial.println("Mangetarm zurückfahren...");
      }
      ZylMagnetarm.set(0);
      nextStepTimer.setTime(600);
      clearanceNextStep = false;
      cycleStep++;
      break;

    case PRESSEN:
      // CRIMPVORGANG STARTEN

      digitalWrite(TOOL_MOTOR_RELAY, HIGH);
      nextStepTimer.setTime(3000);
      clearanceNextStep = false;
      cycleStep++;
      break;

    case SCHNEIDEN:
      // BAND ABSCHNEIDEN
      ZylSchild.set(1);
      ZylMesser.stroke(1500, 200); // push,release [ms]
      if (ZylMesser.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case BLASEN:
      // BAND ABSCHNEIDEN
      ZylAirBlower.stroke(100, 50); // push,release [ms]
      if (ZylAirBlower.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      break;

    case REVOLVER:
      // KARUSSELL DREHEN FALLS KEINE PLOMBE DETEKTIERT
      if (!sealAvailable) { // keine Plombe detektiert
        ZylRevolverschieber.stroke(4000, 3500);
      }
      if (ZylRevolverschieber.stroke_completed()) {
        clearanceNextStep = false;
        cycleStep++;
      }
      ZylSchild.set(0);
      break;

    case PAUSE:
      // WARTEN AUF NÄCHSTEN ZYKLUS
      greenBlink = true;
      //Serial.println(cycleDurationTimer.remainingTimeoutTime()/1000);
      if (cycleDurationTimer.timedOut()) {
        cycleStep = 0;
        clearanceNextStep = false;
        //stepMode = true; // activate this line to deactivate auto mode after every cycle
        cycleDurationTimer.resetTime();
        printOnTextField((" "), "t4"); //clear display text field
        greenBlink = false;
        eepromCounter.countOneUp(shorttimeCounter);
        eepromCounter.countOneUp(longtimeCounter);
      }
      break;
    }
  }
}
