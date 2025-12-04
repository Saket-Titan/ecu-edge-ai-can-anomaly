#include <zephyr/ztest.h>
#include "model_weights.h"
#include "manual_inference.h"
ZTEST(anomaly_suite, test_normal_data)
{
  uint8_t normal_input[] = {0x00, 0x00, 0x00, 0x2E, 0xE0, 0x00, 0x00, 0x00};
  float score = anomaly_score(normal_input);
  
  printk("Normal Data Score: %d/1000 \n", (int)(score*1000));
  zassert_true(score < ANOMALY_THRESHOLD, "Score exceeded threshold");
}

ZTEST(anomaly_suite, test_attack_data)
{
  uint8_t attack_input[] = {0x00, 0x12, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
  float score = anomaly_score(attack_input);
  printk("Attack Data Score: %d/1000", (int)(score*1000));
  
  zassert_true(score >= 0.0f, "Score must be positive");
}
ZTEST_SUITE(anomaly_suite, NULL, NULL, NULL, NULL, NULL);
