# Data Model: Gazebo Propane Stove Heating Control System

**Date**: 2025-01-27  
**Feature**: 001-gazebo-stove-heating-control

## Entities

### Climate Entity: `propane_stove_thermostat`

**Purpose**: Primary thermostat control entity for Home Assistant UI

**Fields**:
- `name`: "Propane Stove Thermostat"
- `id`: `propane_stove_thermostat`
- `current_temperature`: float (from `gazebo_temp` sensor, °C)
- `target_temperature`: float (synced with `desired_temp` number entity, 5-35°C)
- `min_temp`: 5
- `max_temp`: 35
- `hvac_mode`: "heat" | "off" (controlled by presence mode and schedule)
- `hvac_action`: "heating" | "idle" | "off" (derived from relay state)
- `preset_modes`: ["home", "away"]
- `preset_mode`: string (synced with `presence_mode` select)

**State Transitions**:
- `hvac_mode`: "off" → "heat" when Home mode active and schedule allows
- `hvac_mode`: "heat" → "off" when Away mode active or schedule blocks
- `hvac_action`: "idle" → "heating" when relay turns ON
- `hvac_action`: "heating" → "idle" when relay turns OFF

**Validation Rules**:
- `target_temperature` must be between 5-35°C
- `target_temperature` syncs with `desired_temp` number entity
- `hvac_mode` "off" when `presence_mode` == "away"

---

### Number Entity: `desired_temp`

**Purpose**: User-configurable temperature setpoint

**Fields**:
- `name`: "Desired Temperature"
- `id`: `desired_temp`
- `value`: float (5-35°C, step 0.5°C)
- `min_value`: 5
- `max_value`: 35
- `step`: 0.5
- `unit_of_measurement`: "°C"
- `restore_value`: yes
- `initial_value`: 22

**State Transitions**:
- Value changes → triggers climate platform target sync
- Value changes → triggers `manage_heating` script execution

**Validation Rules**:
- Must be within 5-35°C range
- Changes persist across reboot (Home Assistant state persistence)

---

### Number Entity: `heating_hysteresis`

**Purpose**: User-configurable deadband for thermostat control

**Fields**:
- `name`: "Heating Hysteresis"
- `id`: `heating_hysteresis`
- `value`: float (1-5°C, step 0.5°C)
- `min_value`: 1
- `max_value`: 5
- `step`: 0.5
- `unit_of_measurement`: "°C"
- `restore_value`: yes
- `initial_value`: 2

**Note**: ESPHome climate platform uses its own internal hysteresis calculation (symmetric ±H/2). This entity is for reference/display only.

---

### Number Entity: `temp_offset`

**Purpose**: Sensor calibration offset

**Fields**:
- `name`: "Temperature Offset"
- `id`: `temp_offset`
- `value`: float (-5 to +5°C, step 0.1°C)
- `min_value`: -5
- `max_value`: 5
- `step`: 0.1
- `unit_of_measurement`: "°C"
- `restore_value`: yes
- `initial_value`: 0

**State Transitions**:
- Value changes → applied to sensor reading before display/storage

---

### Select Entity: `presence_mode`

**Purpose**: Home/Away mode selection

**Fields**:
- `name`: "Presence Mode"
- `id`: `presence_mode`
- `options`: ["home", "away"]
- `initial_option`: "home"
- `restore_value`: yes

**State Transitions**:
- "home" → "away": Relay turns OFF immediately (Priority 3)
- "away" → "home": Thermostat control resumes (if schedule allows)

**Validation Rules**:
- Only two valid states: "home" or "away"
- Changes trigger `manage_heating` script execution

---

### Switch Entity: `manual_run`

**Purpose**: Emergency Heat override (15-minute timer)

**Fields**:
- `name`: "Manual Run" / "Emergency Heat"
- `id`: `manual_run`
- `state`: boolean (ON/OFF)
- `restore_mode`: ALWAYS_OFF
- `optimistic`: true
- `assumed_state`: false

**State Transitions**:
- OFF → ON: Relay turns ON, 15-minute timer starts, `manual_run_active` = true
- ON → OFF: Relay turns OFF, timer cleared, `manual_run_active` = false
- ON (15 min elapsed): Auto-OFF, `manual_run_active` = false

**Validation Rules**:
- Blocked if `manual_stop_active` == true (Force Off takes precedence)
- Timeout enforced: exactly 15 minutes from activation

---

### Switch Entity: `manual_stop`

**Purpose**: Force Off override (highest priority)

**Fields**:
- `name`: "Manual Stop" / "Force Off"
- `id`: `manual_stop`
- `state`: boolean (ON/OFF)
- `restore_mode`: ALWAYS_OFF
- `optimistic`: true
- `assumed_state`: false

**State Transitions**:
- OFF → ON: Relay turns OFF, `manual_stop_active` = true
- ON → OFF: `manual_stop_active` = false, normal control resumes

**Validation Rules**:
- Highest priority: When ON, blocks all other heating modes
- Cannot be overridden by Emergency Heat

---

### Switch Entity: `relay`

**Purpose**: Physical relay control (GPIO12)

