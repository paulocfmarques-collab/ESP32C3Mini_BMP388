#include "WatchdogManager.h"

#include <esp_idf_version.h>
#include <esp_task_wdt.h>

bool WatchdogManager::begin(uint32_t timeoutSeconds) {
  if (timeoutSeconds == 0) {
    return false;
  }

  esp_err_t result;
#if ESP_IDF_VERSION_MAJOR >= 5
  esp_task_wdt_config_t config = {};
  config.timeout_ms = timeoutSeconds * 1000UL;
  config.idle_core_mask = 0;
  config.trigger_panic = true;

  const esp_err_t initialTaskStatus = esp_task_wdt_status(NULL);
  if (initialTaskStatus == ESP_ERR_INVALID_STATE) {
    result = esp_task_wdt_init(&config);
  } else if (initialTaskStatus == ESP_OK ||
             initialTaskStatus == ESP_ERR_NOT_FOUND) {
    result = esp_task_wdt_reconfigure(&config);
  } else {
    return false;
  }
#else
  result = esp_task_wdt_init(timeoutSeconds, true);
  if (result == ESP_ERR_INVALID_STATE) {
    const esp_err_t taskStatus = esp_task_wdt_status(NULL);
    const bool taskWasSubscribed = taskStatus == ESP_OK;
    if (!taskWasSubscribed && taskStatus != ESP_ERR_NOT_FOUND) {
      return false;
    }

    if (taskWasSubscribed && esp_task_wdt_delete(NULL) != ESP_OK) {
      return false;
    }

    result = esp_task_wdt_deinit();
    if (result != ESP_OK) {
      if (taskWasSubscribed) {
        esp_task_wdt_add(NULL);
      }
      return false;
    }
    result = esp_task_wdt_init(timeoutSeconds, true);
  }
#endif

  if (result != ESP_OK) {
    return false;
  }

  const esp_err_t taskStatus = esp_task_wdt_status(NULL);
  if (taskStatus == ESP_ERR_NOT_FOUND) {
    if (esp_task_wdt_add(NULL) != ESP_OK) {
      return false;
    }
  } else if (taskStatus != ESP_OK) {
    return false;
  }

  enabled_ = true;
  return true;
}

bool WatchdogManager::feed() {
  return enabled_ && esp_task_wdt_reset() == ESP_OK;
}
