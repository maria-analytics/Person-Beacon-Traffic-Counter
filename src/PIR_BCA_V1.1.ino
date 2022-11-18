/*
 * Project PIR BCA beacon counter V1.1
 * Description: 
 * Author: Travis Morrison
 * Last Edits: Dec 13 2021 
 * This is the current deployable code for the battery powered trailhead sensors
 * 
 * 
 * Project to Do:
 * Prioity:
 * Add sleep interupt if battery falls below threshold- may not be nessecary with charge controller
 * 
 * 
 * Wiring
 * Needs to be updated
 * VIN - appears can take 3.3 V (3V3 pin) or 5 V (VUSB pin) 
 * OUT - D2 (digital pin 2)
 * GND - Ground
 */

//======================================================================================================
//Define I/O pins used
const pin_t Pin_PIR = D2;
const pin_t Pin_Beacon = A3;
const pin_t Pin_Battery =  A2;

//Define global variables 
int PIR_cnt = 0; //PIR counter and so forth
int Beacon_cnt = 0; //BCA pin counter and so forth
int Batt_read = 0;
float Batt_volt = 0;

// Define time period to update time
#define WRITE_DATA_MILLIS (60 * 60 * 2 * 1000) //needs to be same as sleep interval

unsigned long lastSync = millis();
unsigned long lastWrite = millis();

SYSTEM_THREAD(ENABLED); //allows the code to run before connecting to the cloud and will run without cloud conncetion
SerialLogHandler logHandler; //
SystemSleepConfiguration config;

// The event name to publish with has to be same as webhook
const char *eventName = "sheetTest1";

//Decleration of functions~ tells the compiler which functions are available and how to use them
void PublishToGoogleSheet();

// setup() runs once, when the device is first turned on.
void setup() {

  //Set the digital pins to input
  //note that analog pins don't need analog read
  pinMode(Pin_PIR, INPUT);


}

// main loop for code, runs continuously
void loop() {
    //make sure where connected before sleep~don't think I need that
    
    // Sleep command until one of the signals gets a signal
    config.mode(SystemSleepMode::ULTRA_LOW_POWER).flag(SystemSleepFlag::WAIT_CLOUD).duration(WRITE_DATA_MILLIS).gpio(Pin_PIR, RISING).analog(Pin_Beacon, 1500, AnalogInterruptMode::ABOVE); 
    System.sleep(config);

    if (digitalRead(Pin_PIR)== HIGH){
      PIR_cnt = PIR_cnt + 1;
    }
    if (analogRead(Pin_Beacon)> 2047){// analog read reads from 0 to 4095 for 0 to 3.3 V, so 2047 corresponds to greater than 1.65 V
      Beacon_cnt = Beacon_cnt + 1;
    }

    //write to google sheets every time it wakes up from sleep routine and since last write is greater than write interval
    if (millis() - lastWrite > WRITE_DATA_MILLIS) {
      if(Particle.connected() == false) {
          Particle.connect();
          waitUntil(Particle.connected);
      }
        // read battery voltage
        Batt_read = analogRead(Pin_Battery);
        Batt_volt = (Batt_read*3.3/4095.0)*5.0; //multiply by 3.3/4095 to convert to voltage, mult. by 5 bc of 5 to 1 voltage divider
        
       
        // Calls Publish to Google sheet function
        PublishToGoogleSheet();
        
        PIR_cnt = 0; 
        Beacon_cnt = 0; 

        delay(500ms);
        //reset last write
        lastWrite = millis();
    }
   

}


//Publish to google sheets function
void PublishToGoogleSheet() {
    char buf[128];
    snprintf(buf, sizeof(buf),"[%d,%d,%.2f]", PIR_cnt, Beacon_cnt, Batt_volt);
    Particle.publish(eventName, buf, PRIVATE);
    Log.info("published: %s", buf);
}