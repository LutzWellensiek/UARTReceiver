#include "gaussian_filter.h"
#include "SerialMon.h"

/*Source file wich contains the functions for gaussian filtering
Created by Csaba Freiberger 03.2023*/
 GaussianFilter::GaussianFilter(/* args */)
 {
     for (uint8_t i = 0; i < MAX_KERNEL_SIZE; i++)
     {
         kernel[i] = 0.0F;
         kernel_values[i] = 0.0F;
     }

     for (uint8_t i = 0; i < MAX_SENSOR_ARRAY_SIZE; i++)
     {
         output[i] = 0.0F;
     }

     for (uint8_t i = 0; i < MAX_SENSOR_NEW_ARRAY_SIZE; i++)
     {
        input_new[i] = 0.0F;
     }

     kernel_radius = {0};
 }

 GaussianFilter::~GaussianFilter()
 {
 }

void GaussianFilter::calcGaussianKernel(float sigma, uint8_t _kernel_width) {


    //Check if kernel size is odd
    if (_kernel_width % 2 == 0)
    {
        //Initialize kernel with size of kernel_width+1
        for (uint8_t i = {0}; i < _kernel_width + 1; i++)
        {
            kernel[i] = 0.0F;
        }
        kernel_width = _kernel_width + 1; //= sizeof(kernel) / sizeof(kernel[0]);
    }
    else
    {
        //Initialize kernel with size of kernel_width
        for (uint8_t i = {0}; i < _kernel_width; i++)
        {
            kernel[i] = 0.0F;
        }
        kernel_width = _kernel_width;
    }
    kernel_radius = (kernel_width - 1) / 2;
    if (kernel_radius<1)
    {
        kernel_radius = {1};
    }
   
    /*Compute values of gaussian function in kernel radius*/
    //Initialise kernel values with new size
    for (uint8_t i = {0}; i < kernel_radius; i++)
    {
        kernel_values[i] = 0.0F;
    }

    for (uint8_t x = {1}; x <= kernel_radius; x++)
    {
        kernel_values[x - 1] = (1 / (sqrt(2 * M_PI) * sigma) * exp(-0.5 * pow(x, 2) / pow(sigma, 2)));
    }

    //Assign gaussian values to the kernel
    for (uint8_t x = {0}; x < kernel_radius; x++)
    {
        kernel[x] = kernel_values[kernel_radius - 1 - x];
    }

    kernel[kernel_radius] = (1 / (sqrt(2 * M_PI) * sigma) * exp(-0.5 * pow(0, 2) / pow(sigma, 2)));

    for (uint8_t x = kernel_radius + 1; x < kernel_width; x++)
    {
        kernel[x] = kernel_values[x - kernel_radius - 1];
    }
}

bool GaussianFilter::getGaussianKernel(float sigma, uint8_t _kernel_width) {
    if (_kernel_width>MAX_KERNEL_SIZE)
    {
        SerialMon.print("The kernel size is too big! The max size is: ");
        SerialMon.println(MAX_KERNEL_SIZE);
        return false;
    }
    else
    {
        this->calcGaussianKernel(sigma, _kernel_width);
        return true;
    }
}

void GaussianFilter::convolve1D(float _input[MAX_SENSOR_ARRAY_SIZE], float _kernel[MAX_KERNEL_SIZE], uint8_t input_width, uint8_t _kernel_width)//(input, sigma)
{
    /*Extend input_new array, with values in a way so the output have the size of the original input*/
    uint16_t input_new_width = input_width + 2 * kernel_radius;
    
    for (uint16_t i = {0}; i < input_new_width; i++)
    {
        input_new[i] = 0.0F;
    }
    //Initialise input_new array with input values in the middle
    for (uint16_t i = kernel_radius; i < input_new_width - kernel_radius; i++)
    {
        input_new[i] = _input[i-kernel_radius];  
    }
    //cout << endl;
    //Insert values of mirrored input at the beginning and at the end with length of kernel radius
    for (uint8_t i = {0}; i < kernel_radius; i++)
    {
        input_new[kernel_radius - i - 1] = _input[i];
        input_new[input_new_width - kernel_radius + i] = _input[input_width - i - 1];
    }

    uint8_t output_width = input_width;

    for (uint8_t i = {0}; i < output_width; i++)
    {
        output[i] = 0.0F;
    }

    double res = 0.0;

    //Convolution of input and kernel
    for (uint8_t j = {0}; j < output_width; j++)
    {

        for (uint8_t k = {0}; k < _kernel_width; k++)
        {
            res += input_new[j + k] * _kernel[k];
        }

        output[j] = res;
        res = 0.0;
    }
}

bool GaussianFilter::calcGausianFilter1D(float _input[MAX_SENSOR_ARRAY_SIZE], float _kernel[MAX_KERNEL_SIZE], uint8_t input_width, uint8_t _kernel_width) {
    if (input_width>MAX_SENSOR_ARRAY_SIZE)
    {
        Serial.print("input size is to big! Tha max size is: ");
        Serial.println(MAX_SENSOR_ARRAY_SIZE);
        return false;
    }
    if (_kernel_width>MAX_KERNEL_SIZE)
    {
        Serial.print("kernel size is to big! Tha max size is: ");
        Serial.println(MAX_KERNEL_SIZE);
        return false;
    }
    this->convolve1D(_input, _kernel, input_width, _kernel_width);
    return true;
    
}

