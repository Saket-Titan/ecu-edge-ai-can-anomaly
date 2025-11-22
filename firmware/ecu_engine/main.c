#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/gpio.h> // to simulate engine failure in form of user button press

LOG_MODULE_REGISTER(ecu_engine, LOG_LEVEL_INF);

#define MAX_PHYSICAL_RPM 8000 // assumed max engine speed is 8000 RPM
#define MAX_RPM_JUMP 100 // max RPM change per 0.1 second or 100 ms
#define COUNT 20 //keeping it low, to use single button for watchdog fault inj

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg)); // to get the independent watchdog device
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),gpios);

int main(void) 
{
  int err;
  int wdt_channel_id;
    // --- Start -> Hardware Check --- //
    //Device check
  if(!device_is_ready(can_dev)){
  LOG_ERR("CAN device not ready");
  return -1;
}

  //watchdog check
  if(!device_is_ready(wdt)){
    LOG_ERR("watchdog device not ready");
    return -1;
  }
  //button check
  if(!gpio_is_ready_dt(&button)){
    LOG_ERR("button device not ready");
    return -1;
  }
  
  // --- End -> Hardware Check --- //

  //watchdog setup min_window = 0ms , max_window = 2000ms to avoid early reset as we will feed it every 1000ms
  struct wdt_timeout_cfg wdt_config = {
    .window.min = 0,
    .window.max = 2000,
    .callback = NULL, // as we don't wany any callback on timeout, but rather a hard reset
    .flags = WDT_FLAG_RESET_SOC // hard reset on timeout
  };

  wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
  if(wdt_channel_id < 0) {
    LOG_ERR("Watchdog install failed: %d", wdt_channel_id);
    return 0;

  }

  //button setup to simulate engine failure and RPM glitch
  gpio_pin_configure_dt(&button, GPIO_INPUT);

  err = wdt_setup(wdt,0);
  if (err < 0) {
    LOG_ERR("watchdog setup failed: %d", err);
    return 0;
  }


LOG_INF("CAN device ready: %s", can_dev->name);
  
  int r = can_start(can_dev);
  LOG_INF("Engine ECU (G474RE) starting... %d", r);
  
  struct can_frame frame = {0}; // to avoid any wrong data exchange
  frame.flags = CAN_FRAME_IDE; // to ensure all the 29 bits is being sent 
  frame.id = 0x18F00400; // the first two bit is to set priority 6, the middle for PGN, and the last 2 bits for the source address of engine.
  
  frame.dlc=8; // the standard data length 
  
  // frame.data[0] = 0xFF;
  // frame.data[1] = 0xFF;
  // frame.data[2] = 0xFF;
  // frame.data[3] = 0xE0;
  // frame.data[4] = 0x2E;
  // frame.data[5] = 0xFF;
  // frame.data[6] = 0xFF;
  // frame.data[7] = 0xFF;


    int current_rpm = 0;
    int previous_rpm = 0;

    int press_counter = 0;
    bool inject_glitch = false;
  
    while(1) {

      // // --- DEBUG ---
      // int btn_val = gpio_pin_get_dt(&button);
      // LOG_INF("Raw Button Value: %d", btn_val);


      // // --- Start -> Button Press For RPM glitch and ENGINE Freeze Logic  ---

      //if the button is pressed for short period than 2 seconds, we simulate a rpm glitch by randomizing rpm value within a range
      // the sleep cycle is 100ms, so 20 cycles will be 2 seconds
      

      if(gpio_pin_get_dt(&button) == 0) {
        press_counter++;
      
      //if the user button not pressed, then feeding watchdog to avoid reset
      if(press_counter > COUNT ){
        LOG_WRN("FAULT INJECTED: Engine freeze simulated, not feeding watchdog");
      }
      else {
        wdt_feed(wdt,wdt_channel_id);

      }
      }
      else {
        if(press_counter > 0 && press_counter <= COUNT){
          LOG_INF("User Input : Injecting Sensor Glitch");
          inject_glitch = true;
        }
        press_counter = 0;
        wdt_feed(wdt, wdt_channel_id);
      }

      // // --- End -> Button Press For RPM glitch and ENGINE Freeze Logic  ---

      

      if(current_rpm < 4000){
        current_rpm += 100;
      }
      else {
        current_rpm = 0;
      }
      if(inject_glitch){
        current_rpm = 25000;
        inject_glitch = false;
      }

      bool data_valid = true;

      if(current_rpm > MAX_PHYSICAL_RPM ){
        LOG_ERR("Safety Violation:  RPM %d exceeds limit", current_rpm);
        data_valid = false;
      }

      if(previous_rpm != 0 && current_rpm != 0 && (current_rpm - previous_rpm) > MAX_RPM_JUMP  ) {
        LOG_ERR("Safety Violation: RPM jump too high!");
        data_valid = false;
      }

      if(data_valid){
        uint16_t raw_rpm = (uint16_t)(current_rpm * 8);
        frame.data[3] = raw_rpm & 0xFF;
        frame.data[4] = (raw_rpm >> 8) & 0xFF;
        int ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
    if(ret == 0){
        LOG_INF("sent engine speed:%d RPM", current_rpm);
      }
    else {
      LOG_ERR("send failed with error %d", ret);
      }
      }


      previous_rpm = current_rpm;
    
    LOG_INF("Engine ECU running...  (Press button for shortly for rpm glitch injection or for 2 seconds to simulate engine failure)");
    k_sleep(K_MSEC(100));
    
    }
    return 0;

}

