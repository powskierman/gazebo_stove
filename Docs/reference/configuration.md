# Gazebo Stove - Modular Configuration Guide

## Overview

The ESPHome configuration has been refactored into 7 modular files for better maintainability and readability. The main `gazebo_stove.yaml` file now imports all sub-modules using YAML includes.

## Module Structure

```
gazebo_stove.yaml (59 lines)
├── includes/core.yaml (79 lines)
├── includes/hardware.yaml (49 lines)
├── includes/sensors.yaml (62 lines)
├── includes/entities.yaml (157 lines)
├── includes/automation.yaml (60 lines)
├── includes/display.yaml (29 lines)
└── includes/buttons.yaml (22 lines)

Total: 517 lines (previously single file)
```

## Module Details

### 1. **core.yaml** (79 lines)
**Purpose**: Core ESPHome configuration and connectivity

**Contents**:
- `esphome`: Device name and boot configuration
- `esp32`: Board selection (esp32doit-devkit-v1) and Arduino framework
- `logger`: Debug logging configuration
- `wifi`: WiFi SSID/password and fallback hotspot
- `mdns`: Device discovery (gazebo-stove.local)
- `api`: Home Assistant integration settings
- `ota`: Over-the-air firmware updates
- `time`: Network Time Protocol (NTP) sync to America/Toronto timezone

**When to modify**: WiFi credentials, board type changes, timezone updates

---

### 2. **hardware.yaml** (49 lines)
**Purpose**: GPIO pin configuration and physical hardware control

**Contents**:
- `one_wire`: GPIO4 - OneWire bus for DS18B20 sensor
- `output`: GPIO2 - Status LED (inverted logic)
- `light`: Binary light control for status LED
- `switch`: GPIO12 - Relay control for propane stove heating

**GPIO Pinout**:
- GPIO2: Status LED (active low)
- GPIO4: OneWire bus
- GPIO12: Heating relay
- GPIO16: Nextion RX
- GPIO17: Nextion TX

**When to modify**: Adding new GPIO devices, changing relay pins, modifying LED behavior

---

### 3. **sensors.yaml** (62 lines)
**Purpose**: Sensor definitions and temperature measurements

**Contents**:
- `sensor.dallas_temp`: DS18B20 temperature sensor (0x7ce066bd0164ff28)
  - Filters: throttle_average, offset calibration, clamping, rate limiting
  - Bad read detection (>10 consecutive failures triggers malfunction alert)
  - Automatic heating control trigger on valid readings
- `sensor.wifi_signal`: WiFi signal strength monitoring

**Features**:
- ±1°C per-reading rate limiting to prevent spurious spikes
- Temperature offset calibration via `temp_offset` number entity
- Automatic malfunction detection and relay shutdown

**When to modify**: Sensor address changes, filter tuning, update intervals

---

### 4. **entities.yaml** (157 lines)
**Purpose**: User-facing controls and status indicators

**Contents**:

#### Binary Sensors
- `sensor_malfunction`: Alert when temperature sensor fails

#### Number Entities (Sliders)
- `desired_temp`: Target setpoint (5°C - 35°C, 0.5°C steps, default 22°C)
- `temp_offset`: Calibration offset (-5°C to +5°C, 0.1°C steps, default 0°C)
- `heating_hysteresis`: Deadband width (1°C - 5°C, 0.5°C steps, default 2°C)

#### Select Entities
- `presence_mode`: Home/Away mode selection

#### Text Sensors
- `device_ip`: Current IP address
- `wifi_ssid`: Connected WiFi network
- `mac_address`: Device MAC address
- `wifi_bssid`: WiFi access point BSSID
- `system_status`: Overall system status (OK/ALARM)

#### Globals
- `bad_read_count`: Counter for temperature sensor failures
- `manual_run_timeout`: Reserved for Phase 3 manual mode

**When to modify**: Adding user controls, changing control ranges, adding alarms

---

### 5. **automation.yaml** (60 lines)
**Purpose**: Control logic and heating automation

**Contents**:

