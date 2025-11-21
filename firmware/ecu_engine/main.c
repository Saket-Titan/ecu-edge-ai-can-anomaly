#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ecu_engine, LOG_LEVEL_INF);

const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

int main(void) 
{
  if(!device_is_ready(can_dev)){
  LOG_ERR("CAN device not ready");
  return -1;
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
    int ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
    if(ret == 0){
        LOG_INF("sent engine speed:1500 RPM");
      }
    else {
      LOG_ERR("send failed with error %d", ret);
      }
    
    k_sleep(K_SECONDS(1));
    
    }
    return 0;

}

