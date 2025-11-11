# Phase 1 Implementation Guide - Foundation & Core Integration

**Timeline**: Weeks 1-2
**Status**: ✅ Configuration files created
**Date**: 2025-11-10

---

## Overview

Phase 1 establishes the ESPHome baseline with hardware connectivity and Home Assistant integration. This guide walks through setup, configuration, testing, and debugging.

---

## Hardware Setup

### Option 1: esp32doit-devkit-v1 (Development/Testing)

**Board Details**:
- Dual-core ESP32 @ 240 MHz
- 320 KB SRAM
- 4 MB Flash
- Built-in WiFi & Bluetooth
- USB-C for programming

**GPIO Connections**:
```
esp32doit-devkit-v1              Component
───────────────────────────────────────
GPIO12 (Pin D6)      →       Relay (control)
GPIO4  (Pin D4)      →       OneWire/DS18B20 (data)
GPIO16 (Pin RX2)     ←       Nextion RX (display)
GPIO17 (Pin TX2)     →       Nextion TX (display)
GPIO2  (Pin D9)      →       Status LED (optional)
GND                  →       Ground (all components)
5V                   →       Nextion power (requires external supply)
```

### Option 2: Custom Board with ESP32 WROOM (Production)

**Board Details**:
- Same ESP32 core as dev board
- Custom PCB with optimized layout
- Direct GPIO pin access
- Optional connectors for reliability

**GPIO Connections**:
```
Custom Board                 Component
───────────────────────────────────────
GPIO12               →       Relay (control)
GPIO4                →       OneWire/DS18B20 (data)
GPIO16               ←       Nextion RX (display)
GPIO17               →       Nextion TX (display)
GPIO2                →       Status LED (optional)
GND                  →       Ground (all components)
5V                   →       Nextion power
3.3V                 →       OneWire pullup resistor
```

### Wiring Checklist

- [ ] Relay control wire connected to GPIO12
- [ ] DS18B20 data wire connected to GPIO4
- [ ] DS18B20 power (3.3V) and ground connected
- [ ] OneWire pullup resistor (4.7kΩ) installed (GPIO4 to 3.3V)
- [ ] Nextion RX connected to GPIO16 (ESP32 TX)
- [ ] Nextion TX connected to GPIO17 (ESP32 RX)
- [ ] Nextion power (5V) connected to external supply
- [ ] Nextion ground connected to ESP32 ground
- [ ] Status LED connected to GPIO2 (optional)
- [ ] All grounds connected together (star grounding)

---

## Software Setup

### 1. Install ESPHome

**Option A: Python Package**
```bash
pip install esphome
```

**Option B: Docker**
```bash
docker pull esphome/esphome
```

**Option B: VSCode Home Assistant Add-on**

- Install Home Assistant
- Install "ESPHome" add-on via add-ons store
- Access at: http://homeassistant.local:6052

### 2. Configure WiFi Credentials

Edit `secrets.yaml`:
```yaml
wifi_ssid: "Your_WiFi_Network"
wifi_password: "Your_WiFi_Password"
ota_password: "SecureOTAPassword"
```

### 3. Find DS18B20 Sensor Address

Before building, you need to find your sensor's OneWire address:

```bash
# Create a temporary test configuration
esphome create-arduino gazebo-stove-temp

# Modify to include OneWire bus discovery
# Add to gazebo_stove.yaml:
# one_wire:
#   - platform: gpio
#     id: ow_bus
#     pin: GPIO4
#
# sensor:
#   - platform: dallas_temp
#     one_wire_id: ow_bus
#     sensors:
#       - address: 0x00
#         name: "Temperature"

# Flash and monitor logs
esphome upload gazebo_stove.yaml
esphome logs gazebo_stove.yaml

# Look for output like:
# [D][dallas.sensor:156]: Found device with address: 0x00000000000001D6
```

Once you have the address, update `gazebo_stove.yaml`:
```yaml
sensor:
  - platform: dallas_temp
    one_wire_id: ow_bus
    id: gazebo_temp
    address: 0x00000000000001D6  # Replace with your address
```

---

## Building & Flashing

### Step 1: Validate Configuration

```bash
esphome config gazebo_stove.yaml
```

Expected output:
```
INFO Configuration is valid!
```

This command reads and validates the YAML configuration. If there are errors, they will be displayed. Warnings about strapping pins (GPIO2, GPIO12) are normal and safe for this application.

### Step 2: Compile

```bash
# Development board (esp32doit-devkit-v1)
esphome compile gazebo_stove.yaml

# Or specify board for custom ESP32 WROOM:
# Edit esp32 section in gazebo_stove.yaml first
esphome compile gazebo_stove.yaml
```

