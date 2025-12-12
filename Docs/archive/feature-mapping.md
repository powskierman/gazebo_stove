# GazeboWx: Arduino → ESPHome Feature Mapping

**Date**: 2025-11-10
**Source Project**: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx` (PlatformIO + Arduino)
**Target Project**: `/Users/michel/Documents/Electronics/Projects/GazeboStove` (ESPHome)

---

## Overview

This document provides a comprehensive mapping of all features, functions, and data structures from the original Arduino/Blynk implementation to their ESPHome/Home Assistant equivalents. This serves as a reference during implementation to ensure no functionality is lost during migration.

---

## Core Features Mapping

### Feature 1: Automatic Thermostat Control

**Arduino Implementation** (`thermostat.cpp`, `main.cpp`)
```cpp
void TempUpdate() {
  sensors.requestTemperatures();
  float ReadF = sensors.getTempCByIndex(0);

  TempAvg = int((ReadF + LastRead + (2 * TempCorrection))/2);

  // Heating-only stove logic (no cooling)
  if (!ManualRun && !ManualStop) {
    if (Home) {
      // Heating mode only - stove inactive during summer
      if (TempAct < TempDes) { Fan(1); }  // Turn on heating
      else if (TempAct >= (TempDes + Hysteresis)) { Fan(0); }  // Turn off with deadband
    }
    else Fan(0);  // Away mode: always off
  }
}
```

**ESPHome Implementation** (gazebo_stove.yaml)

```yaml
climate:
  - platform: thermostat
    name: Gazebo Thermostat
    sensor: gazebo_temp
    default_target_temperature: 22
    min_heating_off_time: 60s
    min_heating_run_time: 60s
    min_idle_time: 60s
    heat_deadband: 0.5
    heat_overrun: 0.5
    heat_action:
      - switch.turn_on: relay
    idle_action:
      - switch.turn_off: relay

# Home Assistant presence mode selection (season not needed for heating-only)
input_select:
  presence_mode:
    name: Presence
    options:
      - home
      - away
    initial: home
```

**Mapping Notes**:
- `TempUpdate()` loop → ESPHome climate component (automatic 5-second cycle)
- `TempAct` (current temperature) ← Dallas sensor entity
- `TempDes` (desired setpoint) ← climate.target_temperature
- **NOTE**: Season mode removed (heating-only stove has no summer operation)
- `Hysteresis` ← single heating_hysteresis number entity (1-5°C range)
- `ManualRun` / `ManualStop` / `Home` → Home Assistant automations override climate logic

**Implementation Status**: ✅ Phase 2

---

### Feature 2: Home/Away Mode

**Arduino Implementation** (thermostat.cpp)
```cpp
// In TempUpdate():
if (Home) {  // Home mode: run thermostat
  if (Winter) { ... }
} else {     // Away mode: always off
  Fan(0);
}

// Set via Blynk:
BLYNK_WRITE(V29) {  // Home LED
  Home = param.asInt();
  Blynk.virtualWrite(V29, Home * 1023);
}
```

**ESPHome Implementation** (Home Assistant automations)
```yaml
input_select:
  presence_mode:
    name: Presence
    options:
      - home
      - away
    initial: home

automation:
  - alias: "Away Mode Override"
    trigger:
      entity_id: input_select.presence_mode
      to: "away"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.relay

  - alias: "Home Mode Allow"
    trigger:
      entity_id: input_select.presence_mode
      to: "home"
    action:
      # Climate component resumes normal operation
      - service: climate.set_temperature
        target:
          entity_id: climate.gazebo_thermostat
        data:
          temperature: "{{ states('input_number.desired_temperature') }}"
```

**Mapping Notes**:
- `Home` variable → `input_select.presence_mode` (home/away)
- Blynk V29 virtual pin → Home Assistant entity state
- Away logic: automation turns off relay when presence="away"
- IFTTT integration for location triggers → Home Assistant device_tracker or zone automation

**Implementation Status**: ✅ Phase 3

---

### Feature 3: Manual Run/Stop with 15-Minute Timeout

**Arduino Implementation** (blynkModule.cpp, thermostat.cpp)
```cpp
extern boolean ManualRun;
extern boolean ManualStop;

