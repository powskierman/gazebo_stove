# Quickstart: Gazebo Propane Stove Heating Control System

**Date**: 2025-01-27  
**Feature**: 001-gazebo-stove-heating-control

## Prerequisites

- ESP32 device (esp32doit-devkit-v1) with ESPHome firmware
- DS18B20 temperature sensor connected to GPIO4
- Relay connected to GPIO12
- Nextion display connected to GPIO16/17 (optional)
- Home Assistant instance with ESPHome integration
- WiFi network configured

## Initial Setup

### 1. ESPHome Configuration

```bash
# Compile and upload configuration
esphome compile gazebo_stove.yaml
esphome upload gazebo_stove.yaml
```

### 2. Home Assistant Integration

ESPHome device should auto-discover in Home Assistant. If not:

1. Go to Settings → Devices & Services
2. Add Integration → ESPHome
3. Enter device IP or use discovery
4. Enter API password (from secrets.yaml)

### 3. Verify Entities

Check that these entities appear in Home Assistant:

- `climate.propane_stove_thermostat`
- `number.desired_temperature`
- `number.heating_hysteresis`
- `select.presence_mode`
- `switch.manual_run`
- `switch.manual_stop`
- `sensor.gazebo_temperature`

## Test Scenarios

### Scenario 1: Basic Thermostat Control

**Goal**: Verify automatic temperature control works

**Steps**:
1. Set `select.presence_mode` to "home"
2. Set `number.desired_temperature` to 25°C
3. Observe `sensor.gazebo_temperature` reading
4. If current temp < 25°C, verify `switch.propane_stove_relay` turns ON
5. Wait for temperature to rise above (25°C + hysteresis)
6. Verify relay turns OFF

**Expected Result**: Relay activates/deactivates based on temperature vs setpoint

**Success Criteria**: SC-001, SC-002

---

### Scenario 2: Force Off Override

**Goal**: Verify Force Off has highest priority

**Steps**:
1. Set `select.presence_mode` to "home"
2. Set `number.desired_temperature` to 25°C (if current < 25°C, relay should be ON)
3. Turn ON `switch.manual_stop` (Force Off)
4. Verify `switch.propane_stove_relay` turns OFF immediately
5. Attempt to turn ON `switch.manual_run` (Emergency Heat)
6. Verify relay stays OFF (Emergency Heat blocked)

**Expected Result**: Force Off blocks all heating, including Emergency Heat

**Success Criteria**: FR-011, FR-011a

---

### Scenario 3: Emergency Heat Timeout

**Goal**: Verify 15-minute timeout works

**Steps**:
1. Ensure `switch.manual_stop` is OFF
2. Turn ON `switch.manual_run` (Emergency Heat)
3. Verify `switch.propane_stove_relay` turns ON
4. Wait 15 minutes
5. Verify relay turns OFF automatically
6. Verify `switch.manual_run` turns OFF

**Expected Result**: Emergency Heat auto-disables after exactly 15 minutes

**Success Criteria**: SC-003, FR-009

---

### Scenario 4: Schedule Control

**Goal**: Verify time-based schedule works

**Steps**:
1. Set `input_boolean.gazebo_heating_schedule` to ON
2. Set `input_datetime.home_mode_time` to current time + 1 minute
3. Set `input_datetime.away_mode_time` to current time + 2 minutes
4. Set `select.presence_mode` to "away"
5. Wait for home_mode_time
6. Verify `select.presence_mode` changes to "home" automatically
7. Verify relay activates (if temp < setpoint)
8. Wait for away_mode_time
9. Verify `select.presence_mode` changes to "away" automatically
10. Verify relay turns OFF

**Expected Result**: Schedule automations switch presence mode at configured times

**Success Criteria**: SC-013, FR-003

---

### Scenario 5: Schedule Disabled Behavior

**Goal**: Verify schedule disabled respects Home/Away mode

**Steps**:
1. Set `input_boolean.gazebo_heating_schedule` to OFF
2. Set `select.presence_mode` to "home"
3. Set `number.desired_temperature` to 25°C
4. Verify relay activates (if temp < 25°C)
5. Set `select.presence_mode` to "away"
6. Verify relay turns OFF
7. Set `select.presence_mode` back to "home"
8. Verify relay activates again

**Expected Result**: When schedule disabled, system respects manual Home/Away mode changes

**Success Criteria**: FR-003a

