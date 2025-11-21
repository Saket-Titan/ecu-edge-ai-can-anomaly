#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ecu_body, LOG_LEVEL_INF);

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

void can_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
    LOG_INF("Rx: ID=0x%08x DLC=%d Data=[%02X %02X]",
              frame->id, frame->dlc, frame->data[3], frame->data[4]);
}

int main(void)
{   //Device check
    if(!device_is_ready(can_dev)){
  LOG_ERR("CAN device not ready");
  return -1;
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
      k_sleep(K_SECONDS(1));
    }
    return 0;
    
}