void TempUpdate() {
  if (!ManualRun && !ManualStop) {
    // Normal thermostat logic
  } else if (ManualRun) {
    Fan(1);  // Force relay on
    // Original code doesn't explicitly timeout manual mode
    // This appears to be handled by Blynk timer or user interaction
  } else if (ManualStop) {
    Fan(0);  // Force relay off
  }
}

// Blynk V6 handles echo/manual control
BLYNK_WRITE(V6) {
  if (param.asInt() == 1) ManualRun = 1;
  else ManualRun = 0;
}
```

**ESPHome Implementation** (Home Assistant)
```yaml
switch:
  - id: manual_run_switch
    platform: gpio
    pin: GPIO12  # Relay control
    name: Manual Run
    # (controlled via Home Assistant)

input_boolean:
  manual_run:
    name: Manual Run Mode
    initial: off
  manual_stop:
    name: Manual Stop Mode
    initial: off

automation:
  - alias: "Manual Run Timeout"
    trigger:
      platform: time_pattern
      minutes: /1  # Check every minute
    condition:
      condition: state
      entity_id: input_boolean.manual_run
      state: "on"
    action:
      - condition: template
        value_template: '{{ (now() - states.input_boolean.manual_run.last_changed).total_seconds() > 900 }}'
      - service: input_boolean.turn_off
        target:
          entity_id: input_boolean.manual_run
      - service: switch.turn_off
        target:
          entity_id: switch.relay

  - alias: "Manual Run Relay Control"
    trigger:
      entity_id: input_boolean.manual_run
      to: "on"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.relay

  - alias: "Manual Stop Relay Control"
    trigger:
      entity_id: input_boolean.manual_stop
      to: "on"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.relay
```

**Mapping Notes**:
- `ManualRun` variable → `input_boolean.manual_run`
- `ManualStop` variable → `input_boolean.manual_stop`
- 15-minute timeout implemented via Home Assistant automation (not original Arduino code, but required feature per spec)
- Blynk V6 → Home Assistant UI switch entity
- Override priority: Away > ManualStop > ManualRun > Thermostat

**Implementation Status**: ✅ Phase 3

---

### Feature 4: Temperature Scheduling (Timer)

**Arduino Implementation** (blynkModule.cpp, thermostat.cpp)
```cpp
extern long BeginTimer;      // Start time (Unix timestamp or seconds)
extern long EndTimer;        // End time
extern long PreviousBeginTimer;
extern long PreviousEndTimer;
extern bool TimerOn;

void OtherUpdates() {
  if (BeginTimer != PreviousBeginTimer) {
    EEPROM.put(10, BeginTimer);
    EEPROM.commit();
    PreviousBeginTimer = BeginTimer;
  }
  if (EndTimer != PreviousEndTimer) {
    EEPROM.put(16, EndTimer);
    EEPROM.commit();
    PreviousEndTimer = EndTimer;
  }
}

// TempUpdate() checks:
if (TimerOn) {
  // If within schedule window, run thermostat
}
```

**ESPHome Implementation** (Home Assistant)
```yaml
input_datetime:
  timer_begin:
    name: Timer Begin Time
    has_date: false
    has_time: true
    initial: "08:00"
  timer_end:
    name: Timer End Time
    has_date: false
    has_time: true
    initial: "22:00"

input_boolean:
  timer_enabled:
    name: Timer Enabled
    initial: on

automation:
  - alias: "Timer Schedule Evaluation"
    trigger:
      platform: time_pattern
      minutes: /1  # Check every minute
    action:
      - service: input_boolean.turn_on_off  # Dynamic decision
        target:
          entity_id: binary_sensor.timer_active
        data:
          state: |
            {%- set now = now().strftime("%H:%M") -%}
            {%- set begin = states('input_datetime.timer_begin')[0:5] -%}
            {%- set end = states('input_datetime.timer_end')[0:5] -%}
            {%- if begin <= now < end -%}
              on
            {%- else -%}
              off
            {%- endif -%}

  - alias: "Timer Off Outside Schedule"
    trigger:
      entity_id: binary_sensor.timer_active
      to: "off"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.relay
