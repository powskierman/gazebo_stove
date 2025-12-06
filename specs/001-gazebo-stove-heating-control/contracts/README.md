# API Contracts: Gazebo Propane Stove Heating Control System

**Date**: 2025-01-27  
**Feature**: 001-gazebo-stove-heating-control

## Overview

This feature uses ESPHome's native Home Assistant API protocol. No custom REST/GraphQL APIs are required. All communication follows ESPHome's standard entity-based protocol.

## Communication Protocol

### ESPHome Native API

**Protocol**: ESPHome Native API (binary protocol over TCP)  
**Port**: 6053 (default)  
**Authentication**: API password (from secrets.yaml)

### Entity-Based Communication

All control and monitoring happens through Home Assistant entities:

- **Climate Entity**: `climate.propane_stove_thermostat`
- **Number Entities**: `number.desired_temperature`, `number.heating_hysteresis`, `number.temperature_offset`
- **Select Entity**: `select.presence_mode`
- **Switch Entities**: `switch.manual_run`, `switch.manual_stop`, `switch.propane_stove_relay`
- **Sensor Entity**: `sensor.gazebo_temperature`
- **Binary Sensor Entity**: `binary_sensor.sensor_malfunction`

## Entity Contracts

### Climate Entity Contract

**Entity ID**: `climate.propane_stove_thermostat`

**Read Operations**:
- `current_temperature`: Get current temperature (°C)
- `target_temperature`: Get desired setpoint (°C)
- `hvac_mode`: Get current mode ("heat" | "off")
- `hvac_action`: Get current action ("heating" | "idle" | "off")
- `preset_mode`: Get current preset ("home" | "away")

**Write Operations**:
- `set_temperature`: Set target temperature (5-35°C)
- `set_hvac_mode`: Set HVAC mode ("heat" | "off")
- `set_preset_mode`: Set preset ("home" | "away")

**State Updates**:
- Entity publishes state changes automatically via ESPHome API
- Home Assistant receives updates within 2 seconds

---

### Number Entity Contracts

#### `number.desired_temperature`

**Read**: Get current setpoint (float, °C)  
**Write**: Set setpoint (float, 5-35°C, step 0.5°C)  
**Side Effects**: 
- Syncs climate platform target temperature
- Triggers `manage_heating` script execution

#### `number.heating_hysteresis`

**Read**: Get current hysteresis (float, °C)  
**Write**: Set hysteresis (float, 1-5°C, step 0.5°C)  
**Side Effects**: None (reference only, platform uses internal calculation)

#### `number.temperature_offset`

**Read**: Get current offset (float, °C)  
**Write**: Set offset (float, -5 to +5°C, step 0.1°C)  
**Side Effects**: Applied to sensor readings

---

### Select Entity Contract

#### `select.presence_mode`

**Read**: Get current mode ("home" | "away")  
**Write**: Set mode ("home" | "away")  
**Side Effects**: 
- Triggers `manage_heating` script execution
- Updates climate platform preset mode

---

### Switch Entity Contracts

#### `switch.manual_run` (Emergency Heat)

**Read**: Get current state (ON/OFF)  
**Write**: 
- Turn ON: Activates relay, starts 15-minute timer
- Turn OFF: Deactivates relay, clears timer

**Side Effects**:
- Sets `manual_run_active` global variable
- Sets `manual_run_timeout` timestamp
- Blocks if `manual_stop_active` == true

#### `switch.manual_stop` (Force Off)

**Read**: Get current state (ON/OFF)  
**Write**:
- Turn ON: Deactivates relay, blocks all heating
- Turn OFF: Resumes normal control

**Side Effects**:
- Sets `manual_stop_active` global variable
- Highest priority override

#### `switch.propane_stove_relay`

**Read**: Get current relay state (ON/OFF)  
**Write**: Controlled by `manage_heating` script only (not directly writable)

**Side Effects**: Physical GPIO12 output controls propane stove

---

### Sensor Entity Contract

#### `sensor.gazebo_temperature`

**Read**: Get current temperature (float, °C)  
**Write**: Read-only (updated by DS18B20 sensor)

**Update Frequency**: Every 10 seconds (throttle_average filter)

---

### Binary Sensor Entity Contract

#### `binary_sensor.sensor_malfunction`

**Read**: Get alarm state (ON = alarm active)  
**Write**: Read-only (updated by sensor fault detection logic)

**State Transitions**:
- OFF → ON: After 10 consecutive bad reads
- ON → OFF: After 1 valid read

---

## Home Assistant Service Calls

### Climate Services

```yaml
# Set target temperature
service: climate.set_temperature
target:
  entity_id: climate.propane_stove_thermostat
data:
  temperature: 22.0

# Set HVAC mode
service: climate.set_hvac_mode
target:
  entity_id: climate.propane_stove_thermostat
data:
  hvac_mode: heat

# Set preset mode
service: climate.set_preset_mode
target:
  entity_id: climate.propane_stove_thermostat
data:
  preset_mode: home
```

### Number Services

```yaml
# Set desired temperature
service: number.set_value
target:
  entity_id: number.desired_temperature
data:
  value: 22.0

# Set hysteresis
service: number.set_value
target:
  entity_id: number.heating_hysteresis
data:
  value: 2.0
```

### Select Services

```yaml
# Set presence mode
service: select.select_option
target:
  entity_id: select.presence_mode
data:
  option: home
```

### Switch Services

```yaml
# Activate Emergency Heat
service: switch.turn_on
target:
  entity_id: switch.manual_run

# Activate Force Off
service: switch.turn_on
target:
  entity_id: switch.manual_stop
```

---

## Error Handling

### Invalid Values

- **Out of Range**: Values clamped to min/max (no error thrown)
- **Invalid Option**: Select option rejected, state unchanged
- **Blocked Operation**: Switch operation blocked, state unchanged (e.g., Emergency Heat blocked by Force Off)

### Communication Failures

- **WiFi Loss**: Device continues operation with cached state
- **API Disconnect**: Home Assistant shows "unavailable", device operates independently
- **Reconnection**: State syncs automatically when connection restored

---

## State Synchronization

### ESPHome → Home Assistant

- All entity state changes published automatically
- Update latency: <2 seconds
- State persistence: Home Assistant database

### Home Assistant → ESPHome

- Service calls processed immediately
- Update latency: <2 seconds
- State changes trigger ESPHome automations

---

## Security

- **API Password**: Required for ESPHome API connection (from secrets.yaml)
- **Local Network Only**: ESPHome API not exposed to internet
- **No External APIs**: All control local (except weather API, read-only)

---

## Performance

- **Entity Updates**: <2 seconds latency
- **Relay Response**: <10 seconds from sensor reading to relay state change
- **Display Updates**: <2 seconds for state changes
- **API Overhead**: Minimal (binary protocol, efficient)



