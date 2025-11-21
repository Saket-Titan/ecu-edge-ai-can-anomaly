#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/gpio.h> // to simulate engine failure in form of user button press

LOG_MODULE_REGISTER(ecu_engine, LOG_LEVEL_INF);

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg)); // to get the independent watchdog device
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),gpios);

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
    LOG_ERR("watchdog device not ready");
    return -1;
  }
  //button check
  if(!gpio_is_ready_dt(&button)){
    LOG_ERR("button device not ready");
    return -1;
  }

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

  //button setup to simulate engine failure
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
  
  frame.data[0] = 0xFF;
  frame.data[1] = 0xFF;
  frame.data[2] = 0xFF;
  frame.data[3] = 0xE0;
  frame.data[4] = 0x2E;
  frame.data[5] = 0xFF;
  frame.data[6] = 0xFF;
  frame.data[7] = 0xFF;
  
    while(1) {

      // // --- DEBUG ---
      // int btn_val = gpio_pin_get_dt(&button);
      // LOG_INF("Raw Button Value: %d", btn_val);

      //if the user button not pressed, then feeding watchdog to avoid reset
      if(gpio_pin_get_dt(&button) == 1){
        wdt_feed(wdt,wdt_channel_id);
      }
      else {
        LOG_WRN("FAULT INJECTED: Engine freeze simulated, not feeding watchdog");
      }
    
    int ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
    if(ret == 0){
        LOG_INF("sent engine speed:1500 RPM");
      }
    else {
      LOG_ERR("send failed with error %d", ret);
      }
    LOG_INF("Engine ECU running...  (Press button to simulate failure)");
    k_sleep(K_SECONDS(1));
    
    }
    return 0;

}