```

**Mapping Notes**:
- `BeginTimer` / `EndTimer` → `input_datetime.timer_begin/end`
- `TimerOn` → `input_boolean.timer_enabled` (user control) + `binary_sensor.timer_active` (current status)
- EEPROM storage → Home Assistant database (automatic)
- Original code uses Unix timestamps; ESPHome uses HH:MM format (simpler for users)
- Timer schedule blocks relay activation; must be combined with other conditions

**Implementation Status**: ✅ Phase 4

---

### Feature 5: DS18B20 Temperature Sensor

**Arduino Implementation** (settings.h, ds18b20.cpp)
```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

const int oneWireBus = 4;  // GPIO4
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void startDs18b20() {
  sensors.begin();
  sensors.setResolution(12);  // 0.0625°C resolution
}

// In TempUpdate():
sensors.requestTemperatures();
float ReadF = sensors.getTempCByIndex(0);
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
one_wire:
  - platform: gpio
    id: ow_bus
    pin: GPIO4

sensor:
  - platform: dallas_temp
    one_wire_id: ow_bus
    id: gazebo_temp
    name: Gazebo DS18B20
    address: 0x0000000000000000  # (find during setup)
    resolution: 12  # 0.0625°C resolution
    filters:
      - throttle_average: 10s  # 10-second averaging
      - lambda: |-
          // Apply temperature correction offset
          return x + id(temp_offset).state;
      - clamp:
          min_value: -40
          max_value: 85
      - delta: 1.0  # Only update if change > 1°C (stability)
```

**Mapping Notes**:
- `OneWire` library → ESPHome `one_wire` platform
- `DallasTemperature` library → ESPHome `dallas_temp` sensor platform
- `sensors.requestTemperatures()` → Automatic in ESPHome (update interval configurable)
- `ReadF = sensors.getTempCByIndex(0)` → `sensor.gazebo_temp.state` in Home Assistant
- Sensor address needs to be discovered during initial setup (ESPHome provides this)
- Filter `throttle_average: 10s` mimics original averaging
- `clamp` filter prevents out-of-range readings
- `delta: 1.0` implements rate limiting (max ±1°C per update)

**Implementation Status**: ✅ Phase 1 & 2

---

### Feature 6: Nextion Display Integration

**Arduino Implementation** (printCurrentWx.cpp, main.cpp)
```cpp
#include <EasyNextionLibrary.h>

EasyNex myNex(Serial2);

void setup() {
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  myNex.begin(9600);
}

void TempUpdate() {
  myNex.writeNum("three.n1.val", ReadF + TempCorrection);
}

void loop() {
  myNex.NextionListen();  // Listen for touch events
}

// Touch event handler
void trigger1() {
  printCurrentWeather();
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
uart:
  - id: uart_nextion
    tx_pin: GPIO17
    rx_pin: GPIO16
    baud_rate: 9600

display:
  - platform: nextion
    uart_id: uart_nextion
    id: gazebo_nextion
    tft_url: "http://192.168.2.10/gazebo.tft"  # Display firmware upload
    lambda: |-
      // Update display components
      // Note: Ensure 'three.n1' is accessible (Page 3 active or component is global)
      it.set_component_value("three.n1", int(id(gazebo_temp).state + id(temp_offset).state));
      it.set_component_value("three.n0", (int)id(desired_temp).state);
      it.set_component_text("status_text", id(status_msg).state.c_str());

text_sensor:
  - platform: template
    id: status_msg
    update_interval: 1s
    lambda: |-
      if (id(sensor_malfunction).state) {
        return "<<< SENSOR MALFUNCTION >>>";
      }
      if (id(manual_run).state) {
        return "MANUAL MODE";
      }
      return "OK";
```

**Mapping Notes**:
- `EasyNex` library → ESPHome `nextion` display platform
- `Serial2` (GPIO16/17) → ESPHome `uart` component
- `myNex.writeNum()` → `it.set_component_value()` in lambda
- `myNex.NextionListen()` → Handled automatically by ESPHome
- Touch events → ESPHome on_touch callbacks (different from EasyNex)
- Display update frequency: every 1-2 seconds (vs. continuous in Arduino)
- Status messages: conditional text display based on system state

**Implementation Status**: ✅ Phase 6

---

### Feature 7: Relay Control (GPIO12)

**Arduino Implementation** (main.cpp, thermostat.cpp)
```cpp
#define RelayPin 12

void setup() {
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);  // Off at startup
}

