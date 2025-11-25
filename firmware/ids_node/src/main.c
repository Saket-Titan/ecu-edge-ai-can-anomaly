#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/gpio.h>
#include <math.h>
#include "model_weights.h"



LOG_MODULE_REGISTER(ecu_body, LOG_LEVEL_INF);

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg));
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),gpios);

K_MSGQ_DEFINE(can_rx_queue, sizeof(struct can_frame), 10, 4);

void can_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{ 
  k_msgq_put(&can_rx_queue, frame, K_NO_WAIT);
}


// RelU Activation 
float relu(float x){
    return x > 0 ? x : 0;
}

// sigmoid activation
float sigmoid(float x) {
    return 1.0f/(1.0f + expf(-x));
}

float anomaly_score(uint8_t *data);




int main(void)
{ 
  int err;
  int wdt_channel_id;  
  //Device check
    if(!device_is_ready(can_dev)){
  LOG_ERR("CAN device not ready");
  return -1;
}
  //watchdog check
  if(!device_is_ready(wdt)){
    LOG_ERR("watchdog device is not ready");
    return -1;
  }
  //button check
  if(!gpio_is_ready_dt(&button)){
    LOG_ERR("button device not ready");
    return -1;
  }
  //watchdog setup min_window = 0ms, max_Window = 2000ms
  struct wdt_timeout_cfg wdt_config = {
      .window.min = 0,
      .window.max = 2000,
      .callback = NULL,
      .flags = WDT_FLAG_RESET_SOC
  };

  wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
  if(wdt_channel_id < 0){
    LOG_ERR("watchdog install failed: %d", wdt_channel_id);
    return 0;

  }
  //button setup
  gpio_pin_configure_dt(&button, GPIO_INPUT);
  err = wdt_setup(wdt,0);
  if (err < 0) {
    LOG_ERR("watchdog setup failed: %d", err);
    return 0;
  }
  LOG_INF("CAN device ready: %s", can_dev->name);
    
    //starting the can 
    can_start(can_dev);
    
    struct can_filter filter = {
      .flags = CAN_FILTER_IDE ,
      .id = 0,
      .mask = 0
    };
    
    can_add_rx_filter(can_dev, &can_callback, NULL, &filter);

    struct can_filter std_filter = {
        .flags = 0, // Standard 11-bit
        .id = 0,
        .mask = 0
    };
    can_add_rx_filter(can_dev, &can_callback, NULL, &std_filter);
    
    LOG_INF("ecu_body (H723ZG) listening on CAN-FD...");

    // --- Start-> Manual UDS request sending setup --- //
    struct can_frame diag_req = {0};
    diag_req.flags = 0;
    diag_req.id = 0x7DF;
    diag_req.dlc = 8;
    diag_req.data[0] = 0x02;
    diag_req.data[1] = 0x10;
    diag_req.data[2] = 0x01;
    
    int64_t last_send_time = 0;
    struct can_frame rx_frame;

    while(1) {

      while (k_msgq_get(&can_rx_queue, &rx_frame, K_NO_WAIT)==0){
        if(rx_frame.id == 0x7E8){
          LOG_INF("UDS RESPONSE : %02X %02X %02X", rx_frame.data[0], rx_frame.data[1], rx_frame.data[2]);
        }
        else  if(rx_frame.id == 0x18F00400){ // specific check, engine mcu to avoid false negative

            float score = anomaly_score(rx_frame.data);

            if(score > ANOMALY_THRESHOLD){
                LOG_ERR("Intrusion Detected: Score: %d/1000", (int)(score*1000));
            }
            else {
                LOG_INF("Rx ID: 0x%08X Data: [%02X %02X %02X %02X %02X %02X %02X %02X]",
                    rx_frame.id,
                    rx_frame.data[0], rx_frame.data[1], rx_frame.data[2], rx_frame.data[3],
                    rx_frame.data[4], rx_frame.data[5], rx_frame.data[6], rx_frame.data[7]);
            }
          
        }
        else {
            //
        }
      }
      
      
      if(gpio_pin_get_dt(&button) != 1){
        wdt_feed(wdt,wdt_channel_id);
      }
      else {
        LOG_WRN("FAULT INJECTED: Body freeze simulated, not feeding watchdog");
      }
      LOG_INF("Body ECU running...  (Press button to simulate failure)");

      int64_t now = k_uptime_get();

      if((now - last_send_time) >= 5000){

        LOG_INF("Time Reached: Sending UDS Request...");
        int ret = can_send(can_dev, &diag_req, K_NO_WAIT, NULL,NULL);
        if(ret ==0){
          LOG_INF("UDS Request Sent Successfully");
        }
        else {
          LOG_ERR("Filed to send UDS Request: %d", ret);
        }
        last_send_time = now;

      }
      k_sleep(K_MSEC(100));
    }

    
    return 0;
    
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
