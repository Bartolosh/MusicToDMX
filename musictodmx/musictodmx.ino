#include <STM32FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>
#include <string.h>


#include "manage_output.h"
#include "fog.h"
#include "fire.h"
#include "LowPassFilter.h"
#include "LowCutFilter.h"

#define SAMPLES 170
#define FRAME_LENGTH 100
#define MS_IN_MIN 60000
#define THRESHOLD_MOV 400   
#define THRESHOLD 350 

#define TRIGPIN D8
#define ECHOPIN D10

SemaphoreHandle_t buffer_mtx;
SemaphoreHandle_t new_data_mtx;
SemaphoreHandle_t bpm_mtx;
SemaphoreHandle_t color_mtx;
SemaphoreHandle_t mov_mtx;
SemaphoreHandle_t fog_mtx;
SemaphoreHandle_t fire_mtx;

int32_t buffer[SAMPLES] = {0};

HardwareSerial Serial4(D0,D1);

RS485Class RS485(Serial4, RS485_DEFAULT_TX_PIN, RS485_DEFAULT_DE_PIN, RS485_DEFAULT_RE_PIN);

HardwareTimer *firstTimer = new HardwareTimer(TIM1);
    
HardwareTimer *secondTimer = new HardwareTimer(TIM2);

uint16_t bpm = 120;
  
/*-------------------- PERIODIC TASK ------------------------*/

void taskInputRecording(void *pvParameters){
    uint8_t c = 0;

    TickType_t xLastWakeTime;
    TickType_t xNextRead;
    /* xFreq is set to 1/4  of seconds */
    const TickType_t xFreq = 250 / (portTICK_PERIOD_MS);
    const TickType_t xFreqRec = 1 / (portTICK_PERIOD_MS);

    xLastWakeTime= xTaskGetTickCount();
    
    while(true){
        xSemaphoreTake(buffer_mtx, portMAX_DELAY);
        
        xNextRead = xTaskGetTickCount();
        while(c < SAMPLES){
            
            
            buffer[c] = (uint16_t)analogRead(A0);
            c++;
            
            vTaskDelayUntil(&xNextRead, xFreqRec);
            
        }
        
        c=0;
        if(uxSemaphoreGetCount(new_data_mtx) == 0){
            xSemaphoreGive(new_data_mtx);
        }
       
        xSemaphoreGive(buffer_mtx);
        
        vTaskDelayUntil(&xLastWakeTime, xFreq);
        
    }
}

void taskInputProcessing(void *pvParameters){

  uint64_t lastChange = 0;
  uint64_t thisChange = 0;
  
  int16_t cutted,filtered;
  int16_t lvl_sound = 0, peak_to_peak = 0;

  int16_t max_peak_fil = 0;
  int16_t peak_fil = 0;
  int16_t min_peak_fil = 1023;
  int16_t max_peak = 0;
  int16_t min_peak = 0;
  int16_t n = 0;

  LowCutFilter *cutfilter;
  LowPassFilter *filter;
  
  cutfilter = new LowCutFilter();
  filter = new LowPassFilter();
  LowCutFilter_init(cutfilter);
  LowPassFilter_init(filter);

  while(true){
    xSemaphoreTake(new_data_mtx, portMAX_DELAY);
    xSemaphoreTake(buffer_mtx, portMAX_DELAY);
    peak_fil = 0;

    for(uint8_t i = 0; i < SAMPLES; i++){

      LowCutFilter_put(cutfilter,buffer[i]);
      cutted = LowCutFilter_get(cutfilter);
      /* filtering signal */
      LowPassFilter_put(filter,cutted);
      filtered = LowPassFilter_get(filter);
      filtered = abs(filtered);
      
      peak_fil = max(peak_fil, filtered);
      
    }
    xSemaphoreGive(buffer_mtx);
    max_peak_fil = max(max_peak_fil,peak_fil);
    min_peak_fil = min(min_peak_fil, peak_fil);

    n++;

    /* each 60 iterations reset peak for lighting movement */
    if((n%60) == 0){
      lvl_sound = 0;
    }

    /* each 80 iterations reset the peak to adapt light  */
    if((n%80) == 0){
      max_peak_fil = 0;
      min_peak_fil = 1023;
      n = 0;
    }

    uint16_t lvl = map(peak_fil, min_peak_fil, max_peak_fil, 0, 1023);
 
    
    if(lvl > THRESHOLD){
      if((millis()-lastChange)>300){
        xSemaphoreGive(color_mtx);
        thisChange = millis();
        peak_to_peak = thisChange-lastChange;
        lastChange = thisChange;
        xSemaphoreTake(bpm_mtx,portMAX_DELAY);
        bpm = 60000/peak_to_peak;
  
        xSemaphoreGive(bpm_mtx);
      }
    }
    if(peak_to_peak > (lvl_sound - THRESHOLD_MOV)){
      lvl_sound = peak_to_peak;
      peak_to_peak = 0;
      xSemaphoreGive(mov_mtx);
    }   
        
  }
}