#### Scripts

**`init_nextion`**
- Initializes Nextion display on boot
- Clears display and sets initial values
- Gracefully handles display offline scenarios

**`manage_heating`** (Core heating control)
- Implements hysteresis-based temperature control
- Logic:
  - **Away Mode**: Relay always OFF
  - **Home Mode**:
    - ON when temp < (desired - hysteresis/2)
    - OFF when temp > (desired + hysteresis/2)
    - Maintains state within deadband
- Detailed logging for troubleshooting

**When to modify**: Heating algorithm changes, adding automation logic, control thresholds

---

### 6. **display.yaml** (29 lines)
**Purpose**: Nextion display integration via UART

**Contents**:
- `uart`: UART Bus 1 configuration (GPIO16/17, 9600 baud)
  - UART debugging enabled (logs data direction)
- `display.nextion`: Nextion component initialization

**UART Pinout**:
- GPIO17: TX (transmit to display)
- GPIO16: RX (receive from display)
- Baud rate: 9600

**When to modify**: Adding display pages, updating UART settings, changing display commands

---

### 7. **buttons.yaml** (22 lines)
**Purpose**: Manual device controls

**Contents**:
- `button.restart`: Reboot device button
- `button.template.shutdown`: Safe shutdown procedure
  - Turns off relay
  - Waits 1 second
  - Reboots device

**When to modify**: Adding control buttons, changing shutdown procedures

---

## Usage & Compilation

### Validation
```bash
esphome config gazebo_stove.yaml
```

### Compilation
```bash
esphome compile gazebo_stove.yaml
```

### Upload
```bash
esphome upload gazebo_stove.yaml
```

### Monitoring
```bash
esphome logs gazebo_stove.yaml
```

## Key Design Decisions

### 1. **Modular Includes**
- Each module handles a specific domain (hardware, sensors, entities, automation)
- Makes it easy to locate and modify specific functionality
- Reduces merge conflicts during team development

### 2. **Heating Control via Script**
- Script-based heating instead of climate component allows custom hysteresis
- Triggered by sensor readings, setpoint changes, and presence mode changes
- Provides detailed logging for debugging

### 3. **Global Temperature Control**
- All temperature-related logic in `sensors.yaml` and `automation.yaml`
- Single source of truth for heating decisions
- Easy to understand control flow

### 4. **Entity Restoration**
- All number entities have `restore_value: yes`
- Settings persist across reboots
- Default values ensure safe operation on first boot

## Future Phases

### Phase 3: Home/Away & Manual Modes
- Automatic presence detection from Home Assistant
- Manual override with timeout protection
- Will add new automation script to `automation.yaml`

### Phase 4: Advanced Features
- Historical temperature tracking
- Heating duration statistics
- Thermal comfort optimization

### Phase 5: Integration & Polish
- Nextion display UI enhancements
- Home Assistant automations
- Mobile app notifications

## File Size Comparison

**Before Refactoring**: 517 lines (single monolithic file)
**After Refactoring**: 517 lines (7 modular files + main include file)

**Benefits**:
- ✅ Easier navigation (max 157 lines per file)
- ✅ Logical organization by function
- ✅ Faster compilation (5.56 seconds)
- ✅ Better for version control
- ✅ Clearer for documentation

## Troubleshooting

### Device not responding
- Check `core.yaml` WiFi credentials
- Monitor `esphome logs gazebo_stove.yaml`

### Heating not activating
- Check `desired_temp` > current temperature
- Verify `presence_mode` is "home"
- Check relay state in `hardware.yaml`

### Temperature sensor errors
- Check OneWire bus (GPIO4) wiring
- Verify sensor address in `sensors.yaml`
- Monitor "Sensor Malfunction" binary sensor

### Display not updating
- Check UART pins in `display.yaml` (GPIO16/17)
- Verify baud rate (9600)
- Monitor Nextion initialization in logs

---

**Last Updated**: 2025-11-11
**Config Version**: 2.0 (Modular)
**Status**: Phase 2 Complete, Phase 3 Pending
