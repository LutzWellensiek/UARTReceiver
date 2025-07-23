#pragma once

#define _USE_MATH_DEFINES   //To include Mathematical Macros
//#include <iostream>
//#include <stdint.h>
/*Header for gaussian filter class, which contains the functions for gaussian filtering
Created by Csaba Freiberger 03.2023*/

#include "Arduino.h"
//#include <cmath>

//using namespace std;

/*Define constants*/
const uint8_t MAX_SENSOR_ARRAY_SIZE = {80};
const uint8_t MAX_KERNEL_SIZE = {39};
const uint8_t MAX_SENSOR_NEW_ARRAY_SIZE = MAX_SENSOR_ARRAY_SIZE + MAX_KERNEL_SIZE;

class GaussianFilter
{
private:
    /*Define global variables*/
    //Initialise kernel array
     float kernel_values[MAX_KERNEL_SIZE];
     float input_new[MAX_SENSOR_NEW_ARRAY_SIZE];
     uint8_t kernel_radius = 0;
public:
    //Initialise output array
    float kernel[MAX_KERNEL_SIZE];
    uint8_t kernel_width = MAX_KERNEL_SIZE;
    float output[MAX_SENSOR_ARRAY_SIZE];
    GaussianFilter(/* args */);
    ~GaussianFilter();
   
    //static void setInput(double _input[MAX_SENSOR_ARRAY_SIZE], uint32_t _input_width);
    //static void setKernel(double _kernel[MAX_KERNEL_SIZE], uint32_t _kernel_width);
    void calcGaussianKernel(float sigma = 1.0F, uint8_t _kernel_width = MAX_KERNEL_SIZE);
    bool getGaussianKernel(float sigma = 1.0F, uint8_t _kernel_width = MAX_KERNEL_SIZE);
    void convolve1D(float _input[MAX_SENSOR_ARRAY_SIZE], float _kernel[MAX_KERNEL_SIZE], uint8_t input_width = MAX_SENSOR_ARRAY_SIZE, uint8_t _kernel_width = MAX_KERNEL_SIZE);
    bool calcGausianFilter1D(float _input[MAX_SENSOR_ARRAY_SIZE], float _kernel[MAX_KERNEL_SIZE], uint8_t input_width = MAX_SENSOR_ARRAY_SIZE, uint8_t _kernel_width = MAX_KERNEL_SIZE);
};



