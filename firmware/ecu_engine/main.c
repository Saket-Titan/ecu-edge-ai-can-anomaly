#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/drivers/gpio.h> // to simulate engine failure in form of user button press

LOG_MODULE_REGISTER(ecu_engine, LOG_LEVEL_INF);

#define MAX_PHYSICAL_RPM 8000 // assumed max engine speed is 8000 RPM
#define MAX_RPM_JUMP 100      // max RPM change per 0.1 second or 100 ms

K_MSGQ_DEFINE(uds_rx_queue, sizeof(struct can_frame), 10, 4);

static const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
static const struct device *const wdt = DEVICE_DT_GET(DT_NODELABEL(iwdg)); // to get the independent watchdog device
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

// --- Start-> UDS-Request Callback Setup --- //
static void diagnostic_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
  //broadcast id is 0x10 for UDS request
  //positive response adds 0x40

  if (frame->data[1] == 0x10) // means service request
  {
    k_msgq_put(&uds_rx_queue, frame, K_NO_WAIT);
  }
}
// --- End-> UDS-Request Callback Setup --- //

int main(void)
{
  int err;
  int wdt_channel_id;

  // --- Start -> Hardware Check --- //
  // Device check
  if (!device_is_ready(can_dev)) {
    LOG_ERR("CAN device not ready");
    return -1;
  }

  // watchdog check
  if (!device_is_ready(wdt)) {
    LOG_ERR("watchdog device not ready");
    return -1;
  }

  // button check
  if (!gpio_is_ready_dt(&button)) {
    LOG_ERR("button device not ready");
    return -1;
  }
  // --- End -> Hardware Check --- //

  // --- Start-> Watchdog setup --- //
  // watchdog setup min_window = 0ms , max_window = 2000ms to avoid early reset as we will feed it every 1000ms
  struct wdt_timeout_cfg wdt_config = {
    .window.min = 0,
    .window.max = 2000,
    .callback = NULL, // as we don't want any callback on timeout, but rather a hard reset
    .flags = WDT_FLAG_RESET_SOC // hard reset on timeout
  };

  wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
  if (wdt_channel_id < 0) {
    LOG_ERR("Watchdog install failed: %d", wdt_channel_id);
    return 0;
  }

  err = wdt_setup(wdt, 0);
  if (err < 0) {
    LOG_ERR("watchdog setup failed: %d", err);
    return 0;
  }
  // --- End-> Watchdog setup --- //

  // button setup to simulate engine failure and RPM glitch
  gpio_pin_configure_dt(&button, GPIO_INPUT);

  LOG_INF("CAN device ready: %s", can_dev->name);

  int r = can_start(can_dev);

  // --- Start-> UDS Diagnostic Filter --- //
  struct can_filter diag_filter = {
    .flags = 0,
    .id = 0x7DF,
    .mask = CAN_STD_ID_MASK
  };
  can_add_rx_filter(can_dev, &diagnostic_callback, NULL, &diag_filter);
  // --- End-> UDS Diagnostic Filter --- //

  LOG_INF("Engine ECU (G474RE) starting... %d \nListening for Diagnostics on 0x7DF... ", r);

  struct can_frame frame = {0}; // to avoid any wrong data exchange
  frame.flags = CAN_FRAME_IDE;  // to ensure all the 29 bits is being sent
  frame.id = 0x18F00400;        // the first two bit is to set priority 6, the middle for PGN, and the last 2 bits for the source address of engine.
  frame.dlc = 8;                // the standard data length

  int current_rpm = 0;
  int previous_rpm = 0;
  bool inject_glitch = false;
  struct can_frame uds_rx_frame;

  int64_t previous_time = 0;
  int64_t now_time = 0;
  int64_t duration = 0;

  while (1) {
    // --- Start->Putting UDS Response out of ISR --- //
    if (k_msgq_get(&uds_rx_queue, &uds_rx_frame, K_NO_WAIT) == 0) {
      LOG_INF("UDS Request Received :  Diagnostic Session Control");

      struct can_frame response = {0};
      response.flags = 0;
      response.id = 0x7E8;
      response.dlc = 8;

      // response format : [Len(2), Service+0x40 (positive response), Default Session(to Ping)]
      response.data[0] = 0x02;
      response.data[1] = 0x50;
      response.data[2] = 0x01;

      // send the respond back
      can_send(can_dev, &response, K_NO_WAIT, NULL, NULL);
      LOG_INF("UDS Positive Response");
    }
    // --- End->Putting UDS Response out of ISR --- //

    // --- Start -> Button Press For RPM glitch and ENGINE Freeze Logic  ---
    // if the button is pressed for short period than 2 seconds, we simulate a rpm glitch by randomizing rpm value within a range
    // the sleep cycle is 100ms, so 20 cycles will be 2 seconds

    if (gpio_pin_get_dt(&button) == 0) {
      now_time = k_uptime_get();
      if (previous_time == 0) {
        previous_time = now_time;
      }
      wdt_feed(wdt, wdt_channel_id);
    } else {
      if (previous_time != 0) {
        duration = now_time - previous_time;
        if (duration >= 2000) {
          LOG_WRN("FAULT INJECTED :  Engine Freeze, not feeding watchdog");
        } else if (duration < 2000) {
          LOG_INF("USER INPUT :  Injecting Sensor Glitch");
          inject_glitch = true;
          wdt_feed(wdt, wdt_channel_id);
        }
        previous_time = 0;
      } else {
        wdt_feed(wdt, wdt_channel_id);
      }
    }
    // --- End -> Button Press For RPM glitch and ENGINE Freeze Logic  ---

    if (current_rpm < 4000) {
      current_rpm += 100;
    } else {
      current_rpm = 0;
    }

    if (inject_glitch) {
      current_rpm = 25000;
      inject_glitch = false;
    }

    bool data_valid = true;

    if (current_rpm > MAX_PHYSICAL_RPM) {
      LOG_ERR("Safety Violation:  RPM %d exceeds limit", current_rpm);
      data_valid = false;
    }

    if (previous_rpm != 0 && current_rpm != 0 && current_rpm - previous_rpm > MAX_RPM_JUMP) {
      LOG_ERR("Safety Violation: RPM jump too high!");
      data_valid = false;
    }

    if (data_valid) {
      uint16_t raw_rpm = (uint16_t)(current_rpm * 8);
      frame.data[3] = raw_rpm & 0xFF;
      frame.data[4] = (raw_rpm >> 8) & 0xFF;
      int ret = can_send(can_dev, &frame, K_NO_WAIT, NULL, NULL);
      if (ret == 0) {
        LOG_INF("sent engine speed:%d RPM", current_rpm);
      } else {
        LOG_ERR("send failed with error %d", ret);
      }
      previous_rpm = current_rpm;
    }

    LOG_INF("Engine ECU running...  (Press button for shortly for rpm glitch injection or for 2 seconds to simulate engine failure)");
    k_sleep(K_MSEC(100));
  }

  return 0;
}