void taskSendingOutput(void *pvParameters){
    
    TickType_t xLastWakeTime_out;
    
    const TickType_t xFreq_out = 41 / portTICK_PERIOD_MS;

    xLastWakeTime_out = xTaskGetTickCount();
    uint8_t light_mode, mov_mode;
    uint8_t fog_state = STOP;
    uint8_t fire_state = STOP;
    uint32_t duration = 0, duration_end = 0;
    uint8_t count_fire = 0;
    uint16_t tmp_bpm = 120;

    while(true){
        xSemaphoreTake(bpm_mtx, portMAX_DELAY);
        tmp_bpm = bpm;
        xSemaphoreGive(bpm_mtx);
        
        if(uxSemaphoreGetCount(color_mtx) > 0){
          
          xSemaphoreTake(color_mtx,portMAX_DELAY);
          
          light_mode = 1;
        }
        else{    
          light_mode = 0;
        }
        if(uxSemaphoreGetCount(mov_mtx) > 0){
          xSemaphoreTake(mov_mtx,portMAX_DELAY);
          mov_mode = 1;
        }
        else{
          mov_mode = 0;
        }

        if(uxSemaphoreGetCount(fog_mtx) == 0){
          duration = millis();
          if(duration_end == 0){
            duration_end = millis();
          }
          fog_state = START;
          if((duration - duration_end) >= 1500){
            duration = 0; 
            duration_end = 0;
            fog_state = STOP;
            xSemaphoreGive(fog_mtx);   
          }
        }
        if(uxSemaphoreGetCount(fire_mtx) == 0){
          if(count_fire == 3){
            xSemaphoreGive(fire_mtx);
          }
          count_fire++;
          fire_state=START;
        }
        else{
          fire_state=STOP;
          count_fire=0;
        }
        send_output(tmp_bpm, light_mode, mov_mode, fog_state, fire_state);
        vTaskDelayUntil(&xLastWakeTime_out,xFreq_out);
        
    }
}

/*-------------------- ASYNC TASK ------------------------*/
void taskFog(void *pvParameters) {
 
  while (true) {
    vTaskSuspend(NULL);                                         
    if(uxSemaphoreGetCount(fog_mtx) == 1){
      xSemaphoreTake(fog_mtx, portMAX_DELAY);
    }
   }
}

long startTime = 0; 
long finishTime = 0; 

uint8_t counter = 0;

void sendLow(void)
{ 
  long duration;
  long distance;
  uint8_t i= 0;
  digitalWrite(TRIGPIN,LOW);
  duration = pulseIn(ECHOPIN, HIGH, 9000);
 
  Serial.println(duration);
  secondTimer->detachInterrupt();
  //Serial.println(micros()-startTime);
  /*if (duration != 0){
    distance = 0.034 * duration / 2;
    secondTimer->detachInterrupt();
    counter = 0;
    Serial.println(distance);
    if(distance>100){
      if(uxSemaphoreGetCount(fire_mtx) == 1){
        xSemaphoreTake(fire_mtx, portMAX_DELAY);
      }
    }
  }
  else{
    counter++;
    if(counter ==20){
      secondTimer->detachInterrupt();
      counter =0;
    }
    else{
      startTime = micros();
      secondTimer->setOverflow(500, MICROSEC_FORMAT); 
      secondTimer->resume();
    }
  }*/
  
  //Serial.println((String)"fire = " + (micros()-startTime)); 
}

void sendHigh(void)
{ 

  digitalWrite(TRIGPIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN,LOW);
  Serial.println("BASSO");
  secondTimer->attachInterrupt(sendLow);
  firstTimer->detachInterrupt();
  
  secondTimer->resume();
  
}



void taskFire(void *pvParameters) {
  uint8_t i = 0; 
  uint32_t duration = 0;
  uint8_t distance = 0;
  while (true) {
    vTaskSuspend(NULL);
    
    digitalWrite( TRIGPIN, LOW );
    delayMicroseconds(2);
    digitalWrite(TRIGPIN,HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN,LOW);
    duration = pulseIn(ECHOPIN, HIGH, 8500);
    distance = 0.034 * duration / 2;
    
    if(distance>100 || distance == 0){
          if(uxSemaphoreGetCount(fire_mtx) == 1){
            xSemaphoreTake(fire_mtx, portMAX_DELAY);
          }
      }
      
      
  
  }
 
}

void taskVal(void *pvParameters) {
  TickType_t xLastWakeTime_val;

  xLastWakeTime_val = xTaskGetTickCount();
  while(true){
    Serial.println("memory =  " + String(uxTaskGetStackHighWaterMark(xTaskGetHandle("fireStart"))));
  
     vTaskDelayUntil(&xLastWakeTime_val,((TickType_t)400/portTICK_PERIOD_MS));
  }
}

void setup(){

    pinMode(TRIGPIN, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    
    fireSelector();
    fogSelector();
    init_fixture();
    /*Serial.setTx(D1);
    Serial.setRx(D0);
    Serial.begin(250000);*/
    Serial.begin(115200);

    firstTimer->setOverflow(2, MICROSEC_FORMAT); 
    
    
    secondTimer->setOverflow(10, MICROSEC_FORMAT); 
    
    
    
    buffer_mtx = xSemaphoreCreateMutex();   /* semaphores for buffer*/
    xSemaphoreGive(buffer_mtx);
    
    bpm_mtx = xSemaphoreCreateMutex();
    xSemaphoreGive(bpm_mtx);
    
    fog_mtx = xSemaphoreCreateBinary();
    xSemaphoreGive(fog_mtx);
   
    fire_mtx = xSemaphoreCreateBinary();
    xSemaphoreGive(fire_mtx);

    new_data_mtx = xSemaphoreCreateBinary();
    
    mov_mtx = xSemaphoreCreateBinary();
    
    color_mtx = xSemaphoreCreateBinary(); 

    xTaskCreate(taskInputRecording, "inputRec", 81, NULL, 4, NULL); 

    xTaskCreate(taskInputProcessing, "inputProc",51, NULL,1, NULL);
    
    xTaskCreate(taskSendingOutput, "outputSend", 65, NULL, 0, NULL);

    xTaskCreate(taskVal, "validationTa", 565, NULL, 2, NULL);

    xTaskCreate(taskFog, "fogStart", 27, NULL, 2, &taskFogHandle);
    xTaskCreate(taskFire, "fireStart", 42, NULL, 3, &taskFireHandle);
    
    vTaskStartScheduler();                                                /* explicit call needed */
    

}

void loop(){}
