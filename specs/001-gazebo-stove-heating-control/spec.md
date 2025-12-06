# Feature Specification: Gazebo Propane Stove Heating Control System

**Feature Branch**: `001-gazebo-stove-heating-control`  
**Created**: 2025-11-10  
**Status**: Analysis & Planning Phase  
**Input**: User description: "GazeboWx ESPHome Conversion - Heating Control System (Heating-Only)"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Automatic Temperature Control (Priority: P1)

**Description**: As a gazebo user, I want the heating system to automatically maintain the gazebo at my desired temperature, so I don't have to manually adjust the stove.

**Why this priority**: Core heating functionality; primary value proposition of the system.

**Independent Test**: Can be fully tested by setting a temperature setpoint in Home Assistant and observing relay activation/deactivation as ambient temperature crosses hysteresis thresholds. Requires only temperature sensor and relay functional.

**Acceptance Scenarios**:

1. **Given** Home mode is active, **When** temperature drops below setpoint, **Then** relay activates and heating starts within 5 seconds
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
2. **Given** Home mode is active and schedule is disabled, **When** temperature is below setpoint, **Then** relay activates (schedule disabled = respect current Home/Away mode)
3. **Given** Home mode is active and schedule is enabled, **When** time is outside scheduled window, **Then** relay stays off
4. **Given** Home mode and schedule is enabled, **When** current time is within scheduled window and temperature is below setpoint, **Then** relay activates
5. **Given** Schedule is disabled, **When** presence mode changes, **Then** system respects new mode (Away = relay off, Home = allow heating)
6. **Given** Timer schedule changes, **When** new begin/end times are set, **Then** settings persist across reboot (EEPROM equivalent in Home Assistant)

---

### User Story 3 - Manual Control Overrides (Priority: P1)

**Description**: As a gazebo user, I want to manually turn the heater on or off for immediate effect, overriding automatic controls.

**Why this priority**: Users need emergency control and ability to run heater on-demand.

**Independent Test**: Can be tested by setting manual run/stop switches in Home Assistant and verifying relay state changes immediately regardless of thermostat algorithm.

**Acceptance Scenarios**:

1. **Given** Automatic mode is active, **When** Manual Run is enabled, **Then** relay turns on immediately and stays on
2. **Given** Automatic mode is active, **When** Manual Stop is enabled, **Then** relay turns off immediately and stays off
3. **Given** Manual Run is active, **When** 15 minutes elapse, **Then** Manual Run times out and automatic mode resumes
4. **Given** Manual Run is active, **When** Manual Stop is enabled, **Then** relay turns off immediately and automatic mode resumes
5. **Given** Manual Run and Manual Stop are activated simultaneously, **When** both switches are ON, **Then** Force Off takes precedence, relay stays OFF, and Emergency Heat is blocked

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

1. **Given** New firmware is available and relay is OFF, **When** OTA update is initiated, **Then** device downloads and flashes new firmware within 2 minutes
2. **Given** Relay is ON (heating active), **When** OTA update is attempted, **Then** update is blocked with error message requiring relay OFF first
3. **Given** OTA update is in progress, **When** update completes, **Then** device reboots automatically and reports new firmware version
4. **Given** Settings and calibration data exist, **When** OTA update completes, **Then** all settings persist (no EEPROM loss)

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
3. **Given** Alarm is active, **When** sensor returns valid readings, **Then** alarm clears automatically after 1 good read and thermostat control resumes with last-known setpoint

---

### User Story 11 - Perceived Temperature (Humidity-Adjusted) (Priority: P3)

**Description**: As a comfort-conscious user, I want the system to account for humidity when determining "feels like" temperature.

**Why this priority**: Improves comfort in humid conditions; low priority but implemented in original.

**Independent Test**: Can be tested by setting humidity values and observing perceived temperature adjustment in thermostat logic.

**Acceptance Scenarios**:

1. **Given** Perceived temperature is enabled, **When** temperature is 25°C and humidity is 70%, **Then** perceived temperature increases by humidity factor (approx. 3°C per 10% above 40%)
2. **Given** Perceived temperature is enabled, **When** season is Winter, **Then** perceived temperature adjustment is disabled (Winter only uses actual temp)

---

### Edge Cases

- **Sensor malfunction**: Device reads are NaN → increment BadRead counter → at 10+ consecutive failures, deactivate relay and alarm → when sensor returns valid reading, alarm clears and thermostat control auto-resumes with last-known setpoint
- **WiFi dropout**: Continue thermostat operation with cached setpoint; create fallback AP if unable to reconnect for >5 minutes
- **Manual mode timeout**: After 15 minutes in Manual Run, return to automatic mode; timer persists across WiFi loss
- **Temperature display bounce**: Limit temperature change to ±1°C per reading interval; average with previous reading for stability
- **Rapid setpoint changes**: Accept setpoint changes immediately but apply only within rate-limited display updates
- **Home/Away mode edge case**: If transition occurs during heating cycle, relay state changes immediately
- **Timer schedule boundaries**: Ensure relay off state when current time transitions outside scheduled window
- **OTA during heating**: OTA update is blocked when relay is ON - user must turn off heating before update can proceed (safety requirement)
- **NTP time sync failure**: Operate with last-known time; schedule timers may not work until time is synced
- **Climate platform target sync**: [NEEDS CLARIFICATION] When desired temperature changes via thermostat card, ESPHome climate platform target must sync with number entity - how to ensure consistency?
- **Priority hierarchy conflicts**: Force Off takes precedence when both Manual Run and Manual Stop are activated simultaneously - relay stays OFF and Emergency Heat is blocked
- **Hysteresis calculation**: [NEEDS CLARIFICATION] ESPHome thermostat platform uses symmetric deadband (±H/2), but user expects asymmetric (±H). Should we use custom script or accept platform limitation?

---

## Requirements *(mandatory)*

### Functional Requirements

#### Thermostat Core
- **FR-001**: System MUST maintain temperature within ±(Hysteresis) degrees of setpoint while in Home + Auto mode
- **FR-002**: System MUST respect Home/Away mode - relay OFF when Away regardless of temperature
- **FR-003**: System MUST respect Timer schedule - relay OFF when outside scheduled window hours (only when schedule is enabled)
- **FR-003a**: System MUST respect last Home/Away mode state when schedule is disabled (if Away, relay stays off; if Home, allow heating)
- **FR-004**: System MUST support adjustable heating hysteresis (1-5°C range, default 2°C)
- **FR-005**: System MUST implement rate-limited temperature readings (max ±1°C change per update)
- **FR-006**: System MUST support heating-only logic (propane stove cannot cool, inactive in summer)

#### Manual Control
- **FR-007**: System MUST provide Manual Run override that activates relay for 15 minutes regardless of thermostat
- **FR-008**: System MUST provide Manual Stop override that deactivates relay regardless of thermostat
- **FR-009**: System MUST timeout Manual Run mode after exactly 15 minutes
- **FR-010**: System MUST allow Manual modes to be cancelled by user action or other mode selection
- **FR-011**: Manual Stop (Force Off) MUST have highest priority - when active, Emergency Heat is blocked and relay remains OFF regardless of Emergency Heat state
- **FR-011a**: If Manual Run and Manual Stop are activated simultaneously, Force Off takes precedence immediately (safety-first approach)

#### Sensor & Calibration
- **FR-012**: System MUST read DS18B20 temperature sensor and filter results (10-second averaging)
- **FR-013**: System MUST apply user-configurable temperature offset for sensor calibration (±adjustable range)
- **FR-014**: System MUST detect sensor malfunction (10+ consecutive bad reads) and alarm
- **FR-014a**: System MUST automatically resume thermostat control with last-known setpoint when sensor malfunction clears (after 1 valid reading)
- **FR-015**: System MUST NOT use perceived temperature (not applicable for heating-only stove)
- **FR-016**: System MUST NOT adjust temperature for humidity (heating-only operation)

