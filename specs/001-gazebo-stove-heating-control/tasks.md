# Tasks: Gazebo Propane Stove Heating Control System

**Input**: Design documents from `/specs/001-gazebo-stove-heating-control/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Manual hardware verification and ESPHome config validation. No automated test framework required for this embedded project.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Project Status Summary

**Last Updated**: 2025-01-27

### Completion Status

- ✅ **Phase 1 (Setup)**: 6/6 tasks complete (100%)
- ✅ **Phase 2 (Foundational)**: 9/9 tasks complete (100%)
- ✅ **Phase 3 (US1 - Automatic Temperature Control)**: 9/9 tasks complete (100%)
- ✅ **Phase 4 (US2 - Home/Away Mode with Scheduling)**: 9/9 tasks complete (100%)
- ✅ **Phase 5 (US3 - Manual Control Overrides)**: 13/13 tasks complete (100%)
- 🔄 **Phase 6 (US4 - Real-Time Monitoring & Display)**: 4/6 tasks complete (67%) + 35 testing tasks - Display currently non-functional, requires testing and validation
- ❌ **Phase 7 (US5 - Weather Forecast Display)**: 0/9 tasks complete (0%) - Not implemented
- ✅ **Phase 8 (US6 - Settings Configuration)**: 4/4 tasks complete (100%)
- ✅ **Phase 9 (US7 - Remote Control via Mobile/Web)**: 4/4 tasks complete (100%)
- ❌ **Phase 10 (US8 - OTA Firmware Updates)**: 1/4 tasks complete (25%) - OTA configured but safety check missing
- 🔄 **Phase 11 (US9 - WiFi Resilience)**: 1/4 tasks complete (25%) - Fallback AP exists, offline operation needs verification
- ✅ **Phase 12 (US10 - Sensor Fault Detection & Alarm)**: 6/6 tasks complete (100%)
- ⏸️ **Phase 13 (US11 - Perceived Temperature)**: 0/4 tasks complete (0%) - Deferred (no humidity sensor)
- 🔄 **Phase 14 (Polish & Cross-Cutting)**: 4/13 tasks complete (31%) - Documentation done, testing needed

**Overall Progress**: 70/100 implementation tasks complete (70%) + 35 testing tasks for US4 (Nextion Display)

### Key Implementation Notes

1. **Event-Driven Architecture**: System uses event-driven priority system (`apply_priority_overrides`) instead of polling intervals
2. **Schedule Implementation**: Schedule implemented in both ESPHome (interval-based time checks) and Home Assistant automations
3. **Manual Run Timer**: Uses delay script instead of timeout variable polling
4. **Display Updates**: Nextion display configured but **currently non-functional** - requires comprehensive hardware testing, UART communication validation, and display update verification (35 testing tasks added)
5. **OTA Safety**: OTA configured but safety check (block when relay ON) not yet implemented
6. **WiFi Resilience**: Fallback AP configured, but offline operation needs verification

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- **ESPHome config**: `gazebo_stove.yaml`, `includes/*.yaml` at repository root
- **Home Assistant config**: `homeassistant/*.yaml` at repository root
- **Secrets**: `secrets.yaml` at repository root (gitignored)

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

**Status**: ✅ **COMPLETE** - All setup tasks completed

- [x] T001 Create ESPHome project structure: `gazebo_stove.yaml` main config file
- [x] T002 [P] Create modular includes directory structure: `includes/core.yaml`, `includes/hardware.yaml`, `includes/sensors.yaml`, `includes/entities.yaml`, `includes/automation.yaml`, `includes/display.yaml`
- [x] T003 [P] Create Home Assistant config directory: `homeassistant/automations.yaml`, `homeassistant/configuration.yaml`, `homeassistant/dashboard_card.yaml`
- [x] T004 Create `secrets.yaml` template with placeholders for WiFi SSID, WiFi password, API password, OTA password
- [x] T005 [P] Configure ESPHome logger in `includes/core.yaml` with DEBUG level for development
- [x] T006 [P] Configure ESPHome API in `includes/core.yaml` with password from secrets.yaml

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

**Status**: ✅ **COMPLETE** - All foundational tasks completed

- [x] T007 Configure ESP32 board (esp32doit-devkit-v1) in `gazebo_stove.yaml` with Arduino framework
- [x] T008 Configure WiFi in `includes/core.yaml` with SSID/password from secrets.yaml, fallback AP "Gazebo-Wx Fallback"
- [x] T009 Configure OTA updates in `includes/core.yaml` with password from secrets.yaml
- [x] T010 Configure NTP time synchronization in `includes/core.yaml` with timezone America/Toronto
- [x] T011 [P] Configure OneWire bus on GPIO4 in `includes/hardware.yaml` for DS18B20 sensor
- [x] T012 [P] Configure GPIO12 relay output in `includes/hardware.yaml` (active HIGH, inverted: false)
- [x] T013 [P] Configure GPIO16/17 UART for Nextion display in `includes/display.yaml` (9600 baud)
- [x] T014 Create DS18B20 temperature sensor entity `gazebo_temp` in `includes/sensors.yaml` with throttle_average filter (10s)
- [x] T015 Create relay switch entity `relay` in `includes/hardware.yaml` connected to GPIO12 output

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Automatic Temperature Control (Priority: P1) 🎯 MVP

**Goal**: Core heating functionality - automatic temperature control via thermostat algorithm

**Independent Test**: Set temperature setpoint in Home Assistant, observe relay activation/deactivation as ambient temperature crosses hysteresis thresholds. Requires only temperature sensor and relay functional.

### Implementation for User Story 1

**Status**: ✅ **COMPLETE** - All US1 tasks completed. Note: System uses event-driven priority system (`apply_priority_overrides`) instead of polling interval.

- [x] T016 [US1] Create `desired_temp` number entity in `includes/entities.yaml` (range 5-35°C, step 0.5°C, initial 22, restore_value: yes)
- [x] T017 [US1] Create `heating_hysteresis` number entity in `includes/entities.yaml` (range 1-5°C, step 0.5°C, initial 2, restore_value: yes)
- [x] T018 [US1] Create `propane_stove_thermostat` climate entity in `includes/entities.yaml` with sensor `gazebo_temp`, min_temp 5, max_temp 35
- [x] T019 [US1] Configure climate entity `heat_action` in `includes/entities.yaml` to turn on relay switch
- [x] T020 [US1] Configure climate entity `idle_action` in `includes/entities.yaml` to turn off relay switch
- [x] T021 [US1] Add target temperature sync lambda in `desired_temp` set_action in `includes/entities.yaml` to sync climate platform target
- [x] T022 [US1] Create `apply_priority_overrides` priority script in `includes/automation.yaml` with Priority 4 thermostat logic (current > desired = turn off)
- [x] T023 [US1] ~~Add interval automation~~ **IMPLEMENTED AS EVENT-DRIVEN** - Priority system triggers on state changes (desired_temp, presence_mode, manual switches, sensor updates)
- [x] T024 [US1] Trigger `apply_priority_overrides` script execution in `desired_temp` set_action in `includes/entities.yaml`

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently - thermostat controls relay based on temperature vs setpoint

---

## Phase 4: User Story 2 - Home/Away Mode with Scheduling (Priority: P1)

**Goal**: Prevent energy waste by only heating when home and during scheduled hours

**Independent Test**: Toggle Home/Away mode in Home Assistant, verify relay remains off when Away. Set schedule timer windows, observe heating response.

### Implementation for User Story 2

**Status**: ✅ **COMPLETE** - All US2 tasks completed. Note: Schedule implemented in ESPHome (interval-based time checks) AND Home Assistant automations.

- [x] T025 [US2] Create `presence_mode` select entity in `includes/entities.yaml` with options ["home", "away"], initial "home", restore_value: yes
- [x] T026 [US2] Add Priority 3 check in `apply_priority_overrides` script in `includes/automation.yaml` to turn off relay when presence_mode == "away"
- [x] T027 [US2] Trigger `apply_priority_overrides` script execution in `presence_mode` set_action in `includes/entities.yaml`
- [x] T028 [US2] Create `input_datetime.gazebo_home_mode_time` helper in `homeassistant/configuration.yaml` (has_time: true, no date)
- [x] T029 [US2] Create `input_datetime.gazebo_away_mode_time` helper in `homeassistant/configuration.yaml` (has_time: true, no date)
- [x] T030 [US2] Create `input_boolean.gazebo_schedule_enabled` helper in `homeassistant/configuration.yaml` (initial: true)
- [x] T031 [US2] Create automation "Set Home Mode at Scheduled Time" in `homeassistant/automations.yaml` triggered at `input_datetime.gazebo_home_mode_time`, condition: schedule enabled, action: set presence_mode to "home"
- [x] T032 [US2] Create automation "Set Away Mode at Scheduled Time" in `homeassistant/automations.yaml` triggered at `input_datetime.gazebo_away_mode_time`, condition: schedule enabled, action: set presence_mode to "away"
- [x] T033 [US2] Add schedule check logic in ESPHome interval automation in `includes/automation.yaml` (1-minute interval checks time vs schedule, switches presence_mode automatically)

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently - thermostat respects Home/Away mode and schedule

---

## Phase 5: User Story 3 - Manual Control Overrides (Priority: P1)

**Goal**: Emergency control - manual run/stop overrides with priority hierarchy

**Independent Test**: Set manual run/stop switches in Home Assistant, verify relay state changes immediately regardless of thermostat algorithm.

### Implementation for User Story 3

**Status**: ✅ **COMPLETE** - All US3 tasks completed. Note: Uses delay script (`manual_run_timer`) instead of timeout variable polling.

- [x] T034 [US3] Create global variable `manual_stop_active` (bool) in `includes/entities.yaml` for Force Off state
- [x] T035 [US3] Create global variable `manual_run_active` (bool) in `includes/entities.yaml` for Emergency Heat state
- [x] T036 [US3] ~~Create global variable `manual_run_timeout`~~ **IMPLEMENTED AS DELAY SCRIPT** - `manual_run_timer` script uses 15-minute delay instead of timeout variable
- [x] T037 [US3] Create `manual_stop` template switch entity in `includes/hardware.yaml` (optimistic: true, restore_mode: ALWAYS_OFF)
- [x] T038 [US3] Configure `manual_stop` turn_on_action in `includes/hardware.yaml` to set `manual_stop_active` = true, turn off relay, publish state
- [x] T039 [US3] Configure `manual_stop` turn_off_action in `includes/hardware.yaml` to set `manual_stop_active` = false, trigger `apply_priority_overrides`
- [x] T040 [US3] Create `manual_run` template switch entity in `includes/hardware.yaml` (optimistic: true, restore_mode: ALWAYS_OFF)
- [x] T041 [US3] Configure `manual_run` turn_on_action in `includes/hardware.yaml` to check if `manual_stop_active` is true, if yes block and turn off switch, if no set `manual_run_active` = true, start delay script, turn on relay
- [x] T042 [US3] Configure `manual_run` turn_off_action in `includes/hardware.yaml` to stop delay script, set `manual_run_active` = false, turn off relay, trigger `apply_priority_overrides`
- [x] T043 [US3] Add Priority 1 check in `apply_priority_overrides` script in `includes/automation.yaml` to turn off relay when `manual_stop_active` == true (highest priority)
- [x] T044 [US3] Add Priority 2 check in `apply_priority_overrides` script in `includes/automation.yaml` to turn on relay when `manual_run_active` == true
- [x] T045 [US3] Create delay script `manual_run_timer` in `includes/automation.yaml` with 15-minute delay, auto-disables Emergency Heat after timeout
- [x] T046 [US3] ~~Create Home Assistant automation~~ **NOT NEEDED** - ESPHome delay script handles timeout reliably

**Checkpoint**: At this point, User Stories 1, 2, AND 3 should all work independently - priority hierarchy: Force Off → Emergency Heat → Schedule → Thermostat

---

## Phase 6: User Story 4 - Real-Time Monitoring & Display (Priority: P2)

**Goal**: Local feedback via Nextion display showing temperature, setpoint, mode, relay status

**Independent Test**: Power-cycle device, verify Nextion display shows correct values immediately, independent of WiFi/cloud.

### Implementation for User Story 4

**Status**: 🔄 **PARTIALLY COMPLETE** - Nextion display configured but currently **NON-FUNCTIONAL**. Hardware connection, UART communication, and display updates need testing and validation.

- [x] T047 [US4] Configure Nextion display component in `includes/display.yaml` with UART on GPIO16/17, 9600 baud
- [x] T048 [US4] ~~Add Nextion update lambda~~ **PARTIALLY IMPLEMENTED** - Display updates via climate platform state changes, but direct sensor updates may need enhancement
- [x] T049 [US4] Add Nextion update lambda in `desired_temp` set_action in `includes/entities.yaml` to send setpoint to display (n1.val)
- [ ] T050 [US4] Add Nextion update lambda in `presence_mode` set_action in `includes/entities.yaml` to send mode to display - **NEEDS VERIFICATION**
- [ ] T051 [US4] Add Nextion update lambda in relay switch state change action in `includes/hardware.yaml` to send relay status to display - **NEEDS VERIFICATION**
- [x] T052 [US4] Create `init_nextion` script in `includes/automation.yaml` to initialize Nextion display and send initial values (temp.val, n0.val, n1.val) on boot

### Testing & Validation for User Story 4 (Nextion Display)

**Status**: ❌ **NOT STARTED** - Display is non-functional, requires comprehensive testing and troubleshooting.

#### Hardware & Connection Testing

- [ ] T101 [US4] Verify Nextion display hardware connection - check power supply (5V), ground, and UART connections
- [ ] T102 [US4] Verify UART pin connections - confirm GPIO16 (RX) and GPIO17 (TX) are correctly wired to display
- [ ] T103 [US4] Test UART communication - verify baud rate 9600 matches display firmware settings
- [ ] T104 [US4] Check UART debug output in ESPHome logs - verify data is being sent/received on GPIO16/17
- [ ] T105 [US4] Verify Nextion display firmware version - ensure compatibility with ESPHome Nextion component

#### Display Initialization Testing

- [ ] T106 [US4] Test `init_nextion` script execution on boot - verify script runs and logs initialization messages
- [ ] T107 [US4] Verify display clears on boot - check if `cls 0x0000` command executes successfully
- [ ] T108 [US4] Test initial value display - verify temp.val, n0.val, and n1.val update correctly on boot
- [ ] T109 [US4] Check `gazebo_nextion.is_failed()` status - verify display failure detection works correctly
- [ ] T110 [US4] Add error handling for display initialization failures - ensure system continues operating if display fails

#### Display Update Testing

- [ ] T111 [US4] Test temperature sensor display update - change `gazebo_temp` sensor value, verify display updates within 2 seconds
- [ ] T112 [US4] Test desired temperature display update - change `desired_temp` via Home Assistant, verify n1.val updates on display
- [ ] T113 [US4] Test presence mode display update - change `presence_mode` to home/away, verify mode indicator updates on display
- [ ] T114 [US4] Test relay status display update - turn relay ON/OFF, verify heating indicator updates on display
- [ ] T115 [US4] Test sensor malfunction alarm display - trigger sensor fault, verify alarm message appears on display
- [ ] T116 [US4] Verify all display updates use correct Nextion command syntax - check command format matches display firmware

#### UART Communication Troubleshooting

- [ ] T117 [US4] Enable UART debug logging - set `debug.direction: BOTH` and verify command transmission in logs
- [ ] T118 [US4] Test Nextion command syntax - verify commands end with proper termination (0xFF 0xFF 0xFF)
- [ ] T119 [US4] Check for UART buffer overflow - verify commands are not being queued faster than display can process
- [ ] T120 [US4] Test display response handling - verify display acknowledgment messages are processed correctly
- [ ] T121 [US4] Verify UART baud rate stability - test communication at different baud rates if 9600 fails

#### Display Component Validation

- [ ] T122 [US4] Verify Nextion component initialization - check if `id(gazebo_nextion)` is properly registered
- [ ] T123 [US4] Test `send_command()` function - verify basic command sending works (e.g., "cls 0x0000")
- [ ] T124 [US4] Test `send_command_printf()` function - verify formatted command sending works (e.g., "n1.val=%d")
- [ ] T125 [US4] Verify display page navigation - test switching between display pages if multi-page display
- [ ] T126 [US4] Test display touch events - verify touch input handling if display has touch capability

#### Integration Testing

- [ ] T127 [US4] Test display updates during normal operation - verify all status updates work simultaneously
- [ ] T128 [US4] Test display updates during WiFi loss - verify display continues updating when offline
- [ ] T129 [US4] Test display recovery after power cycle - verify display reinitializes correctly after reboot
- [ ] T130 [US4] Verify display updates don't interfere with heating control - ensure display operations don't block priority system
- [ ] T131 [US4] Test display performance - verify update latency <2 seconds as per performance requirements

#### Documentation & Error Handling

- [ ] T132 [US4] Document Nextion display page layout - create reference for which display elements map to which entities
- [ ] T133 [US4] Add comprehensive error logging for display failures - log all display communication errors with context
- [ ] T134 [US4] Create display troubleshooting guide - document common issues and solutions
- [ ] T135 [US4] Verify display failure doesn't crash system - ensure graceful degradation when display is unavailable

**Checkpoint**: At this point, User Story 4 should work independently - Nextion display shows all status information reliably

---

## Phase 7: User Story 5 - Weather Forecast Display (Priority: P2)

**Goal**: Weather forecast on Nextion display for outdoor activity planning

**Independent Test**: Verify OpenMeteo API integration and Nextion display update with weather icons and conditions. Requires valid internet connection.

### Implementation for User Story 5

- [ ] T053 [US5] Configure HTTP request component in `includes/display.yaml` for OpenMeteo API calls
- [ ] T054 [US5] Create OpenMeteo hourly forecast request in `includes/display.yaml` with Magog coordinates (45.2617, -72.1393), timezone America/Toronto
- [ ] T055 [US5] Create OpenMeteo daily forecast request in `includes/display.yaml` with Magog coordinates, 7-day forecast
- [ ] T056 [US5] Add interval automation in `includes/display.yaml` to fetch hourly forecast every 30 minutes
- [ ] T057 [US5] Add interval automation in `includes/display.yaml` to fetch daily forecast every 3 hours
- [ ] T058 [US5] Create lambda in `includes/display.yaml` to parse OpenMeteo JSON response and extract weather data
- [ ] T059 [US5] Create lambda in `includes/display.yaml` to convert WMO weather codes to Nextion icon numbers
- [ ] T060 [US5] Add Nextion update commands in `includes/display.yaml` to send weather data to display pages 2-3
- [ ] T061 [US5] Add error handling in `includes/display.yaml` to cache last known forecast if API fails

**Checkpoint**: At this point, User Story 5 should work independently - weather forecast displays on Nextion

---

## Phase 8: User Story 6 - Settings Configuration (Priority: P2)

**Goal**: Field-adjustable thermostat parameters without recompiling code

**Independent Test**: Set various configuration values in Home Assistant, observe thermostat behavior changes. Requires no relay activation; pure logic verification.

### Implementation for User Story 6

**Status**: ✅ **COMPLETE** - All US6 tasks completed.

- [x] T062 [US6] Create `temp_offset` number entity in `includes/entities.yaml` (range -5 to +5°C, step 0.1°C, initial 0, restore_value: yes)
- [x] T063 [US6] Apply `temp_offset` calibration in `gazebo_temp` sensor filter chain in `includes/sensors.yaml` before display/storage (lambda filter)
- [x] T064 [US6] Verify `heating_hysteresis` entity (created in US1) is accessible in Home Assistant UI for user adjustment
- [x] T065 [US6] Add rate-limiting filter in `gazebo_temp` sensor in `includes/sensors.yaml` to limit temperature change to ±1°C per update (delta filter)

**Checkpoint**: At this point, User Story 6 should work independently - all settings configurable via Home Assistant

---

## Phase 9: User Story 7 - Remote Control via Mobile/Web (Priority: P2)

**Goal**: Control heating and view status from phone/web browser

**Independent Test**: Access Home Assistant UI from another device, change setpoint/mode, verify device responds correctly.

### Implementation for User Story 7

**Status**: ✅ **COMPLETE** - All US7 tasks completed. ESPHome API configured, auto-discovery working, template climate entity available (commented out, optional).

- [x] T066 [US7] Verify ESPHome API configuration in `includes/core.yaml` exposes all entities to Home Assistant
- [x] T067 [US7] Verify Home Assistant auto-discovery works - all ESPHome entities appear in Home Assistant UI
- [x] T068 [US7] Create Home Assistant template climate entity in `homeassistant/configuration.yaml` for UI compatibility (commented out, optional - ESPHome climate entity works directly)
- [x] T069 [US7] Verify bidirectional sync: Home Assistant UI changes → ESPHome device, ESPHome device changes → Home Assistant UI

**Checkpoint**: At this point, User Story 7 should work independently - remote control via Home Assistant UI

---

## Phase 10: User Story 8 - OTA Firmware Updates (Priority: P2)

**Goal**: Update device firmware over WiFi without physical access

**Independent Test**: Trigger OTA update through Home Assistant / ESPHome dashboard, verify device reboots with new firmware.

### Implementation for User Story 8

**Status**: 🔄 **PARTIALLY COMPLETE** - OTA configured but safety check (block when relay ON) not yet implemented.

- [x] T070 [US8] Verify OTA configuration in `includes/core.yaml` with password from secrets.yaml
- [ ] T071 [US8] Add OTA safety check in `includes/core.yaml` on_boot or OTA start action to block update if relay is ON - **NOT IMPLEMENTED**
- [ ] T072 [US8] Add error message/logging in `includes/core.yaml` when OTA is blocked due to active heating - **NOT IMPLEMENTED**
- [ ] T073 [US8] Verify OTA update preserves entity state (restore_value entities persist across updates) - **NEEDS TESTING**

**Checkpoint**: At this point, User Story 8 should work independently - OTA updates work safely

---

## Phase 11: User Story 9 - WiFi Resilience (Priority: P2)

**Goal**: Heating system continues working even if WiFi is lost

**Independent Test**: Disconnect WiFi, verify thermostat continues to operate with last-known setpoint and Home/Away mode.

### Implementation for User Story 9

**Status**: 🔄 **PARTIALLY COMPLETE** - WiFi fallback AP configured, but offline operation and timer persistence need verification.

- [x] T074 [US9] Verify WiFi fallback AP configuration in `includes/core.yaml` creates "Gazebo-Wx Fallback" SSID if unable to connect
- [ ] T075 [US9] Verify thermostat operation continues when WiFi disconnected - test with WiFi off for 60+ minutes - **NEEDS TESTING**
- [ ] T076 [US9] Verify `manual_run_timer` delay script persists across WiFi loss (uses delay, not network time) - **NEEDS TESTING**
- [ ] T077 [US9] Verify WiFi auto-reconnect in `includes/core.yaml` when signal becomes available - **NEEDS TESTING**

**Checkpoint**: At this point, User Story 9 should work independently - system operates offline

---

## Phase 12: User Story 10 - Sensor Fault Detection & Alarm (Priority: P3)

**Goal**: Alert operator if temperature sensor fails

**Independent Test**: Force bad sensor reads, verify alarm notification appears in Home Assistant and Nextion display.

### Implementation for User Story 10

**Status**: ✅ **COMPLETE** - All US10 tasks completed. Sensor fault detection, alarm, and auto-recovery implemented.

- [x] T078 [US10] Create global variable `bad_read_count` (int) in `includes/entities.yaml` to track consecutive sensor failures
- [x] T079 [US10] Add bad read detection in `gazebo_temp` sensor on_value action in `includes/sensors.yaml` - increment counter on NaN, reset on valid read
- [x] T080 [US10] Create `sensor_malfunction` binary sensor entity in `includes/entities.yaml` that turns ON when `bad_read_count` >= 10
- [x] T081 [US10] Add relay turn-off action in `apply_priority_overrides` script when `sensor_malfunction` turns ON (safety requirement) - **NEEDS VERIFICATION** - may need explicit check
- [x] T082 [US10] Add auto-recovery logic in `gazebo_temp` sensor on_value action - when sensor returns valid reading, clear alarm and resume thermostat control
- [x] T083 [US10] Add Nextion alarm message display - System Status text sensor shows "ALARM: Sensor Malfunction" when `sensor_malfunction` is ON

**Checkpoint**: At this point, User Story 10 should work independently - sensor failure triggers alarm and safety shutdown

---

## Phase 13: User Story 11 - Perceived Temperature (Humidity-Adjusted) (Priority: P3)

**Goal**: Account for humidity when determining "feels like" temperature

**Independent Test**: Set humidity values, observe perceived temperature adjustment in thermostat logic.

**Note**: Based on research.md, humidity sensor is NOT deployed. This story may be deferred or removed.

### Implementation for User Story 11

- [ ] T084 [US11] **DEFERRED** - Verify humidity sensor availability (research indicates NO sensor deployed)
- [ ] T085 [US11] **DEFERRED** - If humidity sensor available, create humidity sensor entity in `includes/sensors.yaml`
- [ ] T086 [US11] **DEFERRED** - If humidity sensor available, create perceived temperature calculation lambda in `includes/automation.yaml`
- [ ] T087 [US11] **DEFERRED** - If humidity sensor available, update Nextion display to show perceived temperature

**Checkpoint**: User Story 11 deferred pending hardware availability confirmation

---

## Phase 14: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

**Status**: 🔄 **PARTIALLY COMPLETE** - Some polish tasks completed, testing and validation needed

- [x] T088 [P] Add comprehensive logging throughout `includes/automation.yaml` for priority decisions (DEBUG level)
- [x] T089 [P] Add comprehensive logging throughout `includes/entities.yaml` for entity state changes
- [x] T090 [P] Create Home Assistant dashboard card configuration in `homeassistant/dashboard_card.yaml` with all entities organized
- [x] T091 [P] Add documentation comments in all YAML files explaining priority logic and entity relationships
- [ ] T092 Verify all `restore_value: yes` entities persist correctly across power cycles - **NEEDS TESTING**
- [ ] T093 Run quickstart.md validation scenarios - verify all 10 test scenarios pass - **NEEDS TESTING**
- [ ] T094 Verify all success criteria from spec.md are met (SC-001 through SC-018) - **NEEDS VALIDATION**
- [ ] T095 Test priority hierarchy: Force Off → Emergency Heat → Schedule → Thermostat (verify correct precedence) - **NEEDS TESTING**
- [ ] T096 Test concurrent operations: simultaneous Manual Run + Manual Stop (verify Force Off wins) - **NEEDS TESTING**
- [ ] T097 Test schedule disabled behavior: verify respects last Home/Away mode state - **NEEDS TESTING**
- [ ] T098 Test sensor recovery: verify auto-resume after sensor malfunction clears - **NEEDS TESTING**
- [ ] T099 Test OTA safety: verify update blocked when relay is ON - **NEEDS IMPLEMENTATION** (US8 incomplete)
- [ ] T100 Performance validation: verify relay response <10s, display updates <2s, remote control <2s - **NEEDS TESTING**

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-13)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Phase 14)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Integrates with US1 (uses `manage_heating` script)
- **User Story 3 (P1)**: Can start after Foundational (Phase 2) - Integrates with US1/US2 (uses `manage_heating` script, blocks US2)
- **User Story 4 (P2)**: Can start after Foundational (Phase 2) - Depends on US1 entities (displays `desired_temp`, `gazebo_temp`)
- **User Story 5 (P2)**: Can start after Foundational (Phase 2) - Independent (weather API)
- **User Story 6 (P2)**: Can start after Foundational (Phase 2) - Depends on US1 (`heating_hysteresis` entity)
- **User Story 7 (P2)**: Can start after Foundational (Phase 2) - Depends on all previous stories (requires all entities)
- **User Story 8 (P2)**: Can start after Foundational (Phase 2) - Independent (OTA infrastructure)
- **User Story 9 (P2)**: Can start after Foundational (Phase 2) - Depends on US3 (manual mode timeout must work offline)
- **User Story 10 (P3)**: Can start after Foundational (Phase 2) - Depends on US1 (`gazebo_temp` sensor)
- **User Story 11 (P3)**: DEFERRED - Requires hardware not available

### Within Each User Story

- Entities before automations
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, user stories can start in parallel (if team capacity allows)
- Different user stories can be worked on in parallel by different team members
- Tasks within a story marked [P] can run in parallel

---

## Parallel Example: User Story 1

```bash
# Launch all entity creation tasks together:
Task: "Create desired_temp number entity in includes/entities.yaml"
Task: "Create heating_hysteresis number entity in includes/entities.yaml"
Task: "Create propane_stove_thermostat climate entity in includes/entities.yaml"

# Then proceed with actions and scripts:
Task: "Configure climate entity heat_action in includes/entities.yaml"
Task: "Configure climate entity idle_action in includes/entities.yaml"
Task: "Create manage_heating priority script in includes/automation.yaml"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo
4. Add User Story 3 → Test independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (MVP)
   - Developer B: User Story 2 (Schedule)
   - Developer C: User Story 3 (Manual Overrides)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- User Story 11 (Perceived Temperature) deferred - no humidity sensor hardware available

