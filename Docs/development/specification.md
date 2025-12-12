# GazeboWx ESPHome Conversion - Feature Specification

**Feature**: Gazebo Propane Stove Heating Control System (Heating-Only)
**Created**: 2025-11-10
**Status**: Analysis & Planning Phase
**Source**: PlatformIO project at `/Volumes/Docs/Electronics/Projects/NH/GazeboWx`

---

## Executive Summary

GazeboStove is a smart heating control system for an outdoor gazebo with a propane stove, currently built on Arduino/Blynk architecture. This specification documents the conversion to ESPHome + Home Assistant, eliminating cloud dependency while maintaining all heating functionality. The system monitors temperature via DS18B20 sensor, controls a propane stove heating relay, displays weather and setpoints on a Nextion touchscreen, and provides remote control through Home Assistant (replacing Blynk). **Note**: As this is a stove system, it can only provide heating and remains inactive during summer months. The thermostat logic is heating-only with no cooling capability.

---

## User Scenarios & Testing

### User Story 1 - Automatic Temperature Control (Priority: P1)

**Description**: As a gazebo user, I want the heating system to automatically maintain the gazebo at my desired temperature, so I don't have to manually adjust the stove.

**Why this priority**: Core heating functionality; primary value proposition of the system.

**Independent Test**: Can be fully tested by setting a temperature setpoint in Home Assistant and observing relay activation/deactivation as ambient temperature crosses hysteresis thresholds. Requires only temperature sensor and relay functional.

**Acceptance Scenarios**:
1. **Given** Winter mode is active, **When** temperature drops below setpoint, **Then** relay activates and heating starts within 5 seconds
2. **Given** Heating is running, **When** temperature reaches (setpoint + hysteresis), **Then** relay deactivates within 10 seconds
3. **Given** Temperature is stable, **When** setpoint changes by 1°C, **Then** system responds within 15 seconds
4. **Given** Sensor fails to read, **When** 10+ consecutive bad reads occur, **Then** relay deactivates and alarm is logged

---

### User Story 2 - Home/Away Mode with Scheduling (Priority: P1)

**Description**: As a homeowner, I want the system to only heat when I'm home, and I want to pre-schedule heating for specific hours, so I don't heat an empty gazebo.

**Why this priority**: Prevents energy waste; essential for practical operation.

**Independent Test**: Can be tested by toggling Home/Away mode in Home Assistant and verifying relay remains off when Away. Schedule timer can be tested by setting timer windows and observing heating response.

**Acceptance Scenarios**:
1. **Given** Away mode is active, **When** temperature is below setpoint, **Then** relay stays off regardless of temperature
2. **Given** Home mode is active and timer is off, **When** time is outside scheduled window, **Then** relay stays off
3. **Given** Home mode and timer is on, **When** current time is within scheduled window and temperature is below setpoint, **Then** relay activates
4. **Given** Timer schedule changes, **When** new begin/end times are set, **Then** settings persist across reboot (EEPROM equivalent in Home Assistant)

---

### User Story 3 - Manual Control Overrides (Priority: P1)

**Description**: As a gazebo user, I want to manually turn the heater on or off for immediate effect, overriding automatic controls.

**Why this priority**: Users need emergency control and ability to run heater on-demand.

**Independent Test**: Can be tested by setting manual run/stop switches in Home Assistant and verifying relay state changes immediately regardless of thermostat algorithm.

**Acceptance Scenarios**:
1. **Given** Automatic mode is active, **When** Manual Run is enabled, **Then** relay turns on immediately and stays on
2. **Given** Automatic mode is active, **When** Manual Stop is enabled, **Then** relay turns off immediately and stays off
3. **Given** Manual Run is active, **When** 15 minutes elapse, **Then** Manual Run times out and automatic mode resumes
4. **Given** Manual Run is active, **When** Manual Stop is enabled, **Then** relay turns off and automatic mode resumes

---

### User Story 4 - Real-Time Monitoring & Display (Priority: P2)

**Description**: As a gazebo user checking the system status, I want to see current temperature, humidity, desired setpoint, and relay status on the Nextion display without opening an app.

**Why this priority**: Provides local feedback; improves user experience; helps troubleshoot issues.

**Independent Test**: Can be tested by power-cycling the device and verifying Nextion display shows correct values immediately, independent of WiFi/cloud.

