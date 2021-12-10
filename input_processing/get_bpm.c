#include "arm_math.h"
#include "arm_const_struct.h"
#include <stdio.h>

#define SAMPLES 2048

extern float32_t input_f32_10khz[SAMPLES];
static float32_t output[SAMPLES/2];

uint32_t fft_n_bins = 1024;
uint32_t ifft_flag = 0;
uint32_t do_bit_reverse = 1;
arm_cfft_instance_f32 fft_f32_instance;
uint32_t max_index=0;

int32_t main(void){
    arm_status status;
    float32_t max_val;
    status = ARM_MATH_SUCCESS;
    status = arm_cfft_init_f32(&fft_f32_instance,fft_n_bins);

    // Process data through CFFT module
    arm_cfft_f32(&fft_f32_instance,input_f32_10khz,ifft_flag,do_bit_reverse)
    
    // Calculating magnitude at each bin
    arm_cmplx_mag_f32(input_f32_10khz,output,fft_n_bins);

    // Calculates max value and returns corresponding bin value
    arm_max_f32(output,fft_n_bins,&max_val,&max_index);
 
    
}