void Fan(boolean RunFan) {
  if (RunFan) {
    digitalWrite(RelayPin, HIGH);   // On
    FanState = 1;
  } else {
    digitalWrite(RelayPin, LOW);    // Off
    FanState = 0;
  }
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
switch:
  - platform: gpio
    id: relay
    pin: GPIO12
    name: Gazebo Relay
    restore_mode: ALWAYS_OFF  # Off at startup
    icon: mdi:fire

  # User-facing controls (virtual switches)
  - platform: template
    id: manual_run
    name: Manual Run
    turn_on_action:
      - switch.turn_on: relay
      - logger.log: "Manual Run activated"
    turn_off_action:
      - switch.turn_off: relay
      - logger.log: "Manual Run deactivated"
```

**Mapping Notes**:
- `digitalWrite(RelayPin, HIGH/LOW)` → ESPHome switch control
- `Fan(boolean)` function → Switch on/off actions
- `FanState` variable → `switch.relay.state` in Home Assistant
- `restore_mode: ALWAYS_OFF` ensures safe startup (relay off)
- Template switch separates physical relay from user control (allows overrides)

**Implementation Status**: ✅ Phase 1

---

### Feature 8: WiFi & Network Management

**Arduino Implementation** (settings.h, main.cpp)
```cpp
#include <WiFiManager.h>

#define WIFI_SSID     "NH2"
#define WIFI_PASSWORD "seawolfpilot01"

void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WIFI. Rebooting...");
    ESP.restart();
  }
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "Gazebo-Wx Fallback"
    password: "gazebo1234"

  # Manual IP configuration (optional)
  manual_ip:
    static_ip: 192.168.2.167
    gateway: 192.168.2.1
    subnet: 255.255.255.0

text_sensor:
  - platform: wifi_info
    ip_address:
      name: Gazebo IP
    ssid:
      name: Gazebo SSID
    bssid:
      name: Gazebo BSSID

sensor:
  - platform: wifi_signal
    name: Gazebo WiFi Signal
    update_interval: 60s
```

**Mapping Notes**:
- Hardcoded credentials → `!secret` file in ESPHome (not in code)
- WiFiManager library → ESPHome native WiFi with fallback AP
- Fallback AP automatically created if configured SSID unavailable
- `checkWifi()` & restart → ESPHome continues operation without network (graceful degradation)
- IP address exposed as text_sensor for monitoring

**Implementation Status**: ✅ Phase 1

---

### Feature 9: OTA Firmware Updates

**Arduino Implementation** (nh_ota.cpp, main.cpp)
```cpp
#include <ArduinoOTA.h>

void ota() {
  ArduinoOTA
    .onStart([]() { Serial.println("OTA Start"); })
    .onEnd([]() { Serial.println("OTA End"); })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA Progress: %u%%\r\n", (progress / (total / 100)));
    });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  // ...
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
ota:
  - platform: esphome
    password: !secret ota_password  # Optional password for security

