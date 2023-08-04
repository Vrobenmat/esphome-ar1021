#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ar1021 {

using namespace touchscreen;

struct Store {
  volatile bool touch;
  ISRInternalGPIOPin pin;

  static void gpio_intr(Store *store);
};

class AR1021Component : public Touchscreen, public Component, public i2c::I2CDevice {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;
    void calibrate();
    float get_setup_priority() const override;

    void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }

  protected:
    InternalGPIOPin *interrupt_pin_;
    Store store_;
};

} //namespace ar1021
} //namespace esphome