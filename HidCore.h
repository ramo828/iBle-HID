#pragma once

#include <Arduino.h>
#include <BLEAdvertising.h>
#include <BLECharacteristic.h>
#include <BLEServer.h>
#include <BLEService.h>

#include <functional>
#include <utility>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum class HidStatus : uint8_t {
  Ok = 0,
  AlreadyStarted,
  NotStarted,
  NotConnected,
  InvalidArgument,
  NotifyFailed,
  AdvertisingFailed,
  BluetoothUnavailable
};

struct HidConfiguration {
  uint32_t reportDelayMs = 2;
  bool securityEnabled = true;
  uint16_t advertisingMinInterval = 0;
  uint16_t advertisingMaxInterval = 0;
};

class HidCore {
 public:
  HidStatus beginGuard() {
    if (started_) {
      setError(HidStatus::AlreadyStarted);
      return lastError_;
    }
    started_ = true;
    setError(HidStatus::Ok);
    return lastError_;
  }

  void endGuard() {
    started_ = false;
    connected_ = false;
    restartPending_ = false;
    server_ = nullptr;
  }

  bool started() const { return started_; }
  bool connected() const { return connected_; }
  void setConnected(bool connected) { connected_ = connected; }
  void setServer(BLEServer* server) { server_ = server; }

  HidStatus lastError() const { return lastError_; }
  void clearError() { lastError_ = HidStatus::Ok; }

  void setError(HidStatus status) {
    lastError_ = status;
    if (loggingEnabled_ && status != HidStatus::Ok) {
      Serial.printf("[iBle HID] error=%u\n", static_cast<unsigned>(status));
    }
  }

  void setLogging(bool enabled) { loggingEnabled_ = enabled; }
  bool loggingEnabled() const { return loggingEnabled_; }

  HidConfiguration& configuration() { return configuration_; }
  const HidConfiguration& configuration() const { return configuration_; }

  void setRestartPending(bool pending) { restartPending_ = pending; }
  bool takeRestartPending() {
    if (!restartPending_) {
      return false;
    }
    restartPending_ = false;
    return true;
  }

  bool notify(BLECharacteristic* characteristic, const uint8_t* data,
              size_t length) {
    if (!started_ || !connected_) {
      setError(HidStatus::NotConnected);
      return false;
    }
    if (characteristic == nullptr || data == nullptr || length == 0) {
      setError(HidStatus::InvalidArgument);
      return false;
    }
    if (server_ == nullptr || server_->getConnectedCount() == 0) {
      setError(HidStatus::NotConnected);
      return false;
    }
    characteristic->setValue(data, length);
    characteristic->notify();
    if (configuration_.reportDelayMs != 0) {
      vTaskDelay(pdMS_TO_TICKS(configuration_.reportDelayMs));
    }
    setError(HidStatus::Ok);
    return true;
  }

  void defer(std::function<void()> callback, uint32_t delayMs) {
    if (!callback) {
      return;
    }
    DeferredWork* work = new DeferredWork{std::move(callback), delayMs};
    if (xTaskCreate(&HidCore::runDeferred, "ible_hid", 3072, work, 1,
                    nullptr) != pdPASS) {
      delete work;
      setError(HidStatus::NotifyFailed);
    }
  }

  void configureAdvertising(BLEAdvertising* advertising) const {
    if (advertising == nullptr) {
      return;
    }
    if (configuration_.advertisingMinInterval != 0) {
      advertising->setMinInterval(configuration_.advertisingMinInterval);
    }
    if (configuration_.advertisingMaxInterval != 0) {
      advertising->setMaxInterval(configuration_.advertisingMaxInterval);
    }
  }

 private:
  struct DeferredWork {
    std::function<void()> callback;
    uint32_t delayMs;
  };

  static void runDeferred(void* argument) {
    DeferredWork* work = static_cast<DeferredWork*>(argument);
    if (work->delayMs != 0) {
      vTaskDelay(pdMS_TO_TICKS(work->delayMs));
    }
    work->callback();
    delete work;
    vTaskDelete(nullptr);
  }

  HidConfiguration configuration_;
  HidStatus lastError_ = HidStatus::Ok;
  bool started_ = false;
  bool connected_ = false;
  bool loggingEnabled_ = false;
  bool restartPending_ = false;
  BLEServer* server_ = nullptr;
};
