#include "esphome.h"

class CustomThermostat : public Component, public Climate {
 public:
  Sensor *sensor;
  Switch *relay;
  float hysteresis = 0.5;

  void setup() override {
    // Restore state if needed, or default to some values
    this->target_temperature = 22.0;
    this->mode = CLIMATE_MODE_OFF;
  }

  void control(const ClimateCall &call) override {
    if (call.get_target_temperature().has_value()) {
      this->target_temperature = *call.get_target_temperature();
    }
    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
    }
    this->publish_state();
  }

  ClimateTraits traits() override {
    auto traits = ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_visual_min_temperature(5.0);
    traits.set_visual_max_temperature(35.0);
    traits.set_visual_temperature_step(0.5);
    traits.add_supported_mode(CLIMATE_MODE_OFF);
    traits.add_supported_mode(CLIMATE_MODE_HEAT);
    return traits;
  }

  void loop() override {
    // Safety check
    if (this->sensor == nullptr || this->relay == nullptr) return;

    // Update current temperature from sensor
    if (!isnan(this->sensor->state)) {
      this->current_temperature = this->sensor->state;
    }

    // Determine Action
    ClimateAction target_action = CLIMATE_ACTION_IDLE;
    
    if (this->mode == CLIMATE_MODE_OFF) {
      target_action = CLIMATE_ACTION_OFF;
    } else if (this->mode == CLIMATE_MODE_HEAT) {
      if (!isnan(this->current_temperature)) {
        if (this->current_temperature < this->target_temperature - this->hysteresis) {
          target_action = CLIMATE_ACTION_HEATING;
        } else if (this->current_temperature > this->target_temperature + this->hysteresis) {
          target_action = CLIMATE_ACTION_IDLE;
        } else {
          // Deadband - keep current action if valid
          if (this->action == CLIMATE_ACTION_HEATING) {
             target_action = CLIMATE_ACTION_HEATING;
          } else {
             target_action = CLIMATE_ACTION_IDLE;
          }
        }
      }
    }

    // Update State and Relay
    if (target_action != this->action) {
      this->action = target_action;
      
      if (this->action == CLIMATE_ACTION_HEATING) {
        this->relay->turn_on();
      } else {
        this->relay->turn_off();
      }
      this->publish_state();
    } else {
        // Redundancy: Ensure relay matches action periodically
        // (In case priority logic turned it off elsewhere, we fight back? 
        //  Actually, if manual logic runs elsewhere, we might fight. 
        //  But for now let's just assert our state).
        // For now, simpler is better.
    }
  }
};
