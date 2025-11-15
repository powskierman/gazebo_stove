# Quick Reference - Module Locations

## What to Edit for Common Tasks

### 🔧 Hardware Changes
**File**: `includes/hardware.yaml`
- Adding/removing GPIO devices
- Changing relay or LED pins
- Modifying switch restore modes

### 📊 Sensor Configuration
**File**: `includes/sensors.yaml`
- Temperature sensor address
- Sensor filters and smoothing
- Update intervals
- Malfunction detection thresholds

### 🌡️ Heating Control Logic
**File**: `includes/automation.yaml` - `manage_heating` script
- Temperature setpoint calculation
- Hysteresis thresholds
- Away mode behavior
- Relay activation/deactivation logic

### 🎚️ User Controls
**File**: `includes/entities.yaml`
- Desired temperature slider (lines 26-48)
- Temperature offset calibration (lines 50-66)
- Heating hysteresis slider (lines 68-84)
- Presence mode selection (lines 95-107)

### 📱 Display/UI
**File**: `includes/display.yaml`
- Nextion UART configuration
- Display initialization
- Display updates (see automation.yaml for script)

### 🔌 WiFi & Network
**File**: `includes/core.yaml`
- WiFi SSID and password
- NTP timezone
- mDNS hostname
- OTA update settings

### ⏻️ System Controls
**File**: `includes/buttons.yaml`
- Restart button
- Safe shutdown procedure

---

## Quick Navigation

### Temperature Control Flow
1. Sensor reads temperature → `sensors.yaml:172-197`
2. On valid read, triggers heating script → `automation.yaml:43-47`
3. Script evaluates conditions → `automation.yaml:50-92`
4. Relay turns ON/OFF → `hardware.yaml:38-56`

### Setpoint Change Flow
1. User adjusts slider in Home Assistant
2. Set action triggered → `entities.yaml:41-48`
3. Executes manage_heating script → `automation.yaml:43-47`
4. Heating state updates

### Away Mode Flow
1. User selects "away" in Presence Mode → `entities.yaml:99-107`
2. Set action calls manage_heating script
3. Script checks `away_mode` → `automation.yaml:65`
4. Relay forced OFF → `automation.yaml:66-70`

---

## Common Configuration Changes

### Increase Max Temperature Setpoint
Edit `includes/entities.yaml` line 34:
```yaml
max_value: 35  # Change to 45 for higher temp
```

### Adjust Hysteresis Range
Edit `includes/entities.yaml` line 74:
```yaml
max_value: 5  # Change to 10 for wider range
```

### Change Temperature Sensor Address
Edit `includes/sensors.yaml` line 10:
```yaml
address: 0x7ce066bd0164ff28  # Update to new address
```

### Change WiFi Network
Edit `includes/core.yaml` lines 50-51:
```yaml
ssid: !secret wifi_ssid
password: !secret wifi_password
```

### Modify Nextion UART Pins
Edit `includes/display.yaml` lines 9-10:
```yaml
tx_pin: GPIO17  # Change as needed
rx_pin: GPIO16  # Change as needed
```

### Adjust Temperature Filter
Edit `includes/sensors.yaml` line 19:
```yaml
- throttle_average: 10s  # Change averaging window
```

---

## File Organization

```
includes/
├── core.yaml           → Network, API, System
├── hardware.yaml       → GPIO, Relay, LED
├── sensors.yaml        → Temperature, Diagnostics
├── entities.yaml       → User Controls, Status
├── automation.yaml     → Heating Logic, Scripts
├── display.yaml        → Nextion Display, UART
└── buttons.yaml        → Manual Controls

gazebo_stove.yaml      → Main file (includes all modules)
```

---

## Compilation & Upload Commands

**Just validate**:
```bash
esphome config gazebo_stove.yaml
```

**Compile**:
```bash
esphome compile gazebo_stove.yaml
```

**Upload to device**:
```bash
esphome upload gazebo_stove.yaml
```

**Watch logs**:
```bash
esphome logs gazebo_stove.yaml
```

**Search logs for heating events**:
```bash
esphome logs gazebo_stove.yaml | grep HEATING
```

---

## Debug Logging

### View Temperature Readings
```bash
esphome logs gazebo_stove.yaml | grep "Got Temperature"
```

### View Heating Control Decisions
```bash
esphome logs gazebo_stove.yaml | grep "HEATING"
```

### View Sensor Status
```bash
esphome logs gazebo_stove.yaml | grep "TEMP_SENSOR"
```

### View Display Activity
```bash
esphome logs gazebo_stove.yaml | grep "NEXTION"
```

### View All DEBUG messages
```bash
esphome logs gazebo_stove.yaml | grep "\[D\]"
```

---

**Last Updated**: 2025-11-11
