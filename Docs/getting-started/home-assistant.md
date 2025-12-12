# Home Assistant Integration

Add this template climate entity to your `configuration.yaml`:

```yaml
template:
  - climate:
      - name: "Propane Stove Thermostat"
        unique_id: gazebo_propane_stove_thermostat
        current_temperature: "{{ states('sensor.gazebo_temperature') | float(20) }}"
        target_temperature: "{{ states('number.desired_temperature') | float(22) }}"
        target_temperature_step: 0.5
        min_temp: 5
        max_temp: 35
        temperature_unit: C
        hvac_modes: ["heat", "off"]
        preset_modes: ["home", "away"]
        preset_mode: "{{ 'home' if states('select.presence_mode') == 'home' else 'away' }}"
        set_temperature:
          service: number.set_value
          target:
            entity_id: number.desired_temperature
          data:
            value: "{{ temperature }}"
        set_preset_mode:
          service: select.select_option
          target:
            entity_id: select.presence_mode
          data:
            option: "{{ preset_mode }}"
        hvac_action: >-
          {% if states('switch.propane_stove_relay') == 'on' %}
            heating
          {% elif states('select.presence_mode') == 'away' %}
            off
          {% elif states('switch.manual_stop') == 'on' %}
            off
          {% else %}
            idle
          {% endif %}
```

## Required ESPHome Entities

| Entity | Purpose |
|--------|---------|
| `sensor.gazebo_temperature` | Current temperature reading |
| `number.desired_temperature` | Target temperature setpoint |
| `select.presence_mode` | Home/Away mode selector |
| `switch.propane_stove_relay` | Heating relay state |
| `switch.manual_stop` | Force off switch |

## Setup Steps

1. Add template entity to `configuration.yaml`
2. Restart Home Assistant
3. Find `climate.propane_stove_thermostat` in Devices & Services
4. Test temperature slider and verify relay responds

> **Note**: ESPHome handles scheduling. No HA helpers (`input_boolean`, `input_datetime`) needed.