#### Display & Feedback
- **FR-017**: System MUST update Nextion display with current temp, desired temp, mode, and relay status every 2 seconds
- **FR-018**: System MUST display weather forecast (current conditions and 5-day) on Nextion via OpenWeatherMap API
- **FR-019**: System MUST show alarm message on Nextion when sensor malfunction detected
- **FR-020**: System MUST display Home/Away mode and current time on Nextion
- **FR-021**: System MUST display timer status (on/off, begin/end times) on Nextion

#### Configuration & Persistence
- **FR-022**: System MUST persist configuration to equivalent of EEPROM: desired temp, hysteresis values, offsets, timer schedule
- **FR-023**: System MUST load saved settings from Home Assistant on startup
- **FR-024**: System MUST reload configuration if Home Assistant connection drops then reconnects
- **FR-025**: System MUST support batch configuration updates from Home Assistant

#### Connectivity & Remote Control
- **FR-026**: System MUST connect to WiFi and expose all state/control via Home Assistant native API
- **FR-027**: System MUST create fallback AP ("Gazebo-Wx Fallback") if unable to connect to configured SSID
- **FR-028**: System MUST support OTA firmware updates via ESPHome dashboard
- **FR-028a**: System MUST block OTA updates when relay is ON (heating active) - require relay OFF first for safety
- **FR-029**: System MUST maintain operation for ≥1 hour without WiFi (thermostat continues with cached setpoint)
- **FR-030**: System MUST auto-reconnect to WiFi when signal becomes available
- **FR-031**: System MUST expose all settings as Home Assistant climate entity + number/switch entities

#### Time & Scheduling
- **FR-032**: System MUST sync time from NTP server on startup and periodically refresh
- **FR-033**: System MUST support timer begin/end times (format: HH:MM 24-hour)
- **FR-034**: System MUST evaluate timer schedule every minute and update relay control accordingly
- **FR-035**: System MUST operate current time display on Nextion (HH:MM:SS format)

#### Hardware Interface
- **FR-036**: System MUST control relay on GPIO12 (active HIGH for heating)
- **FR-037**: System MUST read OneWire DS18B20 sensor on GPIO4
- **FR-038**: System MUST communicate with Nextion display on UART2 (GPIO16 RX, GPIO17 TX) at 9600 baud
- **FR-039**: System MUST support WiFi module (onboard ESP32 WiFi)
- **FR-040**: System MUST support OTA via WiFi (ESPHome OTA protocol)

### Functional Requirement Clarifications Needed

- **FR-041**: [NEEDS CLARIFICATION] Original mentions both DHT22 and DHT11 sensor in comments; which sensor is actually deployed? (Current implementation uses DS18B20 only, but comments reference DHT humidity)
- **FR-042**: [NEEDS CLARIFICATION] OpenWeatherMap integration - what location/coordinates to fetch forecast for? (Currently hardcoded in library settings)
- **FR-043**: [NEEDS CLARIFICATION] IFTTT integration mentioned for Home/Away via location triggers - replace with Home Assistant presence detection or maintain IFTTT hooks?

### Key Entities *(include if feature involves data)*

#### Climate Entity (Thermostat)
- **name**: "Gazebo Thermostat" / "Propane Stove Thermostat"
- **current_temperature**: float (from DS18B20)
- **target_temperature**: float (user setpoint, 5-30°C typical range)
- **min_temp**: 5, **max_temp**: 35 (configurable bounds)
- **hvac_mode**: "heat" | "off" (Heating when Home, Off when Away)
- **current_humidity**: float (not used for heating-only stove)
- **aux_heat**: boolean (Manual Run mode indicator)
- **preset_modes**: ["auto", "manual_run", "manual_stop"] or ["home", "away"]

#### Sensor Entity (Temperature)
- **name**: "Gazebo DS18B20" / "Gazebo Temperature"
- **unit_of_measurement**: "°C"
- **device_class**: "temperature"
- **state**: float (20.5 = 20.5°C)
- **filters**: [throttle_average: 10s]