**Acceptance Scenarios**:
1. **Given** Device is powered on, **When** startup completes, **Then** Nextion display shows current temp, setpoint, and mode within 5 seconds
2. **Given** Temperature changes by ≥1°C, **When** sensor reads new value, **Then** Nextion display updates within 2 seconds
3. **Given** Setpoint is changed in Home Assistant, **When** change is received, **Then** Nextion display updates setpoint within 2 seconds
4. **Given** Relay state changes, **When** relay switches on/off, **Then** Nextion display updates heating indicator within 1 second

---

### User Story 5 - Weather Forecast Display (Priority: P2)

**Description**: As a gazebo user, I want to see the weather forecast on the Nextion display, so I can plan outdoor activities.

**Why this priority**: Nice-to-have; adds value but not essential for core heating function.

**Independent Test**: Can be tested by verifying OpenWeatherMap API integration and Nextion display update with weather icons and conditions. Requires valid API key and internet connection.

**Acceptance Scenarios**:
1. **Given** OpenWeatherMap API key is configured, **When** device connects to WiFi, **Then** weather forecast is fetched within 30 seconds
2. **Given** Weather forecast is received, **When** parsing completes, **Then** Nextion display shows current conditions and 5-day forecast within 5 seconds
3. **Given** Forecast data is cached, **When** WiFi disconnects, **Then** last known forecast remains displayed

---

### User Story 6 - Settings Configuration (Priority: P2)

**Description**: As a system administrator, I want to adjust thermostat hysteresis, temperature offset, and timing parameters without recompiling code.

**Why this priority**: Enables field tuning and calibration without development cycle.

**Independent Test**: Can be tested by setting various configuration values in Home Assistant and observing thermostat behavior changes. Requires no relay activation; pure logic verification.

**Acceptance Scenarios**:
1. **Given** Heating hysteresis is set to 3°C, **When** temperature reaches (setpoint + 3), **Then** relay deactivates
2. **Given** Heating hysteresis is set to 2°C, **When** temperature rises to (setpoint + 2), **Then** relay deactivates
3. **Given** Temperature offset is set to +2°C, **When** raw sensor reads 20°C, **Then** displayed temperature is 22°C
4. **Given** Perceived temperature is enabled, **When** humidity is high (>60%) and temperature is warm, **Then** displayed temp is increased by humidity factor

---

### User Story 7 - Remote Control via Mobile/Web (Priority: P2)

**Description**: As a homeowner away from the gazebo, I want to control heating and view status from my phone/web browser.

**Why this priority**: Essential for remote monitoring and emergency adjustments.

**Independent Test**: Can be tested by accessing Home Assistant UI from another device, changing setpoint/mode, and verifying device responds correctly.

**Acceptance Scenarios**:
1. **Given** Home Assistant is accessible remotely, **When** user changes setpoint via Home Assistant UI, **Then** device receives update within 2 seconds
2. **Given** Device is updated remotely, **When** change is applied, **Then** Nextion display and relay state reflect change
3. **Given** User changes away/home mode remotely, **When** device receives command, **Then** heating behavior changes immediately

---

### User Story 8 - OTA Firmware Updates (Priority: P2)

**Description**: As a system administrator, I want to update device firmware over WiFi without physical access.

**Why this priority**: Enables bug fixes and features without site visit.

**Independent Test**: Can be tested by triggering OTA update through Home Assistant / ESPHome dashboard and verifying device reboots with new firmware.

**Acceptance Scenarios**:
1. **Given** New firmware is available, **When** OTA update is initiated, **Then** device downloads and flashes new firmware within 2 minutes
2. **Given** OTA update is in progress, **When** update completes, **Then** device reboots automatically and reports new firmware version
3. **Given** Settings and calibration data exist, **When** OTA update completes, **Then** all settings persist (no EEPROM loss)

---

### User Story 9 - WiFi Resilience (Priority: P2)

**Description**: As a gazebo user with unstable WiFi, I want the heating system to continue working even if WiFi is lost.

**Why this priority**: Ensures safety and reliability of heating control.

**Independent Test**: Can be tested by disconnecting WiFi and verifying thermostat continues to operate with last-known setpoint and Home/Away mode.

**Acceptance Scenarios**:
1. **Given** WiFi is connected, **When** WiFi signal is lost, **Then** thermostat continues running with cached setpoint for up to 1 hour
2. **Given** WiFi is lost, **When** device cannot connect to configured SSID, **Then** creates fallback AP ("Gazebo-Wx Fallback") so user can reconfigure
3. **Given** WiFi is lost, **When** WiFi becomes available, **Then** device auto-reconnects without user intervention
4. **Given** WiFi is lost during manual run, **When** 15-minute timer expires, **Then** manual mode timeout still works