Compilation takes 2-5 minutes depending on system. Watch for errors.

### Step 3: Flash Device

**Option A: USB Connection (First time)**
```bash
esphome upload gazebo_stove.yaml
```

Prompts for port selection. Connect device via USB and follow prompts.

**Option B: OTA (Over-The-Air) Updates (After first flash)**
```bash
esphome upload gazebo_stove.yaml
```

Device will be found automatically if on same network.

### Step 4: Monitor Logs

```bash
esphome logs gazebo_stove.yaml
```

Watch for startup messages:
```
[00:00:00][I][esphome.cpp:29]: ESPHome 2024.x.x
[00:00:01][I][logger:175]: Log initialized
[00:00:02][I][wifi:547]: WiFi connecting...
[00:00:05][I][wifi:547]: WiFi connected, IP: 192.168.1.100
[00:00:06][I][api:081]: API Server started
[00:00:08][I][sntp:137]: Time synced from NTP
[00:00:10][I][sensor:033]: Temperature reading: 22.5°C
```

---

## Hardware Verification

### Checklist - Phase 1 Hardware Tests

#### 1. WiFi Connectivity
- [ ] Device connects to WiFi (check logs)
- [ ] IP address assigned (visible in logs)
- [ ] Can ping device: `ping gazebo-wx.local`

**Debug if failed**:
```bash
# Check WiFi logs
esphome logs gazebo_stove.yaml | grep -i wifi

# Verify credentials in secrets.yaml
# Check signal strength: look for "WiFi signal" in logs
```

#### 2. Home Assistant Discovery
- [ ] Device appears in Home Assistant as "gazebo-wx"
- [ ] All entities visible (switch, sensors, numbers, etc.)
- [ ] Can control relay switch from HA UI

**Debug if failed**:
```bash
# Check API connection in logs
esphome logs gazebo_stove.yaml | grep -i api

# Verify HA firewall allows mDNS (port 5353)
# Restart HA if device doesn't appear
```

#### 3. Temperature Sensor (DS18B20)
- [ ] Sensor entity appears in Home Assistant
- [ ] Temperature value updates (check history)
- [ ] Value is reasonable (within ±5°C of actual room temp)

**Debug if failed**:
```bash
# Check OneWire bus logs
esphome logs gazebo_stove.yaml | grep -i "dallas\|onewire"

# Verify GPIO4 connection
# Check OneWire pullup resistor (4.7kΩ between GPIO4 and 3.3V)
# Test with multimeter: GPIO4 to GND should read ~3.3V

# Alternative: Check with Arduino IDE
# Install DallasTemperature library, run example sketch
```

#### 4. Relay Control (GPIO12)
- [ ] Relay switch appears in Home Assistant
- [ ] Clicking switch audibly clicks relay
- [ ] Relay LED (if present) lights up when on
- [ ] Logs show: "Relay turned ON/OFF"

**Debug if failed**:
```bash
# Check GPIO12 logs
esphome logs gazebo_stove.yaml | grep -i "relay\|gpio12"

# Verify GPIO12 wiring to relay
# Test with multimeter: GPIO12 to GND should read ~3.3V when on, 0V when off

# Check relay power supply (usually 5V)
# Verify relay coil resistance with multimeter
```

#### 5. Nextion Display (UART)
- [ ] Display powers on (backlight visible)
- [ ] UART communication: logs show "Nextion Connected" (eventually)
- [ ] Display updates temperature value (Page 0 `temp` component)

**Debug if failed**:
```bash
# Check UART logs (should show debug messages)
esphome logs gazebo_stove.yaml | grep -i "uart\|nextion"

# Verify GPIO16 (RX) and GPIO17 (TX) connections
# Note: TX on ESP32 goes to RX on Nextion, and vice versa

# Check baud rate: Must be 9600 (Nextion default)
# Verify Nextion power (5V external supply, not USB)
# Check if Nextion is set to 9600 baud (device menu → settings)

# Test UART directly:
# Add to gazebo_stove.yaml:
# logger:
#   logs:
#     uart.debug: DEBUG
# This will show all UART bytes
```

#### 6. Status LED (GPIO2, Optional)
- [ ] LED lights up when relay is on
- [ ] LED turns off when relay is off
- [ ] Logs show: "Status LED turned on/off"

**Debug if failed**:
```bash
# Check GPIO2 logs
esphome logs gazebo_stove.yaml | grep -i "led\|gpio2"

# Verify GPIO2 wiring
# Check LED polarity (long pin = +, short = -)
# Verify current-limiting resistor (typically 330-1k Ohm)
```

---

## Common Issues & Solutions

### Issue: "Cannot find device on network"

**Symptoms**:
- ESPHome cannot upload via OTA
- Device doesn't appear in Home Assistant