#### Switch Entities
- **name**: "Gazebo Relay" / "Propane Stove Relay"
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

## Success Criteria *(mandatory)*

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
- **SC-010**: Climate platform target sync: When desired temperature changes, ESPHome climate platform target syncs within 100ms

### Feature Parity Verification

- **SC-011**: All 11 Blynk virtual pins (V0-V31) mapped to Home Assistant entities with equivalent functionality
- **SC-012**: All EEPROM settings migrated to Home Assistant number/select entities with persistence
- **SC-013**: Timer scheduling equivalent to original, with visual feedback on Nextion
- **SC-014**: Manual mode and Home/Away mode all implemented and accessible
- **SC-015**: Weather display maintained (OpenWeatherMap → Nextion, no API change required)

### Migration Validation

- **SC-016**: Original Arduino code retained in version control; rollback path documented
- **SC-017**: Blynk and ESPHome can coexist during transition (device supports both simultaneously)
- **SC-018**: All tests pass on target hardware (esp32doit-devkit-v1) before production deployment

---

## Clarifications

### Session 2025-01-27

- **Q1**: Climate platform target sync - When desired temperature changes via thermostat card, ESPHome climate platform target must sync with number entity. How to ensure consistency?
  - **A1**: Added lambda action in `desired_temp` set_action to sync climate platform target temperature. Climate platform receives updates from Home Assistant API automatically, but explicit sync ensures consistency when `desired_temp` is changed directly.

- **Q2**: Priority hierarchy conflicts - What happens when Force Off and Emergency Heat are both activated simultaneously?
  - **A2**: Force Off (Manual Stop) has highest priority. When Force Off is active, Emergency Heat is blocked and switch behaves as momentary only (does not activate relay). This ensures safety lockout cannot be overridden.

- **Q3**: Hysteresis calculation - ESPHome thermostat platform uses symmetric deadband (±H/2), but user expects asymmetric (±H). Should we use custom script or accept platform limitation?
  - **A3**: Currently using ESPHome climate platform for UI compatibility. Custom thermostat script with asymmetric hysteresis exists but is disabled to avoid conflicts. Priority script (`manage_heating`) handles edge cases (current > desired = turn off immediately). User-configurable hysteresis entity exists but platform uses its own internal calculation.

- **Q4**: Schedule enable/disable default behavior - When heating schedule toggle is disabled, what should the system's default behavior be?
  - **A4**: Respect last Home/Away mode state. When schedule is disabled, system maintains current presence mode (if Away, stay Away; if Home, allow heating). This preserves user intent and avoids unexpected heating when schedule is disabled.

- **Q5**: Concurrent manual operations - If user activates Manual Run (Emergency Heat) and Manual Stop (Force Off) simultaneously, what should happen?
  - **A5**: Force Off takes precedence immediately, Emergency Heat is blocked. Safety-first approach ensures safety lockout cannot be overridden, even if Emergency Heat was activated first. Relay remains OFF.

- **Q6**: Sensor failure recovery behavior - After sensor malfunction alarm clears (sensor returns valid readings), what should the system do?
  - **A6**: Automatically resume thermostat control with last-known setpoint. Auto-recovery restores normal operation without manual intervention, assuming sensor recovery indicates issue is resolved.

- **Q7**: OTA update during active heating - If OTA firmware update is initiated while relay is ON (heating active), what should happen?
  - **A7**: Block OTA update, require relay OFF first. Safety-first approach prevents interrupting active heating and avoids potential safety issues from mid-update failures. User must manually turn off heating before OTA update can proceed.

- **Q8**: Winter mode vs Home mode terminology - Are "Winter mode" and "Home mode" the same concept?
  - **A8**: No winter mode. Remove "Winter mode" terminology entirely, use only "Home/Away mode" throughout specification. Heating-only system uses Home mode (heating allowed) and Away mode (heating disabled).

