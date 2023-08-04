#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "ar1021.h"

namespace esphome {
namespace ar1021 {

static const char *const TAG = "ar1021.component";

static const uint8_t TOUCH_THRESHOLD = 0x02;
static const uint8_t SENSITIVITY_FILTER = 0x03;
static const uint8_t SAMPLING_FAST = 0x04;
static const uint8_t SAMPLING_SLOW = 0x05;
static const uint8_t ACCURACY_FILTER_FAST = 0x06;
static const uint8_t ACCURACY_FILTER_SLOW = 0x07;
static const uint8_t SPEED_THRESHOLD = 0x08;
static const uint8_t SLEEP_DELAY = 0x0A;
static const uint8_t PEN_UP_DELAY = 0x0B;
static const uint8_t TOUCH_MODE = 0x0C;
static const uint8_t TOUCH_OPTIONS = 0x0D;
static const uint8_t CALIBRATION_INSET = 0x0E;
static const uint8_t PEN_STATE_REPORT_DELAY = 0x0F;
static const uint8_t TOUCH_REPORT_DELAY = 0x11;


static const uint8_t GET_VERSION[4] = {0x00, 0x55, 0x01, 0x10};
static const uint8_t ENABLE_TOUCH[4] = {0x00, 0x55, 0x01, 0x12};
static const uint8_t DISABLE_TOUCH[4] = {0x00, 0x55, 0x01, 0x13};
static const uint8_t CALIBRATE_MODE[5] = {0x00, 0x55, 0x02, 0x14, 0x04};
static const uint8_t REGISTER_READ[4] = {0x00, 0x55, 0x01, 0x20};
static const uint8_t REGISTER_WRITE[4] = {0x00, 0x55, 0x01, 0x21};
static const uint8_t REGISTER_START_ADDRESS_REQUEST[4] = {0x00, 0x55, 0x01, 0x22};
static const uint8_t REGISTERS_WRITE_TO_EEPROM[4] = {0x00, 0x55, 0x01, 0x23};
static const uint8_t EEPROM_READ[4] = {0x00, 0x55, 0x01, 0x28};
static const uint8_t EEPROM_WRITE[4] = {0x00, 0x55, 0x01, 0x29};
static const uint8_t EEPROM_WRITE_TO_REGISTERS[4] = {0x00, 0x55, 0x01, 0x2B};

uint8_t calstate = 0;
static const uint8_t CALCONFIRM[4] = {0x55, 0x02, 0x00, 0x14};

#define ERROR_CHECK(err) \
  if ((err) != i2c::ERROR_OK) { \
    ESP_LOGE(TAG, "Failed to communicate!"); \
    this->status_set_warning(); \
    return; \
  }

void Store::gpio_intr(Store *store) { store->touch = true; }

float AR1021Component::get_setup_priority() const { return setup_priority::HARDWARE; }

void AR1021Component::setup() {
    uint8_t buffer[4] = {0};
    ESP_LOGCONFIG(TAG, "Setting up AR1021 touchscreen...");
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLDOWN);
    this->interrupt_pin_->setup();
    this->store_.pin = this->interrupt_pin_->to_isr();
    this->interrupt_pin_->attach_interrupt(Store::gpio_intr, &this->store_, gpio::INTERRUPT_RISING_EDGE);

    if (this->write(nullptr, 0) != i2c::ERROR_OK) {
        ESP_LOGE(TAG, "Failed to communicate!");
        this->interrupt_pin_->detach_interrupt();
        this->mark_failed();
        return; 
    }

    this->write(ENABLE_TOUCH, 4);

