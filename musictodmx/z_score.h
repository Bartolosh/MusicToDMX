#include <stdio.h>
#include <math.h>
#include <string.h>


#define SAMPLE_LENGTH 2048

float stddev(float data[], int len);
float mean(float data[], int len);
int thresholding(float y[], int lag, float threshold, float influence);