---

### User Story 10 - Sensor Fault Detection & Alarm (Priority: P3)

**Description**: As a system operator, I want to be alerted if the temperature sensor fails, so I know heating may be uncontrolled.

**Why this priority**: Safety feature; prevents unnoticed equipment failures.

**Independent Test**: Can be tested by forcing bad sensor reads and verifying alarm notification appears in Home Assistant and Nextion display.

**Acceptance Scenarios**:
1. **Given** Sensor fails to read, **When** 10 consecutive bad reads occur, **Then** alarm message appears on Nextion display
2. **Given** Alarm is active, **When** alarm condition is triggered, **Then** relay deactivates to prevent uncontrolled heating
3. **Given** Alarm is active, **When** sensor returns valid readings, **Then** alarm clears automatically after 1 good read

---

### User Story 11 - Perceived Temperature (Humidity-Adjusted) (Priority: P3)

**Description**: As a comfort-conscious user, I want the system to account for humidity when determining "feels like" temperature.

**Why this priority**: Improves comfort in humid conditions; low priority but implemented in original.

**Independent Test**: Can be tested by setting humidity values and observing perceived temperature adjustment in thermostat logic.

**Acceptance Scenarios**:
1. **Given** Perceived temperature is enabled, **When** temperature is 25°C and humidity is 70%, **Then** perceived temperature increases by humidity factor (approx. 3°C per 10% above 40%)
2. **Given** Perceived temperature is enabled, **When** season is Winter, **Then** perceived temperature adjustment is disabled (Winter only uses actual temp)

---

## Edge Cases

- **Sensor malfunction**: Device reads are NaN → increment BadRead counter → at 10+ consecutive failures, deactivate relay and alarm
- **WiFi dropout**: Continue thermostat operation with cached setpoint; create fallback AP if unable to reconnect for >5 minutes
- **Manual mode timeout**: After 15 minutes in Manual Run, return to automatic mode; timer persists across WiFi loss
- **Temperature display bounce**: Limit temperature change to ±1°C per reading interval; average with previous reading for stability
- **Rapid setpoint changes**: Accept setpoint changes immediately but apply only within rate-limited display updates
- **Home/Away mode edge case**: If transition occurs during heating cycle, relay state changes immediately
- **Timer schedule boundaries**: Ensure relay off state when current time transitions outside scheduled window
- **OTA during heating**: OTA update interrupts heating briefly; resume heating with same setpoint after reboot
- **NTP time sync failure**: Operate with last-known time; schedule timers may not work until time is synced

---

## Requirements

### Functional Requirements

#### Thermostat Core
- **FR-001**: System MUST maintain temperature within ±(Hysteresis) degrees of setpoint while in Home + Auto mode
- **FR-002**: System MUST respect Home/Away mode - relay OFF when Away regardless of temperature
- **FR-003**: System MUST respect Timer schedule - relay OFF when outside scheduled window hours
- **FR-004**: System MUST support adjustable heating hysteresis (1-5°C range, default 2°C)
- **FR-005**: System MUST implement rate-limited temperature readings (max ±1°C change per update)
- **FR-006**: System MUST support heating-only logic (propane stove cannot cool, inactive in summer)

#### Manual Control
- **FR-007**: System MUST provide Manual Run override that activates relay for 15 minutes regardless of thermostat
- **FR-008**: System MUST provide Manual Stop override that deactivates relay regardless of thermostat
- **FR-009**: System MUST timeout Manual Run mode after exactly 15 minutes
- **FR-010**: System MUST allow Manual modes to be cancelled by user action or other mode selection

#### Sensor & Calibration
- **FR-011**: System MUST read DS18B20 temperature sensor and filter results (10-second averaging)
- **FR-012**: System MUST apply user-configurable temperature offset for sensor calibration (±adjustable range)
- **FR-013**: System MUST detect sensor malfunction (10+ consecutive bad reads) and alarm
- **FR-014**: System MUST NOT use perceived temperature (not applicable for heating-only stove)
- **FR-015**: System MUST NOT adjust temperature for humidity (heating-only operation)

#### Display & Feedback
- **FR-016**: System MUST update Nextion display with current temp, desired temp, mode, and relay status every 2 seconds
- **FR-017**: System MUST display weather forecast (current conditions and 5-day) on Nextion via OpenWeatherMap API
- **FR-018**: System MUST show alarm message on Nextion when sensor malfunction detected
- **FR-019**: System MUST display Home/Away mode and current time on Nextion
- **FR-020**: System MUST display timer status (on/off, begin/end times) on Nextion