**Solutions**:
1. Check WiFi connection: `esphome logs gazebo_stove.yaml` (look for WiFi errors)
2. Verify device is on same network as computer
3. Restart device: `esphome run gazebo_stove.yaml` (select physical upload)
4. Check firewall: Allow mDNS (port 5353)
5. Restart Home Assistant

### Issue: "DS18B20 not reading / showing NaN"

**Symptoms**:
- Temperature shows "unavailable" in HA
- Logs show: "Bad temperature read"

**Solutions**:
1. Verify OneWire pullup resistor (4.7kΩ, GPIO4 to 3.3V)
2. Check sensor power (3.3V and GND connected)
3. Verify GPIO4 connection (use multimeter)
4. Try different GPIO pin (GPIO4 to GPIO5, for example)
5. Verify sensor address in gazebo_stove.yaml
6. Test with separate Arduino sketch using DallasTemperature library

### Issue: "Relay doesn't respond"

**Symptoms**:
- Switch in Home Assistant but relay doesn't click
- No change in GPIO12 voltage (multimeter)

**Solutions**:
1. Verify GPIO12 connection to relay input
2. Check relay power supply (usually 5V or 12V depending on relay)
3. Verify relay coil resistance with multimeter (typically 100-500 Ohms)
4. Test relay manually with power supply (should click)
5. Check if relay is stuck (mechanical issue)
6. Try different GPIO pin (GPIO12 to GPIO13, for example)

### Issue: "Nextion display not updating"

**Symptoms**:
- Display shows default values, doesn't update
- Logs show: "UART failed" or "Nextion display failed"
- No UART debug output

**Solutions**:
1. Verify Nextion power (5V external supply, checked with multimeter)
2. Verify UART pins: GPIO16 (RX) and GPIO17 (TX)
3. Check baud rate: 9600 (Nextion settings → Serial → 9600)
4. Verify RX/TX not swapped (easy mistake)
5. Check cable quality (loose connections, damaged wires)
6. Add termination resistors (optional, but helps with signal integrity):
   - 10kΩ from GPIO16 to GND
   - 10kΩ from GPIO17 to GND
7. Test display separately with laptop using UART USB adapter

### Issue: "WiFi keeps disconnecting"

**Symptoms**:
- Device connects then disconnects repeatedly
- Logs show: "WiFi connecting..." repeatedly

**Solutions**:
1. Check WiFi signal strength (should be > -70 dBm)
2. Verify WiFi credentials in secrets.yaml
3. Try 2.4 GHz WiFi (some 5 GHz issues with ESP32)
4. Move device closer to router
5. Check for interference (microwaves, cordless phones)
6. Verify antenna connection (if external antenna)
7. Update ESP32 WiFi driver (rarely needed)

---

## Success Verification

### Phase 1 Complete When:

- [x] Device compiles without errors
- [x] Device flashes to hardware successfully
- [x] Device connects to WiFi
- [x] Device appears in Home Assistant
- [x] Temperature sensor reads and updates (within 2 seconds)
- [x] Relay can be toggled on/off
- [x] Status LED indicates relay state (if present)
- [x] Nextion display powers on and responds
- [x] OTA updates available
- [x] Logs show no critical errors
- [x] Device remains stable for 1+ hours

### Home Assistant Entities Created:

```
switch.propane_stove_relay          ← Relay ON/OFF control
sensor.gazebo_temperature           ← Temperature reading
sensor.wifi_signal_strength         ← WiFi signal dBm
binary_sensor.sensor_malfunction    ← Alarm on sensor failure

number.desired_temperature          ← Setpoint (Phase 2)
number.temperature_offset           ← Calibration
number.heating_hysteresis           ← Heating deadband (heating-only)

select.presence_mode                ← home/away

text_sensor.system_status           ← Status message
text_sensor.device_ip_address       ← IP address
text_sensor.wifi_ssid               ← WiFi network name

light.status_led                    ← Status LED control
button.device_restart               ← Restart button
button.safe_shutdown                ← Safe shutdown
```

---

## Next Steps - Phase 2

Once Phase 1 is complete and stable:

1. **Thermostat Component**: Add climate entity with heating logic
2. **Manual Modes**: Implement Manual Run/Stop with timeouts
3. **Temperature Logic**: Implement thermostat decision algorithm
4. **Home/Away Automation**: Add Home Assistant automations
5. **Integration Testing**: Verify all components work together

See `PLAN.md` Phase 2 for detailed instructions.

---

## Testing Procedures

### Test 1: Cold Boot

```bash
# Unplug device, wait 5 seconds
# Plug back in
# Monitor logs:
esphome logs gazebo_stove.yaml

# Verify:
# - WiFi connects within 30 seconds
# - Temperature sensor reads within 20 seconds
# - Home Assistant shows all entities within 30 seconds
```

