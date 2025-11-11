# ESPHome Modular Configuration - Module Guide

## Overview

This directory contains the modular sub-configurations for the Gazebo Stove ESPHome project. Each file handles a specific domain of functionality.

## Module Files

### 📋 **core.yaml** - System & Network Configuration
**Scope**: Boot-up, WiFi, API, OTA, NTP, Logging
**Lines**: 79
**Key Variables**:
- Device name: `gazebo-stove`
- WiFi SSID: `!secret wifi_ssid`
- WiFi Password: `!secret wifi_password`
- Timezone: `America/Toronto`
- OTA Password: `!secret ota_password`

**When to Edit**:
- Changing WiFi credentials
- Updating timezone
- Modifying log levels
- Changing boot behavior

---

### 🔧 **hardware.yaml** - Physical Hardware & GPIO Control
**Scope**: OneWire, GPIO outputs, LED, Relay
**Lines**: 49
**GPIO Pins**:
- GPIO2: Status LED (inverted)
- GPIO4: OneWire bus
- GPIO12: Heating relay

**Components**:
- Status LED: Binary light control
- Relay: GPIO switch with on/off callbacks

**When to Edit**:
- Adding new GPIO devices
- Changing pin assignments
- Modifying relay behavior
- Adding/removing hardware

---

### 📊 **sensors.yaml** - Data Collection & Measurement
**Scope**: Temperature sensor, WiFi signal, filtering
**Lines**: 62
**Sensor Addresses**:
- DS18B20 Address: `0x7ce066bd0164ff28`

**Features**:
- Throttle averaging (10s window)
- Temperature offset filter
- Value clamping (-40°C to +85°C)
- Rate limiting (±1°C per reading)
- Bad read detection (>10 triggers alarm)
- Automatic heating trigger on valid readings

**When to Edit**:
- Changing sensor address (after replacement)
- Adjusting filter parameters
- Modifying update intervals
- Changing malfunction thresholds

---

### 🎚️ **entities.yaml** - User Controls & Status Indicators
**Scope**: Numbers, selects, binary sensors, text sensors, globals
**Lines**: 157

**User Controls**:
- `desired_temp`: Temperature setpoint (5-35°C, default 22°C)
- `temp_offset`: Calibration offset (-5 to +5°C, default 0°C)
- `heating_hysteresis`: Deadband control (1-5°C, default 2°C)
- `presence_mode`: Home/Away selection

**Status Indicators**:
- `sensor_malfunction`: Binary alarm
- `system_status`: Text status message
- `device_ip`, `wifi_ssid`: Network diagnostics

**Global Variables**:
- `bad_read_count`: Temperature sensor failure counter
- `manual_run_timeout`: Reserved for Phase 3

**When to Edit**:
- Adding user controls
- Changing slider ranges
- Adding status indicators
- Modifying entity definitions

---

### ⚙️ **automation.yaml** - Control Logic & Automation Scripts
**Scope**: Heating algorithm, display initialization, system automation
**Lines**: 60

**Scripts**:

1. **`init_nextion`**
   - Initializes Nextion display on boot
   - Clears screen and sets default values
   - Gracefully handles offline display

2. **`manage_heating`**
   - Core heating control algorithm
   - Hysteresis-based temperature regulation
   - Away mode support
   - Detailed diagnostic logging

**Heating Logic**:
```
if away_mode:
  relay OFF (always)
else:
  if temp < (desired - hysteresis/2):
    relay ON
  else if temp > (desired + hysteresis/2):
    relay OFF
  else:
    maintain current state (deadband)
```

**When to Edit**:
- Modifying heating algorithm
- Adding new automation scripts
- Changing control thresholds
- Adding system logic

---

### 📱 **display.yaml** - Nextion Display Integration
**Scope**: UART configuration, display component
**Lines**: 29
**UART Configuration**:
- TX Pin: GPIO17
- RX Pin: GPIO16
- Baud Rate: 9600

**Features**:
- UART debugging enabled
- Bidirectional communication support
- Component initialization

**When to Edit**:
- Changing UART pins
- Modifying baud rate
- Updating display initialization
- Adding display commands

---

### ⏻️ **buttons.yaml** - Manual Controls
**Scope**: User-triggered actions
**Lines**: 22

**Buttons**:
1. `restart_button`: Device restart
2. `shutdown_button`: Safe shutdown
   - Turns off relay
   - Waits 1 second
   - Reboots device

**When to Edit**:
- Adding control buttons
- Modifying shutdown procedure
- Adding emergency controls

---

## Include Order

The main `gazebo_stove.yaml` includes modules in this order:

1. **core.yaml** - Must be first (system config)
2. **hardware.yaml** - GPIO definitions (needed by sensors)
3. **sensors.yaml** - Measurement definitions
4. **entities.yaml** - User controls & globals
5. **automation.yaml** - Scripts (uses entities)
6. **display.yaml** - UART & display
7. **buttons.yaml** - Controls (no dependencies)

⚠️ **Important**: Changing the include order may cause errors if modules have dependencies.

## Creating New Modules

If adding a new feature module:

1. **Create file**: `includes/feature.yaml`
2. **Add clear header** with purpose and scope
3. **Comment each section** clearly
4. **Add to main file**: Append to `gazebo_stove.yaml` in logical order
5. **Document it**: Add entry to this README
6. **Test thoroughly**: `esphome config` and `esphome compile`

Example structure:
```yaml
# ============================================================================
# FEATURE NAME
# ============================================================================
# Purpose: Clear description of what this module does
# Dependencies: List any required entities/components

# Component definitions...
```

## Debugging Module Issues

### Syntax Errors
```bash
esphome config gazebo_stove.yaml
```
Shows exact line numbers and syntax issues.

### Undefined Entities
Check that dependencies are included before usage:
- `automation.yaml` needs `entities.yaml` included first
- `sensors.yaml` needs `hardware.yaml` (OneWire bus)

### Component Not Found
Ensure module is included in `gazebo_stove.yaml` with correct syntax:
```yaml
<<: !include includes/module.yaml
```

### Logic Errors
Monitor runtime behavior:
```bash
esphome logs gazebo_stove.yaml | grep [COMPONENT_TAG]
```

## Testing Changes

After modifying any module:

1. **Validate syntax**:
   ```bash
   esphome config gazebo_stove.yaml
   ```

2. **Compile**:
   ```bash
   esphome compile gazebo_stove.yaml
   ```

3. **Upload to device**:
   ```bash
   esphome upload gazebo_stove.yaml
   ```

4. **Check logs**:
   ```bash
   esphome logs gazebo_stove.yaml
   ```

## Best Practices

✅ **Do**:
- Keep modules focused on single domain
- Add clear section comments
- Document changes in module header
- Test after any modification
- Use consistent formatting

❌ **Don't**:
- Put unrelated components in same module
- Reference components from "later" modules
- Remove comments (they help future developers)
- Manually edit compiled binaries
- Change include order without testing

## Version History

| Date | Version | Changes |
|------|---------|---------|
| 2025-11-11 | 1.0 | Initial modularization from monolithic config |
| - | 2.0 | Phase 3 features (planned) |
| - | 3.0 | Advanced features (planned) |

## Quick Links

- **Main Configuration**: `../gazebo_stove.yaml`
- **Detailed Guide**: `../CONFIG_MODULES.md`
- **Quick Reference**: `../QUICK_REFERENCE.md`
- **Project Documentation**: `../Docs/`

---

**Last Updated**: 2025-11-11