#### Configuration & Persistence
- **FR-021**: System MUST persist configuration to equivalent of EEPROM: desired temp, hysteresis values, offsets, timer schedule
- **FR-022**: System MUST load saved settings from Home Assistant on startup
- **FR-023**: System MUST reload configuration if Home Assistant connection drops then reconnects
- **FR-024**: System MUST support batch configuration updates from Home Assistant

#### Connectivity & Remote Control
- **FR-025**: System MUST connect to WiFi and expose all state/control via Home Assistant native API
- **FR-026**: System MUST create fallback AP ("Gazebo-Wx Fallback") if unable to connect to configured SSID
- **FR-027**: System MUST support OTA firmware updates via ESPHome dashboard
- **FR-028**: System MUST maintain operation for ≥1 hour without WiFi (thermostat continues with cached setpoint)
- **FR-029**: System MUST auto-reconnect to WiFi when signal becomes available
- **FR-030**: System MUST expose all settings as Home Assistant climate entity + number/switch entities

#### Time & Scheduling
- **FR-031**: System MUST sync time from NTP server on startup and periodically refresh
- **FR-032**: System MUST support timer begin/end times (format: HH:MM 24-hour)
- **FR-033**: System MUST evaluate timer schedule every minute and update relay control accordingly
- **FR-034**: System MUST operate current time display on Nextion (HH:MM:SS format)

#### Hardware Interface
- **FR-035**: System MUST control relay on GPIO12 (active HIGH for heating)
- **FR-036**: System MUST read OneWire DS18B20 sensor on GPIO4
- **FR-037**: System MUST communicate with Nextion display on UART2 (GPIO16 RX, GPIO17 TX) at 9600 baud
- **FR-038**: System MUST support WiFi module (onboard ESP32 WiFi)
- **FR-039**: System MUST support OTA via WiFi (ESPHome OTA protocol)

### Functional Requirement Clarifications Needed

- **FR-040**: [NEEDS CLARIFICATION] Original mentions both DHT22 and DHT11 sensor in comments; which sensor is actually deployed? (Current implementation uses DS18B20 only, but comments reference DHT humidity)
- **FR-041**: [NEEDS CLARIFICATION] OpenWeatherMap integration - what location/coordinates to fetch forecast for? (Currently hardcoded in library settings)
- **FR-042**: Widget color preference not needed (heating-only operation, no cooling mode)
- **FR-043**: [NEEDS CLARIFICATION] IFTTT integration mentioned for Home/Away via location triggers - replace with Home Assistant presence detection or maintain IFTTT hooks?

### Key Entities & Data Structures

#### Climate Entity (Thermostat)
- **name**: "Gazebo Thermostat"
- **current_temperature**: float (from DS18B20)
- **target_temperature**: float (user setpoint, 5-30°C typical range)
- **min_temp**: 5, **max_temp**: 35 (configurable bounds)
- **hvac_mode**: "heat" | "off" (Heating when Home, Off when Away)
- **current_humidity**: float (not used for heating-only stove)
- **aux_heat**: boolean (Manual Run mode indicator)
- **preset_modes**: ["auto", "manual_run", "manual_stop"]

#### Sensor Entity (Temperature)
- **name**: "Gazebo DS18B20"
- **unit_of_measurement**: "°C"
- **device_class**: "temperature"
- **state**: float (20.5 = 20.5°C)
- **filters**: [throttle_average: 10s]

#### Switch Entities
- **name**: "Gazebo Relay"
- **icon**: "mdi:fire" (when on), "mdi:fire-off" (when off)
- **state**: "on" | "off"
- **device_class**: "outlet" or "switch"

#### Number Entities (Configuration)
- **Desired Temperature Setpoint**: range 5-35°C, step 0.5°C
- **Heating Hysteresis**: range 1-5°C, step 0.5°C (deadband for stove ON/OFF control)
- **Temperature Offset**: range -5 to +5°C, step 0.1°C

#### Select/Mode Entities
- **Presence Mode**: "home" | "away" (Home: allow thermostat control, Away: stove off)
- **Override Mode**: "auto" | "manual_run" | "manual_stop"

#### Text Sensor Entities
- **Display Message**: Shows menu status, alarm messages, etc.
- **WiFi SSID**: Current WiFi network name
- **WiFi IP**: Current IP address
- **WiFi Signal Strength**: dBm value