**Success Criteria**: Device boots cleanly, all entities available

### Test 2: Temperature Stability

```bash
# Run for 5+ minutes
# Watch temperature value in Home Assistant

# Verify:
# - Reading updates every 10 seconds
# - Values change by ≤1°C per update (rate limiting works)
# - No "NaN" or "unavailable" values
# - Logs show no errors
```

**Success Criteria**: Stable temperature readings

### Test 3: Relay Control

```bash
# From Home Assistant UI:
# 1. Toggle relay ON → listen for click
# 2. Wait 2 seconds
# 3. Toggle relay OFF → listen for click
# 4. Repeat 10 times

# Verify:
# - Each toggle produces audible click
# - No missed toggles
# - Logs show: "Relay turned ON/OFF"
# - Status LED matches relay state
```

**Success Criteria**: Reliable relay control

### Test 4: WiFi Resilience

```bash
# Turn off WiFi at router
# Wait 1 minute
# Turn WiFi back on
# Monitor device logs

# Verify:
# - Device recognizes WiFi is down
# - Device reconnects within 30 seconds
# - No device crash or reboot
# - All sensors continue to work (no Home Assistant needed)
```

**Success Criteria**: Graceful WiFi dropout handling

### Test 5: OTA Update

```bash
# Make small change to gazebo_stove.yaml (add comment)
# Compile
# Run: esphome upload gazebo_stove.yaml
# Select OTA upload

# Verify:
# - Update completes without errors
# - Device reboots
# - All settings preserved
# - Home Assistant still shows device
```

**Success Criteria**: OTA updates work reliably

---

## Development Tips

### Faster Iteration

```bash
# Build without uploading (faster)
esphome compile gazebo_stove.yaml

# Upload only when ready
esphome upload gazebo_stove.yaml

# Or use Docker for faster builds
esphome docker compile gazebo_stove.yaml
```

### Advanced Debugging

```bash
# Enable verbose UART logging
# Modify gazebo_stove.yaml:
logger:
  level: DEBUG
  logs:
    uart.debug: DEBUG  # Shows all UART bytes
    component: DEBUG

# Monitor with timestamps
esphome logs gazebo_stove.yaml --verbose

# Save logs to file
esphome logs gazebo_stove.yaml > device.log 2>&1

# Search logs for issues
grep -i "error\|failed\|warn" device.log
```

### Configuration Template

To quickly switch between dev and production boards:

```yaml
# At top of gazebo_stove.yaml
substitutions:
  # Uncomment one:
  device_name: "gazebo-wx-dev"  # Development board
  # device_name: "gazebo-wx-prod"  # Production board

esphome:
  name: ${device_name}
  friendly_name: "Gazebo WX ${device_name}"

# Use substitutions for board-specific settings
esp32:
  board: ${device_board}  # Defined in substitutions
```

---

## Safety Considerations

### Relay Safety

- [x] Relay defaults to OFF on boot (`restore_mode: ALWAYS_OFF`)
- [x] Never enable heating without manual user action
- [x] Emergency stop: Press device reset button
- [x] Safe shutdown: Use "Safe Shutdown" button (turns off relay first)

### Sensor Safety

- [x] Detect sensor failure (alarm after 10 bad reads)
- [x] Disable relay on sensor failure (safety fallback)
- [x] Log all sensor errors for debugging

### WiFi Safety

- [x] Device operates offline (thermostat works without internet)
- [x] No cloud dependency (local Home Assistant only)
- [x] Auto-reconnect to WiFi without manual intervention

---

## File Reference

**Phase 1 Files**:
- `gazebo_stove.yaml` - Main ESPHome configuration
- `secrets.yaml` - WiFi and OTA credentials (DO NOT COMMIT)
- `PHASE_1_IMPLEMENTATION.md` - This guide

**Related Files**:
- `PLAN.md` - Overall 10-phase roadmap
- `SPECIFICATION.md` - Feature requirements
- `NEXTION_TEST.yaml` - Display test configuration
- `CLARIFICATIONS_RESOLVED.md` - Hardware specifications

---

## Summary

Phase 1 establishes:
- ✅ ESPHome baseline configuration
- ✅ WiFi connectivity
- ✅ Home Assistant integration
- ✅ Temperature sensor functionality
- ✅ Relay control
- ✅ Nextion display communication
- ✅ OTA update capability

**Estimated Duration**: 2-4 hours (including hardware setup, compilation, testing)

**Next Phase**: Phase 2 - Thermostat Control Logic

---

**Document Version**: 1.0.0
**Status**: Ready for Phase 1 Implementation
**Date**: 2025-11-10
