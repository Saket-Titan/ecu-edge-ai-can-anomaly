#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(ecu_body, LOG_LEVEL_INF);

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg));
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),gpios);

void can_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
    LOG_INF("Rx: ID=0x%08x DLC=%d Data=[%02X %02X]",
              frame->id, 
              frame->dlc, 
              frame->data[3], 
              frame->data[4]);
}

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
    
    LOG_INF("ecu_body (H723ZG) listening on CAN-FD...");
    
    while(1) {
      if(gpio_pin_get_dt(&button) == 0){
        wdt_feed(wdt,wdt_channel_id);
      }
      else {
        LOG_WRN("FAULT INJECTED: Body freeze simulated, not feeding watchdog");
      }
      LOG_INF("Body ECU running...  (Press button to simulate failure)");

      k_sleep(K_SECONDS(1));
    }
    return 0;
    
}