#### Binary Sensor Entities
- **Sensor Malfunction**: boolean (alarm condition)
- **Timer Active**: boolean (schedule window active)

---

## Success Criteria

### Measurable Outcomes

- **SC-001**: Device maintains temperature within ±(Hysteresis) degrees of setpoint, measured over 1-hour heating cycle (target: 95% of time within range)
- **SC-002**: Relay responds to temperature changes within 10 seconds of sensor reading new value
- **SC-003**: Manual Run timeout works: relay automatically deactivates exactly 15 minutes after activation
- **SC-004**: WiFi dropout resilience: thermostat continues operating ≥60 minutes without connectivity
- **SC-005**: Settings persistence: 100% of configuration values survive power cycle and Home Assistant reconnection
- **SC-006**: Nextion display updates within 2 seconds of state changes (temperature, setpoint, relay, mode)
- **SC-007**: Home Assistant remote control: setpoint changes apply on device within 2 seconds of submission
- **SC-008**: OTA update success rate: 100% successful firmware updates without state loss
- **SC-009**: Sensor fault detection: malfunction alarm appears within 10 minutes of first bad read; relay deactivates within 1 minute

### Feature Parity Verification
- **SC-010**: All 11 Blynk virtual pins (V0-V31) mapped to Home Assistant entities with equivalent functionality
- **SC-011**: All EEPROM settings migrated to Home Assistant number/select entities with persistence
- **SC-012**: Timer scheduling equivalent to original, with visual feedback on Nextion
- **SC-013**: Manual mode and Home/Away mode all implemented and accessible
- **SC-014**: Weather display maintained (OpenWeatherMap → Nextion, no API change required)

### Migration Validation
- **SC-015**: Original Arduino code retained in version control; rollback path documented
- **SC-016**: Blynk and ESPHome can coexist during transition (device supports both simultaneously)
- **SC-017**: All tests pass on target hardware (esp32doit-devkit-v1) before production deployment

---

## Architecture Overview

### System Boundary
```
┌─────────────────────────────────────────────┐
│ GazeboWx ESPHome Device (esp32doit-devkit-v1)   │
│  ┌──────────────┐  ┌──────────────┐         │
│  │ Temperature  │  │ Thermostat   │         │
│  │ Sensor DS18  │  │ Algorithm    │         │
│  │ (GPIO4)      │──│ (Hysteresis) │         │
│  └──────────────┘  └──────┬───────┘         │
│                           │                 │
│                    ┌──────▼──────┐          │
│                    │ Relay Ctrl  │          │
│                    │ (GPIO12)    │          │
│                    └─────────────┘          │
│  ┌──────────────────────────────────────┐  │
│  │ Nextion Display (UART GPIO16/17)    │  │
│  │ - Current/Desired Temps             │  │
│  │ - Weather Forecast                  │  │
│  │ - Mode Status                       │  │
│  └──────────────────────────────────────┘  │
│  ┌──────────────────────────────────────┐  │
│  │ WiFi & Home Assistant API           │  │
│  │ - Remote Control & Monitoring       │  │
│  │ - Configuration Sync                │  │
│  │ - OTA Updates                       │  │
│  └──────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
        │                  │
        ▼                  ▼
    WiFi Network   Home Assistant
                   (Primary UI & Logic)
```

### Configuration vs Code Distribution
- **YAML Configuration** (gazebo_stove.yaml): GPIO mappings, sensor definitions, display setup, climate entity config
- **Automations** (Home Assistant): timer scheduling, presence-based logic, manual mode timeouts
- **Home Assistant UI**: All user-facing controls, settings adjustment, remote access

---

## Dependencies & Technology Stack

### Hardware
- **MCU**: esp32doit-devkit-v1 (ESP32-based board)
- **Sensors**: DS18B20 OneWire temperature sensor
- **Display**: Nextion 3.5" touchscreen (9600 baud UART)
- **Relay**: Single GPIO-controlled relay (active HIGH)
- **WiFi**: Onboard ESP32 802.11b/g/n

### Software
- **Framework**: ESPHome 2024.x (latest stable)
- **Integration**: Home Assistant (via native ESPHome API)
- **Components Used**:
  - `wifi` - WiFi connectivity and fallback AP
  - `api` - Home Assistant native API
  - `ota` - OTA firmware updates
  - `time` - NTP time synchronization (SNTP)
  - `sensor` - Temperature (Dallas), WiFi signal strength
  - `switch` - GPIO relay control
  - `climate` - Thermostat component with heating logic
  - `display` - Nextion UART display integration
  - `one_wire` - OneWire protocol for DS18B20
  - `uart` - Serial communication to Nextion
  - `number` - User configurable values (setpoint, hysteresis, offset)
  - `select` - Mode selection (winter/summer, home/away)

