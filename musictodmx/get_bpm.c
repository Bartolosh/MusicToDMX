#include "get_bpm.h"

void get_bpm(float *buffer){
    float *buffer_im[SAMPLES];
    arduinoFFT FFT = arduinoFFT();
    FFT.Windowing(buffer,SAMPLES,FFT_WIN_TYP_HAMMING,FFT_FORWARD);
    FFT.Compute(buffer,buffer_im,FFT_FORWARD);
    FFT.ComplexToMagnitude(buffer,buffer_im,SAMPLES);
    
    //TODO: need to fine tune the third param
    double peak = FFT.MajorPeak(buffer,SAMPLES,5000);
    Serial.println('Spectrum values:');
    Serial.print('[');
    for(int i = 0 ; i < SAMPLES; i++){
        Serial.print(buffer[i]);
        Serial.print(',');
    }
    Serial.println(']');
}