# Build & push via ESPHome dashboard or CLI:
# esphome run gazebo_stove.yaml
```

**Mapping Notes**:
- `ArduinoOTA` → ESPHome native `ota` platform
- Manual OTA via CLI → ESPHome dashboard or `esphome run` command
- OTA works over WiFi; no USB cable needed
- Password optional for security (recommended in production)
- ESPHome automatic handling of progress and restart

**Implementation Status**: ✅ Phase 1

---

### Feature 10: Time Synchronization (NTP)

**Arduino Implementation** (ntpServer.cpp, myTime.cpp)
```cpp
#include <Time.h>

static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = -4;  // EDT

void getTime() {
  // Sync time from NTP
  setSyncProvider(getNtpTime);
  setSyncInterval(300);  // Sync every 5 minutes
}

long getNtpTime() {
  // NTP polling logic
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
time:
  - platform: sntp
    id: sntp_time
    servers:
      - us.pool.ntp.org
      - time.nist.gov
    timezone: America/New_York
    on_time_sync:
      then:
        - logger.log: "Time synced"

text_sensor:
  - platform: template
    id: current_time
    update_interval: 1s
    lambda: |-
      return id(sntp_time).now().strftime("%H:%M:%S");
```

**Mapping Notes**:
- `<Time.h>` library → ESPHome `sntp` time platform
- `setSyncProvider()` → Automatic SNTP in ESPHome
- `setSyncInterval(300)` → Configurable (default 6 hours)
- Timezone handling → ESPHome `timezone` parameter
- Current time for display → Template sensor

**Implementation Status**: ✅ Phase 4

---

### Feature 11: EEPROM Settings Persistence

**Arduino Implementation** (eepromPresets.cpp, settings.h)
```cpp
#include <EEPROM.h>

void EEPROM.begin(20);

// Write settings:
EEPROM.write(3, TempDes);
EEPROM.put(10, BeginTimer);
EEPROM.commit();

// Read settings:
void GetPresets() {
  TempDes = EEPROM.read(3);
  BeginTimer = EEPROM.get(10);
}
```

**ESPHome Implementation** (Home Assistant automations)
```yaml
# Settings stored as Home Assistant input_* entities
globals:
  - id: desired_temp
    type: float
    restore_value: no
    initial_value: '22.0'
  - id: temp_offset
    type: float
    restore_value: no
    initial_value: '0.0'

input_number:
  desired_temperature:
    name: Desired Temperature
    min: 5
    max: 35
    step: 0.5
    unit_of_measurement: "°C"
    restore: true  # Persist value

input_datetime:
  timer_begin:
    name: Timer Begin
    has_time: true
    restore: true

# ESPHome reads from Home Assistant on startup
on_boot:
  priority: 600
  then:
    - lambda: |-
        id(desired_temp).publish_state(
          Home Assistant state retrieved
        );

# Home Assistant database handles persistence automatically
```

**Mapping Notes**:
- EEPROM 20-byte limit → Home Assistant database (unlimited)
- EEPROM addresses → Home Assistant entity_id
- Manual EEPROM commit → Automatic Home Assistant persistence
- Settings loaded on boot from Home Assistant database
- No data loss risk from EEPROM corruption
- Backup/restore via Home Assistant snapshot feature

**Implementation Status**: ✅ Phase 8

---

### Feature 12: Sensor Malfunction Detection

**Arduino Implementation** (blynkModule.cpp, thermostat.cpp)
```cpp
extern int BadRead;              // Counts consecutive failed reads

void TempUpdate() {
  sensors.requestTemperatures();
  float ReadF = sensors.getTempCByIndex(0);

  if (isnan(ReadF)) {
    BadRead++;
    return;
  }
  BadRead = 0;  // Reset counter on good read
}

void OtherUpdates() {
  if (MenuItem == 0 && !ButtonPressed) {
    if (BadRead > 10) {
      Blynk.virtualWrite(V10, String("<<< SENSOR MALFUNCTION >>>"));
      BadRead = 0;
      if (!ManualRun) {  // Manual mode supersedes
        Fan(0);
      }
    }
  }
}
```

**ESPHome Implementation** (gazebo_stove.yaml)
```yaml
globals:
  - id: bad_read_count
    type: int
    restore_value: no
    initial_value: '0'

sensor:
  - platform: dallas_temp
    one_wire_id: ow_bus
    id: gazebo_temp
    filters:
      - lambda: |-
          if (isnan(x)) {
            id(bad_read_count) += 1;
            if (id(bad_read_count) > 10) {
              id(sensor_malfunction).publish_state(true);
              // Turn off relay in safety
            }
            return {};  // Return NaN to skip update
          }
          id(bad_read_count) = 0;  // Reset counter
          if (id(sensor_malfunction).state) {
            id(sensor_malfunction).publish_state(false);
          }
          return x;

binary_sensor:
  - platform: template
    id: sensor_malfunction
    name: Sensor Malfunction
    on_state:
      then:
        - if:
            condition:
              binary_sensor.is_on: sensor_malfunction
            then:
              - switch.turn_off: relay  # Safety disable
              - logger.log: "SENSOR MALFUNCTION - Relay disabled"
```

**Mapping Notes**:
- `BadRead` counter → ESPHome lambda variable (per-sensor)
- `isnan(ReadF)` check → ESPHome filter lambda
- 10+ consecutive bad reads → binary_sensor activation
- Alarm message → text_sensor or Nextion display message
- Relay cutoff → Automatic on alarm (safety feature)

**Implementation Status**: ✅ Phase 5

---

### Feature 13: Perceived Temperature (NOT IMPLEMENTED - Heating-Only System)

**Status**: ❌ NOT APPLICABLE - Heating-only stove system

**Rationale**:
- Original feature required humidity sensor (DHT22/DHT11) which is not deployed
- Perceived temperature was only used in Summer mode for comfort calculation
- This is a heating-only stove system with no summer cooling operation
- Feature removed to simplify configuration and reduce dependencies

**Original Code** (for reference only):
```cpp
// Original Arduino code supported perceived temperature
// if(UsePerceivedTemp == true && Winter == false && ReadF > 20 && Humidity > 40)
// Formula: perceived_temp = actual_temp + (humidity - 40) / 10
```

**ESPHome Implementation**: REMOVED
- No humidity sensor input required
- No perceived temperature calculation needed
- Uses actual temperature directly for heating control

**Implementation Status**: ✅ Not needed for heating-only stove

---

### Feature 14: Settings Menu (Blynk Dashboard)

**Arduino Implementation** (blynkModule.cpp)
```cpp
int MenuItem = 0;              // Current menu item
boolean ButtonPressed = false; // Settings button state

void NextMenuItem() {
  MenuItem = (MenuItem + 1) % NumMenuItems;
}

// Blynk virtual pins for each menu item:
BLYNK_WRITE(V1) {  // Desired temperature
  TempDes = param.asInt();
}

BLYNK_WRITE(V3) {  // Settings selector
  MenuItem = param.asInt();
}
```

**ESPHome Implementation** (Home Assistant UI)
```yaml
# Replace Blynk dashboard with Home Assistant entities
input_number:
  desired_temperature:
    name: Desired Temperature
    min: 5
    max: 35
    step: 0.5
  heating_hysteresis:
    name: Heating Hysteresis
    min: 1
    max: 5
    step: 0.5
  temperature_offset:
    name: Temperature Offset
    min: -5
    max: 5
    step: 0.1

input_select:
  operation_mode:
    name: Mode
    options:
      - auto
      - manual_run
      - manual_stop

# Create Home Assistant dashboard card to display all settings
# Users interact with HA UI instead of Blynk mobile app
```

**Mapping Notes**:
- Blynk virtual pins → Home Assistant input_* entities
- MenuItem navigation → No longer needed (HA UI is flat, not hierarchical)
- Button-press menu selection → Direct entity editing in HA UI
- Dashboard colors (Blynk feature) → HA UI theming (not migrated, not critical)
- Settings accessible from web UI, mobile app, or physical Nextion display

**Implementation Status**: ✅ Phase 2-4

---

## Data Structure Mapping

### Variable Mapping Table

| Arduino Variable | Type | EEPROM | Blynk Pin | ESPHome/HA Equivalent | Entity Type | Persistent |
|---|---|---|---|---|---|---|
| `TempDes` | int | 3 | V1 | `input_number.desired_temperature` | number | ✅ |
| `TempAct` | int | - | V0 | `sensor.gazebo_temperature` | sensor | - |
| `TempAvg` | int | - | - | Template calculation | sensor | - |
| `TempCorrection` | int | - | - | `input_number.temperature_offset` | number | ✅ |
| `LastRead` | float | - | - | Internal filter state | - | - |
| `Humidity` | int | - | V2 | `sensor.humidity_estimated` | sensor | - |
| `BeginTimer` | long | 10 | - | `input_datetime.timer_begin` | datetime | ✅ |
| `EndTimer` | long | 16 | - | `input_datetime.timer_end` | datetime | ✅ |
| `PreviousBeginTimer` | long | - | - | (HA comparison) | - | - |
| `PreviousEndTimer` | long | - | - | (HA comparison) | - | - |
| `TimerOn` | bool | - | - | `input_boolean.timer_enabled` | boolean | ✅ |
| `Hysteresis` | int | - | - | `input_number.heating_hysteresis` | number | ✅ |
| `UsePerceivedTemp` | bool | - | - | (not applicable, heating-only) | - | - |
| `Season` | bool | - | - | (not applicable, heating-only) | - | - |
| `Home` | bool | - | V29 | `input_select.presence_mode` | select | ✅ |
| `ManualRun` | bool | - | V6 | `input_boolean.manual_run` | boolean | - |
| `ManualStop` | bool | - | - | `input_boolean.manual_stop` | boolean | - |
| `MenuItem` | int | - | V3 | (not needed in HA UI) | - | - |
| `ButtonPressed` | bool | - | - | (Nextion touch event) | - | - |
| `FanState` | bool | - | - | `switch.relay.state` | switch | - |
| `BadRead` | int | - | - | (internal counter) | binary_sensor | - |
| `NormalWidgetColor` | String | - | - | (HA theming, not migrated) | - | - |

---

## Function Mapping

### Core Logic Functions

| Arduino Function | Location | Purpose | ESPHome Equivalent |
|---|---|---|---|
| `TempUpdate()` | thermostat.cpp | Main thermostat control loop | ESPHome climate component |
| `Fan(bool)` | thermostat.cpp | Toggle relay on/off | `switch.turn_on/off` service |
| `TimerStatus()` | blynkModule.cpp | Check if within timer window | Home Assistant automation |
| `KillManual()` | blynkModule.cpp | Clear manual mode | Automation timeout |
| `LongHoldDetect()` | blynkModule.cpp | Detect long button press | Nextion touch with delay |
| `BlynkInit()` | blynkModule.cpp | Initialize Blynk connection | ESPHome API (automatic) |
| `MenuReset()` | blynkModule.cpp | Reset settings menu | Not needed (HA UI) |
| `GetPresets()` | eepromPresets.cpp | Load settings from EEPROM | Home Assistant DB read |
| `startDs18b20()` | ds18b20.cpp | Initialize temperature sensor | `dallas_temp` component |
| `getTime()` | myTime.cpp | Sync time from NTP | SNTP component |
| `printCurrentWeather()` | printCurrentWx.cpp | Fetch and display weather | OpenWeatherMap API + display |

### Display Functions

| Arduino Function | Purpose | ESPHome Equivalent |
|---|---|---|
| `sendNextion()` | Send data to Nextion display | Nextion display lambda |
| `whichIcon()` | Map weather ID to icon | Template sensor filter |
| `digitalClockDisplay()` | Format and display time | Text sensor template |

---

## Configuration & Compilation

### Arduino Build Configuration

**platformio.ini**
```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = wemos_d1_mini32
framework = arduino

lib_deps =
  blynkkk/Blynk@^0.6.7
  https://github.com/Bodmer/OpenWeather.git
  https://github.com/tzapu/WiFiManager.git
  paulstoffregen/Timelib
  milesburton/DallasTemperature@^3.9.1
  paulstoffregen/OneWire@^2.3.5
  seithan/Easy Nextion Library@^1.0.6
```

### ESPHome Configuration

**gazebo_stove.yaml**
```yaml
esphome:
  name: gazebo-wx
  platform: ESP32
  board: esp32 wroom

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

api:
  # Automatic Home Assistant integration

ota:
  - platform: esphome

# (All components defined in YAML)
```

**Dependencies Mapping**
| Arduino Library | ESPHome Component |
|---|---|
| Blynk | (removed - use Home Assistant API) |
| BlynkSimpleEsp32 | (removed) |
| WiFiManager | (built-in WiFi with fallback) |
| Timelib | (sntp time platform) |
| DallasTemperature | (dallas_temp sensor) |
| OneWire | (one_wire platform) |
| EasyNextionLibrary | (nextion display) |
| OpenWeather | (http_request + JSON parsing) |

---

## Known Gaps & Clarifications Needed

1. **Humidity Sensor**: Code mentions DHT22/DHT11 in comments but hardware uses DS18B20 only. Need clarification:
   - [ ] Is humidity sensor actually deployed?
   - [ ] If yes, which GPIO and sensor model?
   - [ ] If no, disable perceived temperature feature

2. **OpenWeatherMap Location**: Current code has hardcoded forecast location in library config. Need:
   - [ ] Desired location (latitude/longitude or city name)
   - [ ] API key access (currently hardcoded)
   - [ ] Fallback if API unavailable

3. **Nextion Display Model**: Comments show page/component references ("three.n0.val") but unclear:
   - [ ] Exact Nextion model/resolution
   - [ ] Display firmware version
   - [ ] Available pages and component IDs

4. **Widget Color Preference**: Blynk feature to set dashboard colors (red/blue). Need:
   - [ ] Is this important for users?
   - [ ] How to represent in Home Assistant UI (theme/icon colors)

5. **IFTTT Integration**: Current setup requires manual IFTTT recipe. Options:
   - [ ] Continue with IFTTT webhooks
   - [ ] Migrate to Home Assistant device_tracker (modern approach)
   - [ ] Both for migration period

6. **Original Timeout Logic**: Code doesn't explicitly timeout Manual Run mode. Need:
   - [ ] Confirm 15-minute timeout is required (per spec)
   - [ ] How was it enforced in original? (missing in code)

---

## Implementation Checklist

- [ ] **Phase 1**: GPIO mapping verified
- [ ] **Phase 1**: Sensor reads verified
- [ ] **Phase 1**: Relay control verified
- [ ] **Phase 1**: WiFi & API functional
- [ ] **Phase 2**: Climate component configured
- [ ] **Phase 2**: Thermostat hysteresis tested
- [ ] **Phase 3**: Home/Away mode tested
- [ ] **Phase 3**: Manual modes with timeout tested
- [ ] **Phase 4**: Timer scheduling tested
- [ ] **Phase 4**: NTP time sync verified
- [ ] **Phase 5**: Sensor malfunction alarm tested
- [ ] **Phase 6**: Nextion display updates verified
- [ ] **Phase 7**: Weather forecast integration tested
- [ ] **Phase 8**: Settings persistence across reboot
- [ ] **Phase 9**: 72-hour stability test passed
- [ ] **Phase 10**: Production deployment complete

---

## References & Source Code Locations

**Original Arduino Source**:
- Main: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/src/main.cpp:1-175`
- Thermostat: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/src/thermostat.cpp`
- Blynk Integration: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/src/blynkModule.cpp`
- Settings: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/src/settings.h`
- Display: `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/src/printCurrentWx.cpp`

**Existing ESPHome Config**:
- `/Volumes/Docs/Electronics/Projects/NH/GazeboWx/gazebo_stove.yaml`

**Target Project**:
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/`

---

**Document Status**: Complete Analysis
**Date**: 2025-11-10
**Next Step**: Ready for implementation planning review and stakeholder sign-off
