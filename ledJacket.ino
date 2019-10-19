#include "arduinoFFT.h"
#include <FastLED.h>
  
#define SAMPLES 256               //Must be a power of 2
#define SAMPLING_FREQUENCY 20000  //Hz
#define REFRESH_RATE 25           //Hz
#define ARDUINO_IDE_PLOTTER_SIZE 500

#define LED_PIN 17
#define NUM_LEDS 1

CRGB leds[NUM_LEDS];
  
arduinoFFT FFT = arduinoFFT();
  
unsigned long sampling_period_us;
unsigned long useconds_sampling;
 
unsigned long refresh_period_us;
unsigned long useconds_refresh;
  
double vReal[SAMPLES];
double vImag[SAMPLES];
 
uint8_t analogpin = A0;

int maxOverallBucketValues[6] = {0,0,0,0,0,0};
double bucketValuePercentages[6] = {0,0,0,0,0,0};
  
void setup() {
  Serial.begin(115200);
 
  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
  refresh_period_us = round(1000000*(1.0/REFRESH_RATE));
 
  pinMode(analogpin, INPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

}
  
void loop() {
  listenAndTransform();

  leds[0] = CRGB(((255*bucketValuePercentages[0])/100), 0, 0);
//  leds[0] = CRGB(0,((255*bucketValuePercentages[1])/100), 0);
//  leds[0] = CRGB(0,0, ((255*bucketValuePercentages[2])/100));
//  leds[0] = CRGB(255, 0, 0);
  FastLED.show();
}


int listenAndTransform(){
    useconds_refresh = micros();
   
  /*SAMPLING*/
  for(int i=0; i<SAMPLES; i++)
  {
    useconds_sampling = micros();
  
    vReal[i] = analogRead(analogpin);
    vImag[i] = 0;
  
    while(micros() < (useconds_sampling + sampling_period_us)){
      //wait...
    }
  }  
 
  /*FFT*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  int maxSampleBucketValues[6] = {0,0,0,0,0,0};

/* Pull out the max value per bucket in this sample; also check to see if it's greater that the overall bucket */
  for (int i=2; i<(SAMPLES/2); i++){
    if (i < 4 && vReal[i] > maxSampleBucketValues[0]){      
      maxSampleBucketValues[0] = vReal[i];
      vReal[i] > maxOverallBucketValues[0] ? maxOverallBucketValues[0] = vReal[i] : NULL;
    } else if (i < 8 && vReal[i] > maxSampleBucketValues[1]){
      maxSampleBucketValues[1] = vReal[i];
      vReal[i] > maxOverallBucketValues[1] ? maxOverallBucketValues[1] = vReal[i] : NULL;
    } else if (i < 15 && vReal[i] > maxSampleBucketValues[2]){
      maxSampleBucketValues[2] = vReal[i];
      vReal[i] > maxOverallBucketValues[2] ? maxOverallBucketValues[2] = vReal[i] : NULL;
    } else if (i < 25 && vReal[i] > maxSampleBucketValues[3]){
      maxSampleBucketValues[3] = vReal[i];
      vReal[i] > maxOverallBucketValues[3] ? maxOverallBucketValues[3] = vReal[i] : NULL;
    } else if (i < 40 && vReal[i] > maxSampleBucketValues[4]){
      maxSampleBucketValues[4] = vReal[i];
      vReal[i] > maxOverallBucketValues[4] ? maxOverallBucketValues[4] = vReal[i] : NULL;
    } else if  (vReal[i] > maxSampleBucketValues[5]){
      maxSampleBucketValues[5] = vReal[i];
      vReal[i] > maxOverallBucketValues[5] ? maxOverallBucketValues[5] = vReal[i] : NULL;
    } 
   }
    

    bucketValuePercentages[0] = maxSampleBucketValues[0]*100/maxOverallBucketValues[0];
    bucketValuePercentages[1] = maxSampleBucketValues[1]*100/maxOverallBucketValues[1];
    bucketValuePercentages[2] = maxSampleBucketValues[2]*100/maxOverallBucketValues[2];
    bucketValuePercentages[3] = maxSampleBucketValues[3]*100/maxOverallBucketValues[3];
    bucketValuePercentages[4] = maxSampleBucketValues[4]*100/maxOverallBucketValues[4];
    bucketValuePercentages[5] = maxSampleBucketValues[5]*100/maxOverallBucketValues[5];

            
    Serial.println();
    Serial.println();
    Serial.println(analogRead(analogpin));
    Serial.println(bucketValuePercentages[0]);
    Serial.println();
    Serial.println(makePercentageBars(0, (maxSampleBucketValues[0]*100)/maxOverallBucketValues[0]));
    Serial.println(makePercentageBars(1, (maxSampleBucketValues[1]*100)/maxOverallBucketValues[1]));
    Serial.println(makePercentageBars(2, (maxSampleBucketValues[2]*100)/maxOverallBucketValues[2]));
    Serial.println(makePercentageBars(3, (maxSampleBucketValues[3]*100)/maxOverallBucketValues[3]));
    Serial.println(makePercentageBars(4, (maxSampleBucketValues[4]*100)/maxOverallBucketValues[4]));
    Serial.println(makePercentageBars(5, (maxSampleBucketValues[5]*100)/maxOverallBucketValues[5]));
  while(micros() < (useconds_refresh + refresh_period_us)){
    //wait...
  }

  return bucketValuePercentages;
}


String makePercentageBars(int bucketNumber, int sizeOfBar){
  String valueBar = "";
  valueBar += String(bucketNumber);
  valueBar += ": ";
  
  for (int i=0; i<=sizeOfBar; i++){
    valueBar += String("#");
  }

  return valueBar;
}