---

### Scenario 6: Sensor Failure Detection

**Goal**: Verify sensor malfunction alarm works

**Steps**:
1. Disconnect DS18B20 sensor (or simulate bad reads)
2. Wait for 10 consecutive bad reads
3. Verify `binary_sensor.sensor_malfunction` turns ON
4. Verify `switch.propane_stove_relay` turns OFF (safety)
5. Reconnect sensor (or restore good reads)
6. Wait for 1 valid reading
7. Verify alarm clears automatically
8. Verify thermostat control resumes

**Expected Result**: Sensor failure triggers alarm and relay OFF, auto-recovery when sensor recovers

**Success Criteria**: SC-009, FR-014, FR-014a

---

### Scenario 7: OTA Update Safety

**Goal**: Verify OTA update blocked when heating active

**Steps**:
1. Ensure `switch.propane_stove_relay` is ON (heating active)
2. Attempt OTA firmware update via ESPHome dashboard
3. Verify update is blocked/rejected
4. Turn OFF relay (via `switch.manual_stop` or wait for temp)
5. Attempt OTA update again
6. Verify update proceeds successfully

**Expected Result**: OTA update blocked when relay is ON, allowed when relay is OFF

**Success Criteria**: FR-028a

---

### Scenario 8: WiFi Resilience

**Goal**: Verify system continues operating without WiFi

**Steps**:
1. Set `select.presence_mode` to "home"
2. Set `number.desired_temperature` to 25°C
3. Verify relay is ON (if temp < 25°C)
4. Disconnect WiFi (disable router or move device out of range)
5. Wait 5+ minutes
6. Verify relay continues operating (ON/OFF based on temperature)
7. Verify thermostat maintains last-known setpoint
8. Reconnect WiFi
9. Verify device reconnects automatically
10. Verify state syncs with Home Assistant

**Expected Result**: System operates independently for ≥60 minutes without WiFi

**Success Criteria**: SC-004, FR-029

---

### Scenario 9: Concurrent Manual Operations

**Goal**: Verify Force Off takes precedence when both switches activated

**Steps**:
1. Set `select.presence_mode` to "home"
2. Simultaneously turn ON both `switch.manual_run` and `switch.manual_stop`
3. Verify `switch.propane_stove_relay` is OFF
4. Verify `switch.manual_run` is blocked (momentary only)
5. Turn OFF `switch.manual_stop`
6. Verify `switch.manual_run` now activates relay

**Expected Result**: Force Off takes precedence, Emergency Heat blocked when Force Off active

**Success Criteria**: FR-011a

---

### Scenario 10: Temperature Setpoint Sync

**Goal**: Verify climate platform target syncs with number entity

**Steps**:
1. Set `number.desired_temperature` to 22°C
2. Verify `climate.propane_stove_thermostat` target_temperature is 22°C
3. Change target via thermostat card UI to 25°C
4. Verify `number.desired_temperature` updates to 25°C
5. Change `number.desired_temperature` to 20°C
6. Verify climate platform target updates to 20°C within 100ms

**Expected Result**: Bidirectional sync between number entity and climate platform

**Success Criteria**: SC-010

---

## Troubleshooting

### Relay Not Responding

1. Check `switch.manual_stop` state (Force Off blocks all heating)
2. Check `select.presence_mode` (Away mode blocks heating)
3. Check schedule status (outside window blocks heating)
4. Check sensor readings (malfunction blocks heating)
5. Verify GPIO12 connection

### Temperature Not Updating

1. Check DS18B20 sensor connection (GPIO4)
2. Check OneWire bus pull-up resistor
3. Review ESPHome logs for sensor errors
4. Verify sensor address in configuration

### Home Assistant Not Discovering Device

1. Verify WiFi connection
2. Check ESPHome API password matches secrets.yaml
3. Verify device is on same network as Home Assistant
4. Check firewall rules (port 6053)

### Schedule Not Working

1. Verify `input_boolean.gazebo_heating_schedule` is ON
2. Check `input_datetime` entities are configured
3. Verify Home Assistant automations are enabled
4. Check time synchronization (NTP)

---

## Next Steps

After completing quickstart scenarios:

1. Review [data-model.md](./data-model.md) for entity details
2. Review [contracts/README.md](./contracts/README.md) for API details
3. Proceed to `/speckit.tasks` to generate implementation tasks
4. Begin implementation following task list