### External Services
- **OpenWeatherMap API**: Weather forecast data (API key required)
- **NTP Pool**: Time synchronization (us.pool.ntp.org, etc.)

---

## Deliverables & Success Definition

### Phase 1: Complete (Current)
- ✅ Code analysis and specification document
- ✅ Constitution defining principles and constraints
- ✅ Feature requirement mapping

### Phase 2: ESPHome Configuration (In Plan)
- [ ] Core gazebo_stove.yaml with GPIO, sensors, climate entity
- [ ] Nextion display integration (UART communication)
- [ ] Home Assistant API configuration
- [ ] Basic thermostat algorithm validation

### Phase 3: Feature Implementation (In Plan)
- [ ] Timer scheduling via Home Assistant automations
- [ ] Manual mode overrides with 15-minute timeout
- [ ] Home/Away mode logic
- [ ] Sensor malfunction detection and alarm
- [ ] Settings persistence (via Home Assistant database)
- [ ] Weather forecast integration

### Phase 4: Testing & Validation (In Plan)
- [ ] Hardware connectivity verification (all GPIO functional)
- [ ] Thermostat behavior under various temperature profiles
- [ ] Remote control latency and reliability
- [ ] WiFi resilience and fallback AP behavior
- [ ] OTA update success and state preservation
- [ ] Nextion display update timing and accuracy

### Phase 5: Migration & Cutover (In Plan)
- [ ] Parallel operation (Blynk + ESPHome coexist)
- [ ] User validation period
- [ ] Legacy code archival
- [ ] Production deployment

---

## Notes & Known Issues from Original Code

1. **Hardcoded Credentials**: Blynk auth token and WiFi credentials in blynkModule.cpp - will be migrated to Home Assistant secrets
2. **OpenWeather Config**: Forecast location hardcoded in library; needs parametrization for flexibility
3. **Nextion Communication**: EasyNex library used; ESPHome native Nextion support may differ - testing required
4. **Time Library Mixup**: Code imports both `<../include/time.h>` and `<Time.h>` - ESPHome uses SNTP directly
5. **DHT Sensor Comments**: Code comments mention DHT22/DHT11 humidity reading; actual hardware uses DS18B20 only - clarify actual sensor deployment
6. **Blynk Timer Dependency**: All timing uses BlynkTimer; will migrate to ESPHome's internal scheduler
7. **OTA Token**: Original uses ArduinoOTA with hardcoded upload port; ESPHome uses its own OTA mechanism

---

## Risks & Mitigation

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|-----------|
| Nextion UART communication differs in ESPHome | Display non-functional | Medium | Test UART commands early; have fallback to logging only |
| Home Assistant database not suitable for frequent updates | Settings don't persist | Low | Use Home Assistant's built-in entity state persistence |
| Weather API quota exceeded | Forecast missing | Low | Cache forecast data; graceful degradation to last known |
| Timer scheduling complexity | Missed heating windows | Medium | Implement simple YAML-based automations; test extensively |
| WiFi dropout causes relay to stick on | Safety issue | Low | Implement watchdog timer; relay defaults to OFF on boot |
| Manual mode timeout not enforced | Heating runs indefinitely | Low | Use Home Assistant automation with strict timing |

---

## Rollback & Contingency Plan

**If ESPHome conversion fails to meet requirements:**
1. Retain original Arduino code in `legacy/` git branch
2. Device can continue running Blynk firmware; ESPHome binary not pushed to production
3. Incremental rollback by reverting to last working firmware build
4. Both systems can run simultaneously during transition for A/B testing

**Validation checkpoints before marking as production-ready:**
- [ ] 48-hour continuous operation test with full feature set
- [ ] WiFi dropout recovery confirmed 3x
- [ ] Temperature control within ±2°C over full range
- [ ] All settings persist across 5 power cycles
- [ ] Remote control latency <2 seconds
- [ ] Nextion display updates correctly for all states

---

## Sign-Off & Version Control

- **Specification Version**: 1.0.0
- **Date**: 2025-11-10
- **Prepared By**: Code Analysis
- **Status**: Ready for Architecture Design & Planning Phase

**Next Step**: Proceed to detailed ESPHome configuration planning in PLAN.md
