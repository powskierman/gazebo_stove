# Your Home Assistant Configuration Review

**Date**: November 15, 2025
**Device**: GazeboStove ESPHome Heating Control
**Status**: ✅ Ready to integrate

---

## Summary

Your Home Assistant configuration is well-structured and secure. The integration with GazeboStove requires **minimal changes**:

### ✅ What to Keep (No Changes Needed)
- All your existing configurations
- Alexa integration
- Light groups
- Time/date sensors
- Wake-up light helpers
- Sunset light automation
- SSL/TLS settings
- MQTT integration
- Logger configuration
- Entity controller

### ✅ What to Add (New Section)
- One template climate entity for the thermostat UI
- Comprehensive comments explaining the GazeboStove integration

### ❌ What NOT to Add (Important!)
- **NO** input_boolean for schedule enabled
- **NO** input_datetime for schedule times
- **NO** Home Assistant automations for scheduling

---

## Why the Old HA-Based Approach is Not Recommended

Your configuration likely had these old schedule helpers (they're now obsolete):

```yaml
# ❌ DO NOT ADD THESE - ESPHome handles scheduling now
input_boolean:
  gazebo_schedule_enabled:  # NOT NEEDED
    name: "Gazebo Heating Schedule"

input_datetime:
  gazebo_home_mode_time:     # NOT NEEDED
    name: "Gazebo Home Mode Time"
  gazebo_away_mode_time:     # NOT NEEDED
    name: "Gazebo Away Mode Time"
```

**Why these are obsolete**:
- ESPHome now handles all scheduling natively
- Device works independently without Home Assistant
- Schedule times are configured via ESPHome number entities
- No HA helpers needed, reducing complexity
- More reliable (no HA dependency)

---

## What to Add: The Thermostat Template Entity

Add this single section to your configuration.yaml (after your input_boolean helpers):

```yaml
# ============================================================================
# GAZEBO STOVE HEATING CONTROL - ESPHome Integration
# ============================================================================

template:
  - climate:
      - name: "Propane Stove Thermostat"
        unique_id: gazebo_propane_stove_thermostat

        # Current temperature from ESPHome sensor
        current_temperature: "{{ states('sensor.gazebo_temperature') | float(20) }}"

        # Target temperature from ESPHome number entity
        target_temperature: "{{ states('number.desired_temperature') | float(22) }}"

        # UI settings
        target_temperature_step: 0.5
        min_temp: 5
        max_temp: 35
        temperature_unit: C

        # Heating-only system (propane stove cannot cool)
        hvac_modes:
          - "heat"
          - "off"

        # Preset modes (mapped from ESPHome presence_mode)
        preset_modes:
          - "home"
          - "away"
        preset_mode: "{{ 'home' if states('select.presence_mode') == 'home' else 'away' }}"

        # When user adjusts setpoint slider
        set_temperature:
          service: number.set_value
          target:
            entity_id: number.desired_temperature
          data:
            value: "{{ temperature }}"

        # When user changes preset mode (home/away)
        set_preset_mode:
          service: select.select_option
          target:
            entity_id: select.presence_mode
          data:
            option: "{{ preset_mode }}"

        # Visual indicator of heating status
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

---

## Integration Steps

### Step 1: Copy the Template Entity
Copy the template climate entity above into your configuration.yaml file.

**Location**: Place it after your input_boolean helpers and before the `tts:` section

### Step 2: Verify Entity IDs
Check that your ESPHome device provides these entities:

```
sensor.gazebo_temperature        → Temperature sensor reading
number.desired_temperature       → Setpoint (user adjustable)
select.presence_mode             → home/away mode
switch.propane_stove_relay       → Relay control (heating on/off)
switch.manual_stop               → Force off switch
```

**If entity names differ**, update the entity IDs in the template.

### Step 3: Restart Home Assistant
After editing configuration.yaml:
1. Go to Settings → System → Logs
2. Check for any YAML syntax errors
3. If OK: Settings → System → Restart Home Assistant
4. Wait 1-2 minutes for restart

### Step 4: Verify in UI
After restart:
1. Go to Settings → Devices & Services → Entities
2. Search for "thermostat" or "Propane Stove"
3. Should see: `climate.propane_stove_thermostat`
4. Click it to see the thermostat card

---

## What You Can Do Now

### Via Home Assistant UI
Once integrated, you can:

✅ **View Temperature**
- Current room temperature (from sensor)
- Target temperature (your desired setpoint)
- Heating status (heating/idle/off)

✅ **Adjust Setpoint**
- Slide the temperature slider
- Changes made via HA are sent to ESPHome
- Relay responds within seconds

✅ **Switch Presence Mode**
- Toggle between Home and Away mode
- Home: Thermostat controls heating
- Away: Heating disabled (standby)

✅ **Access Manual Controls**
- Devices & Services → Entities
- Search for "gazebo" or "stove"
- Available controls:
  - switch.manual_run (Emergency Heat, 15-min timeout)
  - switch.manual_stop (Force Off)
  - number.home_mode_time_hour/minute
  - number.away_mode_time_hour/minute
  - switch.heating_schedule (enable/disable)

### Via Automations
You can create HA automations that:
- Monitor `sensor.gazebo_temperature`
- React to `switch.propane_stove_relay` state changes
- Trigger notifications based on temperature
- Integrate with voice assistants
- etc.

---

## Entity Reference

### Temperature & Control
| Entity | Purpose | Example Value |
|--------|---------|----------------|
| `sensor.gazebo_temperature` | Current room temperature | 22.5°C |
| `number.desired_temperature` | Desired temperature setpoint | 22°C (adjustable 5-35°C) |
| `switch.propane_stove_relay` | Relay control (heating on/off) | on/off |

### Schedule Control
| Entity | Purpose | Example Value |
|--------|---------|----------------|
| `switch.heating_schedule` | Enable/disable schedule | on/off |
| `number.home_mode_time_hour` | Home mode hour (0-23) | 6 (6 AM) |
| `number.home_mode_time_minute` | Home mode minute (0-59) | 0 |
| `number.away_mode_time_hour` | Away mode hour (0-23) | 22 (10 PM) |
| `number.away_mode_time_minute` | Away mode minute (0-59) | 0 |

### Presence & Mode
| Entity | Purpose | Example Value |
|--------|---------|----------------|
| `select.presence_mode` | Current mode | home/away |

### Manual Overrides
| Entity | Purpose | Example Value |
|--------|---------|----------------|
| `switch.manual_run` | Emergency heat (15-min) | on/off |
| `switch.manual_stop` | Force off (highest priority) | on/off |

### Calibration
| Entity | Purpose | Example Value |
|--------|---------|----------------|
| `number.temperature_offset` | Sensor calibration (-5 to +5°C) | 0°C |
| `number.heating_hysteresis` | Temperature deadband | 2°C |

---

## Important Notes

### Schedule Configuration
The schedule is **NOT** configured in Home Assistant anymore. To change schedule times:

**Option 1: Via Home Assistant UI (Easiest)**
1. Settings → Devices & Services → Entities
2. Search for "home_mode_time" or "away_mode_time"
3. Click the number entity
4. Adjust hour/minute values
5. Changes take effect immediately (no reboot needed)

**Option 2: Via ESPHome Dashboard**
1. Go to ESPHome installation
2. Select your device
3. Edit entity values
4. Changes take effect immediately

**Option 3: Direct YAML (requires recompile)**
1. Edit includes/entities.yaml in ESPHome
2. Change number entity default values
3. Recompile and upload

### Thermostat Priority System

The thermostat UI shows the status, but the actual control logic runs in ESPHome with a 4-level priority system:

```
Priority 1: Manual Stop (Force Off) ← Highest
  └─ Blocks all heating

Priority 2: Manual Run (Emergency Heat)
  └─ Forces heating for 15 minutes

Priority 3: Presence Mode (Schedule)
  └─ Away mode = no heat, Home mode = allow thermostat

Priority 4: Thermostat Control
  └─ Temperature-based on/off control
```

The thermostat card you're adding displays this, but doesn't implement it. That's by design - ESPHome handles it reliably.

---

## Common Issues & Troubleshooting

### Issue: "Thermostat not appearing in UI"
**Cause**: Template entity not found or syntax error
**Fix**:
1. Check for YAML syntax errors (Settings → System → Logs)
2. Verify entity IDs exist (Settings → Devices & Services → Entities)
3. Restart Home Assistant (Settings → System → Restart)

### Issue: "Setpoint changes don't affect temperature"
**Cause**: Number entity ID wrong or ESPHome not receiving updates
**Fix**:
1. Verify entity IDs in template (search in Entities list)
2. Check ESPHome logs: `esphome logs gazebo_stove.yaml`
3. Ensure ESPHome API is connected to HA

### Issue: "Schedule not changing mode at scheduled time"
**Cause**: Schedule is ESPHome-based, not HA-based
**Fix**:
1. Check ESPHome logs for "Schedule: Switching to..."
2. Verify schedule is enabled: `switch.heating_schedule` = ON
3. Verify schedule times are set: number.home_mode_time_hour, etc.
4. Device doesn't need HA for schedule to work

### Issue: "Temperature reading is off"
**Cause**: Sensor needs calibration
**Fix**:
1. Adjust `number.temperature_offset` in Home Assistant
2. -5 to +5°C range
3. Example: If reading is 2°C too high, set offset to -2°C
4. Takes effect immediately

---

## Next Steps

1. **Copy** the template climate entity to your configuration.yaml
2. **Verify** entity IDs match your ESPHome setup
3. **Restart** Home Assistant
4. **Test** by adjusting setpoint and watching temperature change
5. **Enjoy** your integrated heating control system

---

## File: Modified configuration.yaml

A complete modified configuration.yaml is provided in `/tmp/configuration_modified.yaml` ready to use.

**Changes made**:
- Added comprehensive comments
- Added template climate entity
- Organized Gazebo Stove section
- Added helpful notes and entity reference

**What stays the same**:
- All your existing configuration
- Alexa, lights, automations, scripts
- Logging, MQTT, entity controller

---

## Support & Documentation

For more detailed information, see:

- **[README.md](https://github.com/your-repo/README.md)** - Project overview
- **[HASS_IMPLEMENTATION_GUIDE.md](https://github.com/your-repo/HASS_IMPLEMENTATION_GUIDE.md)** - Complete HA integration guide
- **[EVENT_DRIVEN_PRIORITY_SYSTEM.md](https://github.com/your-repo/EVENT_DRIVEN_PRIORITY_SYSTEM.md)** - How the priority system works

---

## Summary

✅ **Your configuration is well-structured**

✅ **Minimal changes needed** (just one template entity)

✅ **No complex helpers required** (schedule is ESPHome-based)

✅ **Fully documented** for future reference

✅ **Ready to integrate** immediately

Start with the template entity and you're done!

---

*Review Date: November 15, 2025*
*Status: Ready for Integration*