    i2c::ErrorCode err;
    err = this->read(buffer, 4);
    ERROR_CHECK(err);
    //fixup actually check success, 0x55 0x02 0x00 0x12
}

void AR1021Component::loop() {
  
  uint8_t buffer[4] = {0};
  i2c::ErrorCode err;
  std::string out = {};
  TouchPoint tp;
  
  switch (calstate) {
    case 1:
      this->write(DISABLE_TOUCH, 4);
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      this->write(CALIBRATE_MODE, 5);
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      calstate = 2;
      ESP_LOGD(TAG, "start calibration");
      return;
    case 2:
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      if (std::equal(std::begin(buffer), std::end(buffer), std::begin(CALCONFIRM))) {
        calstate = 3;
        tp.id = 0;
        tp.state = 3;
        tp.x = 0;
        tp.y = 0;
        this->defer([this, tp]() { this->send_touch_(tp); });
        ESP_LOGD(TAG, "calibration 1");
      }
      return;
    case 3:
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      if (std::equal(std::begin(buffer), std::end(buffer), std::begin(CALCONFIRM))) {
        calstate = 4;
        tp.id = 0;
        tp.state = 4;
        tp.x = 0;
        tp.y = 0;
        this->defer([this, tp]() { this->send_touch_(tp); });
        ESP_LOGD(TAG, "calibration 2");
      }
      return;
    case 4:
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      if (std::equal(std::begin(buffer), std::end(buffer), std::begin(CALCONFIRM))) {
        calstate = 5;
        tp.id = 0;
        tp.state = 5;
        tp.x = 0;
        tp.y = 0;
        this->defer([this, tp]() { this->send_touch_(tp); });
        ESP_LOGD(TAG, "calibration 3");
      }
      return;
    case 5:
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      if (std::equal(std::begin(buffer), std::end(buffer), std::begin(CALCONFIRM))) {
        calstate = 6;
        tp.id = 0;
        tp.state = 6;
        tp.x = 0;
        tp.y = 0;
        this->defer([this, tp]() { this->send_touch_(tp); });
        ESP_LOGD(TAG, "calibration 4");
      }
      return;
    case 6:
      err = this->read(buffer, 4);
      ERROR_CHECK(err);
      out = format_hex_pretty(buffer, 4);
      ESP_LOGD(TAG, "wait completion: %s", out);
      if (std::equal(std::begin(buffer), std::end(buffer), std::begin(CALCONFIRM))) {
        calstate = 0;
        ESP_LOGD(TAG, "calibration complete");
        this->write(ENABLE_TOUCH, 4);
        err = this->read(buffer, 4);
        ERROR_CHECK(err);
      }
      return;
  }

  if (!this->store_.touch)
    return;
  this->store_.touch = false;

  uint8_t point = 0;
  uint8_t bigbuff[5] = {0};
  uint32_t sum_l = 0, sum_h = 0;

  err = this->read(bigbuff, 5);
  ERROR_CHECK(err);
 
  tp.id = bigbuff[0];
  tp.state = 0x09;
  out = format_hex_pretty(bigbuff, 5);
  ESP_LOGD(TAG, "raw touch: %s", out);

  uint16_t y = (uint16_t) ((bigbuff[4] << 7) | (bigbuff[3]));
  uint16_t x = (uint16_t) ((bigbuff[2] << 7) | (bigbuff[1]));

  y = (y * this->display_->get_height()) / 0xfff;
  x = (x * this->display_->get_width()) / 0xfff;

  ESP_LOGD(TAG, "touch x %d", x);
  ESP_LOGD(TAG, "touch y %d", y);

  switch (this->rotation_) {
    case ROTATE_0_DEGREES:
      tp.y = y;
      tp.x = x;
      break;
    case ROTATE_90_DEGREES:
      tp.x = this->display_height_ - y;
      tp.y = x;
      break;
    case ROTATE_180_DEGREES:
      tp.y = this->display_height_ - y;
      tp.x = this->display_width_ - x;
      break;
    case ROTATE_270_DEGREES:
      tp.x = y;
      tp.y = this->display_width_ - x;
      break;
  }

  this->defer([this, tp]() { this->send_touch_(tp); });

  this->status_clear_warning();  
}

void AR1021Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AR1021 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("Interrupt Pin: ", this->interrupt_pin_);
}

void AR1021Component::calibrate() {
  calstate = 1;
}

} //namespace ar1021
} //namespace esphome