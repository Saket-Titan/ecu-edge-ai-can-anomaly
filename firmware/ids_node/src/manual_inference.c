#include <math.h>
#include <zephyr/types.h>
#include "manual_inference.h"
#include "model_weights.h"

// RelU Activation 
static float relu(float x){
    return x > 0 ? x : 0;
}

// sigmoid activation
static float sigmoid(float x) {
    return 1.0f/(1.0f + expf(-x));
}

float anomaly_score(uint8_t *data){


    //input normalization
    float input[8];
    for(int i = 0; i < 8; i++){
        input[i] = (float) data[i]/255.0f;

    }

    //encoder : putting the 8 bits into the 4 bits
    float  hidden[4];
    for(int h = 0; h<4; h++){
        hidden[h] = layer_0_b[h];
        for(int i=0; i<8; i++){
            hidden[h] += input[i] * layer_0_w[(i*4)+h];
        }
        hidden[h] = relu(hidden[h]);
    }

    float output[8];
    for (int j = 0; j < 8; j++){
        output[j] = layer_1_b[j];
        for(int h = 0; h < 4; h++){
            output[j] += hidden[h]*layer_1_w[(h*8)+j];
        }
        output[j] = sigmoid(output[j]);
    }

    float total_error = 0.0f;
    for(int i = 0; i < 8; i++){
        total_error += fabsf(input[i] - output[i]);
    }
    float avg_error = total_error/8.0f;
    return avg_error;
}
