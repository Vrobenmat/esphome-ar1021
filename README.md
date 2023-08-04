# esphome-ar1021
ESPHome custom component to support the AR1021 touch controller over SPI. Interrupt driven and support for calibration and backlight control. Assumes calibration point register is default (12.5%) Very hacky at the moment and needs a clean, but it works with an Olimex MOD-LCD2 with SJ2 cut & soldered and SJ3 soldered.

Place in your ESPHome root directory (/config/esphome for Home Assitant) under ./custom_components/ar1021

Minimal example YAML looks like this:

globals:
  - id: tscal
    type: int
  - id: tscalstate
    type: int
  
i2c:
  sda: GPIOx
  scl: GPIOx
  scan: true
  frequency: 10kHz
   
display:
  - platform: x
    id: lcd
    model: x
    dimensions: XxY
    cs_pin: x
    dc_pin: x
    pages:
      - id: home
        lambda: |-
          it.fill(my_black);
      - id: cal1
        lambda: |-
          auto blue = Color(0, 0, 255);
          auto red = Color(255, 0, 0);
          auto green = Color(0, 255, 0);
          auto black = Color(0, 0, 0);
          it.fill(black);
          it.filled_circle(30, 30, 10, red);
          it.filled_circle(210, 30, 10, blue);
          it.filled_circle(210, 290, 10, blue);
          it.filled_circle(30, 290, 10, blue);
          it.print(120, 160, id(my_font), red, TextAlign::CENTER, "Calibration: Touch 1");
          it.print(30, 30, id(my_font), black, TextAlign::CENTER, "1");
          it.rectangle(50, 150, 140, 20, blue);
      - id: cal2
        lambda: |-
          auto blue = Color(0, 0, 255);
          auto red = Color(255, 0, 0);
          auto green = Color(0, 255, 0);
          auto black = Color(0, 0, 0);
          it.fill(black);
          it.filled_circle(30, 30, 10, green);
          it.filled_circle(210, 30, 10, red);
          it.filled_circle(210, 290, 10, blue);
          it.filled_circle(30, 290, 10, blue);
          it.print(120, 160, id(my_font), red, TextAlign::CENTER, "Calibration: Touch 2");
          it.print(210, 30, id(my_font), black, TextAlign::CENTER, "2");
          it.rectangle(50, 150, 140, 20, blue);
      - id: cal3
        lambda: |-
          auto blue = Color(0, 0, 255);
          auto red = Color(255, 0, 0);
          auto green = Color(0, 255, 0);
          auto black = Color(0, 0, 0);
          it.fill(black);
          it.filled_circle(30, 30, 10, green);
          it.filled_circle(210, 30, 10, green);
          it.filled_circle(210, 290, 10, red);
          it.filled_circle(30, 290, 10, blue);
          it.print(120, 160, id(my_font), red, TextAlign::CENTER, "Calibration: Touch 3");
          it.print(210, 290, id(my_font), black, TextAlign::CENTER, "3");
          it.rectangle(50, 150, 140, 20, blue);
      - id: cal4
        lambda: |-
          auto blue = Color(0, 0, 255);
          auto red = Color(255, 0, 0);
          auto green = Color(0, 255, 0);
          auto black = Color(0, 0, 0);
          it.fill(black);
          it.filled_circle(30, 30, 10, green);
          it.filled_circle(210, 30, 10, green);
          it.filled_circle(210, 290, 10, green);
          it.filled_circle(30, 290, 10, red);
          it.print(120, 160, id(my_font), red, TextAlign::CENTER, "Calibration: Touch 4");
          it.print(30, 290, id(my_font), black, TextAlign::CENTER, "4");
          it.rectangle(50, 150, 140, 20, blue);

font:
  - file:
      type: gfonts
      family: Roboto
      weight: 300
    id: my_font
    size: 14

switch:
  - platform: gpio
    id: backlight
    name: "LCD Backlight"
    restore_mode: ALWAYS_ON
    pin: #details below depend on platform
      number: GPIOx
      inverted: true
      mode:
        output: true
        pullup: true

touchscreen:
  - platform: ar1021
    id: ar1021ts
    interrupt_pin: GPIO36
    on_touch:
      then:
        - lambda: |-
            id(tscal) = touch.id;
            id(tscalstate) = touch.state;
        - if:
            condition:
              lambda: return id(tscal) == 0;
            then:
              - lambda: ESP_LOGD("ar1021.cal", "%d", id(tscal));
              - display.page.show_next: lcd
              - component.update: lcd

button:
  - platform: template
    id: calibrate
    name: "Calibration Mode"
    on_press:
      then:
        - display.page.show: cal1
        - component.update: lcd
        - lambda: |-
            id(ar1021ts).calibrate();