**Fields**:
- `name`: "Propane Stove Relay"
- `id`: `relay`
- `state`: boolean (ON/OFF)
- `pin`: GPIO12
- `inverted`: false (active HIGH)

**State Transitions**:
- Controlled by priority script (`manage_heating`)
- ON: Heating active
- OFF: Heating inactive

**Validation Rules**:
- Must be OFF when `manual_stop_active` == true
- Must be OFF when `presence_mode` == "away" (if schedule enabled)
- Must be OFF when outside schedule window (if schedule enabled)

---

### Sensor Entity: `gazebo_temp`

**Purpose**: Temperature reading from DS18B20 sensor

**Fields**:
- `name`: "Gazebo Temperature"
- `id`: `gazebo_temp`
- `value`: float (°C)
- `unit_of_measurement`: "°C"
- `device_class`: "temperature"
- `accuracy_decimals`: 1
- `filters`: [throttle_average: 10s]

**State Transitions**:
- Valid reading: Updates value, clears bad read counter
- Invalid reading (NaN): Increments bad read counter
- 10+ consecutive bad reads: Triggers alarm, relay turns OFF

**Validation Rules**:
- Value range: -55°C to +125°C (DS18B20 specification)
- Rate-limited: Max ±1°C change per update
- Bad read threshold: 10 consecutive failures

---

### Binary Sensor Entity: `sensor_malfunction`

**Purpose**: Sensor fault detection alarm

**Fields**:
- `name`: "Sensor Malfunction"
- `id`: `sensor_malfunction`
- `state`: boolean (ON = alarm active)
- `device_class`: "problem"

**State Transitions**:
- OFF → ON: After 10 consecutive bad sensor reads
- ON → OFF: After 1 valid sensor read (auto-recovery)

**Validation Rules**:
- Alarm triggers relay OFF (safety requirement)
- Auto-clears when sensor recovers

---

### Global Variables (ESPHome Lambda State)

**Purpose**: Maintain priority control state

**Variables**:
- `manual_stop_active`: bool (Force Off state)
- `manual_run_active`: bool (Emergency Heat state)
- `manual_run_timeout`: unsigned long (milliseconds timestamp)

**State Transitions**:
- `manual_stop_active`: Set by `manual_stop` switch actions
- `manual_run_active`: Set by `manual_run` switch actions
- `manual_run_timeout`: Set when Emergency Heat activated, checked every second

---

## Relationships

```
presence_mode (select)
    ↓
    ├─→ climate.preset_mode (synced)
    └─→ manage_heating script (Priority 3 check)

desired_temp (number)
    ↓
    ├─→ climate.target_temperature (synced via lambda)
    └─→ manage_heating script (Priority 4 check)

gazebo_temp (sensor)
    ↓
    ├─→ climate.current_temperature
    └─→ manage_heating script (temperature comparison)

manual_stop (switch)
    ↓
    └─→ manual_stop_active (global var) → manage_heating (Priority 1)

manual_run (switch)
    ↓
    ├─→ manual_run_active (global var) → manage_heating (Priority 2)
    └─→ manual_run_timeout (global var) → timeout check

relay (switch)
    ↓
    └─→ Physical GPIO12 output (propane stove control)
```

---

## Data Flow

### Priority Control Flow

```
1. State Change Event (switch, select, number, sensor)
   ↓
2. manage_heating script triggered
   ↓
3. Priority Check (in order):
   ├─ Priority 1: manual_stop_active? → Relay OFF, return
   ├─ Priority 2: manual_run_active? → Relay ON, return
   ├─ Priority 3: presence_mode == "away"? → Relay OFF, return
   ├─ Priority 3: Schedule check → Relay OFF if outside window, return
   └─ Priority 4: Thermostat control → Relay ON/OFF based on temperature
   ↓
4. Relay state updated
   ↓
5. Nextion display updated (if relay state changed)
```

### Temperature Control Flow

```
1. gazebo_temp sensor reading
   ↓
2. Apply temp_offset calibration
   ↓
3. Update climate.current_temperature
   ↓
4. Trigger manage_heating script
   ↓
5. Compare: current_temp vs (desired_temp ± hysteresis)
   ↓
6. Update relay state (if Priority 4 active)
```

---

## Persistence

### ESPHome Device-Side
- `restore_value: yes` for number/select entities
- Global variables reset on boot (no persistence)

### Home Assistant-Side
- Entity state persistence (database)
- `input_datetime` helpers persist schedule times
- `input_boolean` persists schedule enable state

---

## Validation Summary

| Entity | Validation Rule | Error Handling |
|--------|---------------|----------------|
| `desired_temp` | 5-35°C range | Clamp to min/max |
| `heating_hysteresis` | 1-5°C range | Clamp to min/max |
| `temp_offset` | -5 to +5°C range | Clamp to min/max |
| `presence_mode` | "home" or "away" only | Reject invalid option |
| `gazebo_temp` | -55 to +125°C (sensor spec) | Mark as invalid if out of range |
| `relay` | Must respect priority hierarchy | Force OFF if higher priority active |



