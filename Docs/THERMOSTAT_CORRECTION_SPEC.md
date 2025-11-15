# GazeboStove Thermostat Control System - Correction Specification

**Document Version**: 1.0
**Date**: 2025-11-14
**Status**: Specification for Correction
**Project Phase**: Phase 3-4 Review & Correction

---

## Executive Summary

This document provides a comprehensive analysis of the GazeboStove thermostat control system, identifying deviations from expected functionality, documenting strengths and weaknesses, and providing detailed sequential correction steps. The analysis reveals **three critical issues** and **one missing feature** that must be addressed to achieve proper operation.

**Critical Findings:**
1. **Priority Hierarchy Inversion** - Force Off (Manual Stop) must be highest priority, not third
2. **Missing Schedule Feature** - Home Mode Time / Away Mode Time scheduling not implemented
3. **Incorrect Hysteresis Logic** - Symmetric deadband vs. expected asymmetric behavior
4. **Missing Force Off Override** - Emergency Heat should be momentary when Force Off is active

**Impact**: Current system does not meet safety requirements (Force Off can be overridden by Away Mode) and lacks critical energy-saving automation (scheduling).

---

## Table of Contents

1. [Current State Analysis](#1-current-state-analysis)
2. [Expected Functionality Specification](#2-expected-functionality-specification)
3. [Gap Analysis - What's Broken](#3-gap-analysis---whats-broken)
4. [Strengths Assessment](#4-strengths-assessment)
5. [Weaknesses Assessment](#5-weaknesses-assessment)
6. [Sequential Correction Steps](#6-sequential-correction-steps)
7. [Detailed Component Specifications](#7-detailed-component-specifications)
8. [Testing & Validation Requirements](#8-testing--validation-requirements)
9. [Appendices](#9-appendices)

---

## 1. Current State Analysis

### 1.1 System Architecture

**Platform**: ESPHome on ESP32 (esp32doit-devkit-v1)
**Integration**: Home Assistant via Native API
**Implementation Status**: Phase 3 Complete, Phase 4 In Progress

**Key Components:**
- **Relay Control**: GPIO12 (propane stove heating element)
- **Temperature Sensor**: DS18B20 on GPIO4 (OneWire)
- **Display**: Nextion 3.5" on GPIO16/17 (UART)
- **Status LED**: GPIO2

### 1.2 Current Control Logic Implementation

Location: [includes/automation.yaml](../includes/automation.yaml:26-115)

**Current Priority Hierarchy** (INCORRECT):
```
Priority 1 (HIGHEST):  Away Mode (presence_mode == "away")
Priority 2:            Manual Run (Emergency Heat, 15-min timer)
Priority 3:            Manual Stop (Force Off)
Priority 4 (LOWEST):   Thermostat Algorithm
```

**Implementation Details:**

#### Priority 1: Away Mode
```yaml
if (away_mode) {
  if (id(relay).state) {
    id(relay).turn_off();
  }
  return;
}
```
- **Action**: Turns relay OFF unconditionally
- **Behavior**: Overrides ALL other modes including Manual Stop
- **Issue**: This is WRONG - Force Off should have highest priority

#### Priority 2: Manual Run (Emergency Heat)
- **Duration**: 15 minutes with auto-timeout
- **Action**: Forces relay ON
- **Timer**: Uses `manual_run_timeout` global variable
- **Behavior**: Deactivates Manual Stop (mutually exclusive)
- **Issue**: Should be Priority 3, and should respect Force Off

#### Priority 3: Manual Stop (Force Off)
- **Action**: Forces relay OFF indefinitely
- **Behavior**: Deactivates Manual Run (mutually exclusive)
- **Switch Type**: Toggle (correctly implemented)
- **Issue**: Should be Priority 1 (HIGHEST), not Priority 3

#### Priority 4: Thermostat Algorithm
- **Lower Threshold**: `desired - hysteresis/2.0`
- **Upper Threshold**: `desired + hysteresis/2.0`
- **Logic**: Symmetric deadband around setpoint
- **Issue**: User expects asymmetric hysteresis (see Section 3.3)

### 1.3 User Interface Entities

**Implemented in Home Assistant:**

| Entity | Type | Purpose | Status |
|--------|------|---------|--------|
| `select.presence_mode` | Select | Home/Away mode selection | ✅ Implemented |
| `switch.manual_run` | Template Switch | Emergency heat (15 min) | ✅ Implemented |
| `switch.manual_stop` | Template Switch | Force heating off | ✅ Implemented |
| `number.desired_temp` | Number | Temperature setpoint (5-35°C) | ✅ Implemented |
| `number.heating_hysteresis` | Number | Deadband (1-5°C) | ✅ Implemented |
| `number.temp_offset` | Number | Sensor calibration (-5 to +5°C) | ✅ Implemented |
| `sensor.gazebo_temperature` | Dallas Temp | Actual temperature | ✅ Implemented |

**Missing Entities (Phase 4):**

| Entity | Type | Purpose | Status |
|--------|------|---------|--------|
| `input_datetime.home_mode_time` | DateTime | Time to switch to Home mode | ❌ Missing |
| `input_datetime.away_mode_time` | DateTime | Time to switch to Away mode | ❌ Missing |
| Home Mode Time Automation | Automation | Trigger at home_mode_time | ❌ Missing |
| Away Mode Time Automation | Automation | Trigger at away_mode_time | ❌ Missing |

### 1.4 Current Thermostat Algorithm

**Location**: [includes/automation.yaml](../includes/automation.yaml:89-115)

```
Current Temperature: T_current
Desired Temperature: T_desired
Hysteresis: H

Lower Threshold = T_desired - H/2
Upper Threshold = T_desired + H/2

Control Logic:
  if T_current < Lower_Threshold:
    Turn relay ON
  else if T_current > Upper_Threshold:
    Turn relay OFF
  else:
    Maintain current state (deadband)
```

**Example with T_desired=22°C, H=2°C:**
- Lower: 21°C
- Upper: 23°C
- Heat ON when T < 21°C
- Heat OFF when T > 23°C
- Deadband: 21-23°C (maintain state)

---

## 2. Expected Functionality Specification

### 2.1 Priority Hierarchy (CORRECT)

**Source**: User requirements (2025-11-14)

```
Priority 1 (HIGHEST):  Force Off (Manual Stop)
Priority 2:            Emergency Heat (Manual Run)
Priority 3:            Schedule (Home Mode Time / Away Mode Time)
Priority 4 (LOWEST):   Thermostat Algorithm
```

### 2.2 Detailed Priority Specifications

#### Priority 1: Force Off (Manual Stop) - HIGHEST PRIORITY

**Control**: Toggle switch in Home Assistant
**Behavior**:
- When switch is ON: Stove is OFF (relay disabled)
- When switch is OFF: Lower priorities can control stove
- **Critical**: Must override ALL other modes (Away, Schedule, Emergency Heat, Thermostat)

**User Statement**: "Force off has precedence"

**Safety Rationale**:
- Maintenance mode
- Emergency shutdown
- User must have absolute control to disable heating
- No automated system should override a manual safety cutoff

#### Priority 2: Emergency Heat (Manual Run)

**Control**: Momentary switch with 15-minute timer
**Behavior**:
- When actuated: Relay turns ON for 15 minutes, then auto-disables
- Auto-timeout: Exactly 15 minutes (900 seconds)
- **Special Case**: If Force Off is ON, switch is momentary only (does NOT turn on stove)

**User Statement**: "When switch is actuated it shall remain on for 15 minutes then turn off. If Force Off is on, switch is momentary. Stove does not turn on."

**Use Case**: Emergency heating during scheduled away time without disabling entire schedule

#### Priority 3: Schedule (Home Mode Time / Away Mode Time)

**Control**: Two time inputs in Home Assistant + two automations
**Behavior**:
- **Home Mode Time**: Time when stove becomes operational (e.g., 6:00 AM)
- **Away Mode Time**: Time when stove goes to standby (e.g., 10:00 PM)
- **Operational Period**: Between Home Mode Time and Away Mode Time
- **Standby Period**: Between Away Mode Time and Home Mode Time (next day)

**User Statement**:
- "When within the period between home and away time, the stove is operational"
- "When within the period between away time and home time, the stove is non-operational"

**Schedule Example**:
```
Home Mode Time:  06:00 → Stove operational (thermostat can run)
Away Mode Time:  22:00 → Stove standby (heating disabled)

Timeline:
00:00 ─────────── 06:00 ───────────── 22:00 ───────── 24:00
      STANDBY           OPERATIONAL          STANDBY
```

**Implementation Note**: This requires Home Assistant automations, not ESPHome logic

#### Priority 4: Thermostat Algorithm - LOWEST PRIORITY

**Control**: Automatic based on temperature sensor + desired setpoint + hysteresis
**Behavior**:
- Display present state (heating/idle)
- Display requested temperature (setpoint)
- Display actual temperature (sensor reading)
- **Heat ON**: When `T_actual < T_desired - hysteresis`
- **Heat OFF**: When `T_actual > T_desired + hysteresis`

**User Statement**:
- "When the requested temperature is higher than actual temperature + hysteresis, the stove should turn on"
- "When the requested temperature is lower than actual temperature - hysteresis, the stove should turn off"

**Note**: This describes ASYMMETRIC hysteresis, not symmetric (see Section 3.3)

### 2.3 User Interface Requirements

**Home Assistant Dashboard Must Display**:
1. **Thermostat Card**:
   - Current temperature (sensor reading)
   - Desired temperature (setpoint slider)
   - Heat status indicator (heating/idle/off)

2. **Manual Switches**:
   - Emergency Heat (momentary with timer countdown)
   - Force Off (toggle switch)

3. **Current Mode Indicator**:
   - Home (operational)
   - Away (standby)

4. **Heating Schedule**:
   - Home Mode Time (time picker)
   - Away Mode Time (time picker)

---

## 3. Gap Analysis - What's Broken

### 3.1 Critical Issue #1: Priority Hierarchy Inversion

**Problem**: Away Mode (Priority 1) overrides Force Off (Priority 3)

**Current Behavior**:
```
User enables Force Off (Manual Stop) → Relay turns OFF
System enters Away Mode automatically → Relay remains OFF
User disables Force Off → Relay STAYS OFF (because Away Mode is active)
User disables Away Mode → NOW relay can turn on

Result: User cannot override Away Mode with Force Off
```

**Expected Behavior**:
```
User enables Force Off → Relay turns OFF
System enters Away Mode automatically → Relay STAYS OFF (Force Off active)
User disables Force Off → Relay can turn on if Schedule allows

Result: Force Off ALWAYS takes precedence
```

**Safety Impact**: HIGH
**Functional Impact**: CRITICAL
**User Confusion**: HIGH

**Root Cause**: Priority order in `manage_heating` script checks Away Mode first (line 44) before Manual Stop (line 81)

**Location**: [includes/automation.yaml:44-50](../includes/automation.yaml:44-50)

### 3.2 Critical Issue #2: Missing Schedule Feature (Phase 4)

**Problem**: Home Mode Time / Away Mode Time scheduling is not implemented

**Current Behavior**:
- User must manually toggle Presence Mode between "home" and "away"
- No automatic time-based switching
- No energy-saving automation

**Expected Behavior**:
- At 6:00 AM (or configured Home Mode Time): Automatically set Presence Mode to "home"
- At 10:00 PM (or configured Away Mode Time): Automatically set Presence Mode to "away"
- User can adjust times without device recompilation

**Impact**:
- Missing key energy-saving feature
- User must remember to manually switch modes
- Defeats purpose of automated heating schedule

**Status**: Documented in [PHASE_4_SIMPLE.md](PHASE_4_SIMPLE.md) but not implemented

**Implementation Required**:
1. Two `input_datetime` entities in Home Assistant
2. Two automations (time-based triggers)
3. Dashboard card for schedule control

### 3.3 Critical Issue #3: Incorrect Hysteresis Logic

**Problem**: Symmetric deadband vs. expected asymmetric hysteresis

**Current Implementation** (INCORRECT):
```
Lower Threshold = T_desired - H/2
Upper Threshold = T_desired + H/2

Example: T_desired=22°C, H=2°C
  Lower = 21°C (heat on when below)
  Upper = 23°C (heat off when above)
  Deadband = 2°C total (symmetric around setpoint)
```

**User's Expected Implementation** (CORRECT):
```
Turn ON when:  T_actual < T_desired - hysteresis
Turn OFF when: T_actual > T_desired + hysteresis

Example: T_desired=22°C, H=2°C
  Turn ON:  T < 20°C  (desired - hysteresis)
  Turn OFF: T > 24°C  (desired + hysteresis)
  Deadband = 4°C total (asymmetric, wider range)
```

**Comparison Table**:

| Setpoint | Hysteresis | Current Logic | Expected Logic |
|----------|------------|---------------|----------------|
| 22°C | 2°C | ON: <21°C, OFF: >23°C | ON: <20°C, OFF: >24°C |
| 20°C | 1°C | ON: <19.5°C, OFF: >20.5°C | ON: <19°C, OFF: >21°C |
| 18°C | 3°C | ON: <16.5°C, OFF: >19.5°C | ON: <15°C, OFF: >21°C |

**Impact**:
- Current: More frequent heating cycles (narrower deadband)
- Expected: Fewer heating cycles (wider deadband), better for propane stove longevity
- User expectation mismatch

**User Statement Analysis**:
> "When the requested temperature is higher than actual temperature + hysteresis, the stove should turn on"

This is ambiguous but appears to mean:
```
Turn ON when: T_desired > T_actual + H
Rearranged:   T_actual < T_desired - H
```

> "When the requested temperature is lower than actual temperature - hysteresis, the stove should turn off"

```
Turn OFF when: T_desired < T_actual - H
Rearranged:    T_actual > T_desired + H
```

**Conclusion**: User expects asymmetric hysteresis with full `H` offset on both sides of setpoint, not `H/2`.

**Location**: [includes/automation.yaml:90-91](../includes/automation.yaml:90-91)

### 3.4 Issue #4: Missing Force Off Override for Emergency Heat

**Problem**: Emergency Heat does not check if Force Off is active

**Current Behavior**:
```
User enables Force Off → Relay OFF
User presses Emergency Heat → Relay turns ON (Force Off is deactivated)
```

**Expected Behavior**:
```
User enables Force Off → Relay OFF
User presses Emergency Heat → Switch is momentary only, stove does NOT turn on
```

**User Statement**: "If Force Off is on, switch is momentary. Stove does not turn on"

**Interpretation**:
- When Force Off is active, Emergency Heat button should acknowledge the press but not activate heating
- Switch state may toggle momentarily but relay remains OFF
- This prevents accidental override of a safety lockout

**Impact**: MEDIUM (safety concern)

**Location**: [includes/hardware.yaml:64-79](../includes/hardware.yaml:64-79)

---

## 4. Strengths Assessment

### 4.1 Architecture & Design Strengths

✅ **Modular Configuration**
- Clean separation of concerns across 7 YAML files
- Easy to maintain and understand
- Good organization (core, hardware, sensors, entities, automation, display, buttons)

✅ **ESPHome Platform Choice**
- Declarative configuration (YAML vs. C++ code)
- Native Home Assistant integration
- Built-in OTA updates
- Excellent sensor filtering and processing

✅ **Temperature Sensor Implementation**
- 10-second moving average for stability
- Temperature offset calibration support
- Bad read detection (10+ consecutive failures trigger alarm)
- Proper clamping (-40 to 85°C)
- Delta filtering (max ±1°C change per reading)

✅ **Manual Mode Mutual Exclusivity**
- Manual Run and Manual Stop properly deactivate each other
- Prevents conflicting commands
- Clear logging of state changes

✅ **15-Minute Emergency Heat Timer**
- Correctly implemented with auto-timeout
- Timeout check every 30 seconds
- Proper cleanup on timeout expiration
- Remaining time calculation for display

✅ **Global Variables for State Management**
- Clean state tracking with `manual_run_active`, `manual_stop_active`
- Separate timeout tracking (`manual_run_timeout`)
- Bad read counter for sensor health

✅ **Comprehensive Logging**
- DEBUG level for thermostat decisions
- INFO level for state changes
- WARN/ERROR levels for anomalies
- Clear log messages for troubleshooting

✅ **Sensor Malfunction Detection**
- Counts consecutive bad reads
- Alarm triggers after 10 failures
- Binary sensor for Home Assistant alerts
- Automatic recovery when sensor returns

### 4.2 Implementation Quality Strengths

✅ **Code Clarity**
- Well-commented YAML
- Descriptive entity names
- Logical flow in scripts

✅ **Home Assistant Integration**
- All entities properly exposed
- Restore functionality for settings
- Diagnostic entities for troubleshooting

✅ **Safety Features**
- Relay defaults to OFF at boot (`restore_mode: ALWAYS_OFF`)
- Sensor malfunction forces relay OFF
- Manual modes can override automation

✅ **User Experience**
- Status LED indicates relay state
- WiFi signal monitoring
- IP address and network info exposed

---

## 5. Weaknesses Assessment

### 5.1 Critical Weaknesses

❌ **Priority Hierarchy Inversion** (CRITICAL)
- Force Off should be Priority 1, currently Priority 3
- Safety risk: Away Mode can override manual safety shutoff
- Violates principle of least surprise

❌ **Missing Schedule Feature** (CRITICAL)
- Core Phase 4 functionality not implemented
- No time-based home/away automation
- Documented but not deployed

❌ **Incorrect Hysteresis Calculation** (HIGH)
- Symmetric deadband (±H/2) vs. expected asymmetric (±H)
- Results in more frequent heating cycles
- May cause premature equipment wear

❌ **No Force Off Check in Emergency Heat** (MEDIUM)
- Emergency Heat doesn't verify Force Off state before activating
- Potential safety concern
- User expectation violation

### 5.2 Design Weaknesses

⚠️ **Schedule Logic Split Across Systems**
- ESPHome handles relay control
- Home Assistant handles time-based triggers
- Adds complexity and potential sync issues

⚠️ **No Visual Indication of Active Priority Level**
- User cannot easily see which mode is controlling the relay
- Multiple switches can be ON simultaneously
- Confusing when multiple modes are active

⚠️ **Presence Mode Naming Ambiguity**
- "Away" mode in select vs. "Manual Stop" switch
- Both disable heating but have different semantics
- User may not understand the difference

⚠️ **No Notification System**
- Phase 4 spec mentions notifications but not implemented
- User not alerted when automations run
- Silent mode changes can be confusing

### 5.3 Implementation Weaknesses

⚠️ **Hardcoded Constants**
- 15-minute timeout not user-configurable
- Could be a Home Assistant number entity
- Requires recompilation to change

⚠️ **Limited Error Handling**
- What happens if Home Assistant disconnects during operation?
- No documented behavior for API loss
- ESPHome should continue with last known settings

⚠️ **Display Integration Status Unclear**
- Nextion display code present but not validated
- No testing documentation
- Unknown if display shows correct priority status

⚠️ **No Historical State Tracking**
- Can't see heating cycle history
- No runtime counters
- Difficult to optimize settings

---

## 6. Sequential Correction Steps

### Overview

The following steps must be executed **in order** to correct the thermostat control system. Each step includes validation criteria to ensure successful completion before proceeding.

**Estimated Total Time**: 6-8 hours
**Risk Level**: MEDIUM (requires careful testing)
**Rollback Strategy**: Backup current YAML files before modifications

---

### Step 1: Priority Hierarchy Correction (CRITICAL)

**Objective**: Reorder priority checks to make Force Off (Manual Stop) the highest priority

**Duration**: 30 minutes
**Risk**: MEDIUM (core control logic change)
**Testing Required**: YES (extensive)

**Actions**:

1.1. **Backup Current Configuration**
   - Copy `includes/automation.yaml` to `includes/automation.yaml.backup`
   - Commit current state to Git (if using version control)

1.2. **Modify `manage_heating` Script Priority Order**

**Location**: [includes/automation.yaml:28-115](../includes/automation.yaml:28-115)

**Current Order** (INCORRECT):
```
1. Check away_mode
2. Check manual_run
3. Check manual_stop
4. Run thermostat algorithm
```

**Corrected Order** (REQUIRED):
```
1. Check manual_stop (Force Off) → HIGHEST PRIORITY
2. Check manual_run (Emergency Heat)
3. Check away_mode (Schedule - will be renamed in Step 3)
4. Run thermostat algorithm → LOWEST PRIORITY
```

**Specific Changes**:
- Move lines 81-87 (Manual Stop check) to immediately after line 30
- Rename variable `away_mode` to `schedule_away` for clarity
- Update log messages to reflect new priority order

**Validation Criteria**:
- ✅ Manual Stop active → Relay OFF regardless of other modes
- ✅ Manual Stop overrides Away Mode
- ✅ Manual Stop overrides Emergency Heat
- ✅ Manual Stop overrides Thermostat

---

### Step 2: Emergency Heat Force Off Check (HIGH PRIORITY)

**Objective**: Prevent Emergency Heat from activating when Force Off is enabled

**Duration**: 20 minutes
**Risk**: LOW (simple conditional addition)
**Testing Required**: YES

**Actions**:

2.1. **Modify Manual Run Switch Turn-On Action**

**Location**: [includes/hardware.yaml:64-79](../includes/hardware.yaml:64-79)

**Add Conditional Check**:
Before activating Emergency Heat, check if Manual Stop is active. If true:
- Log "Emergency Heat requested but Force Off is active - ignoring"
- Switch may toggle momentarily but relay stays OFF
- Display message to user (optional)

**Pseudo-code**:
```
turn_on_action:
  if manual_stop_active:
    log: "Emergency Heat blocked by Force Off"
    publish_state(false)  # Switch returns to OFF
    return
  else:
    # Existing activation logic
```

**Validation Criteria**:
- ✅ Force Off active → Emergency Heat press has no effect
- ✅ Force Off inactive → Emergency Heat works normally
- ✅ Log message indicates when Emergency Heat is blocked

---

### Step 3: Implement Home/Away Schedule (Phase 4)

**Objective**: Add time-based automation for Home Mode Time and Away Mode Time

**Duration**: 2-3 hours
**Risk**: LOW (Home Assistant only, no ESPHome changes)
**Testing Required**: YES

**Actions**:

3.1. **Create Time Input Entities in Home Assistant**

**File**: Home Assistant `configuration.yaml` (or Helpers via UI)

**Add**:
```yaml
input_datetime:
  gazebo_home_mode_time:
    name: "Gazebo Home Mode Time"
    has_date: false
    has_time: true
    icon: mdi:home-clock
    initial: "06:00:00"

  gazebo_away_mode_time:
    name: "Gazebo Away Mode Time"
    has_date: false
    has_time: true
    icon: mdi:clock-out
    initial: "22:00:00"
```

**Validation**: Time pickers appear in Home Assistant UI

---

3.2. **Create Home Mode Automation**

**File**: Home Assistant `automations.yaml` (or via UI)

**Add**:
```yaml
- alias: "Gazebo: Set Home Mode at Time"
  description: "Automatically set presence to home at configured time"
  trigger:
    - platform: time
      at: input_datetime.gazebo_home_mode_time
  condition: []
  action:
    - service: select.select_option
      target:
        entity_id: select.gazebo_presence_mode
      data:
        option: "home"
    - service: system_log.write
      data:
        level: info
        message: "Gazebo: Automatic home mode enabled at {{ now().strftime('%H:%M') }}"
  mode: single
```

**Validation**: Automation triggers at configured time and sets Presence Mode to "home"

---

3.3. **Create Away Mode Automation**

**File**: Home Assistant `automations.yaml`

**Add**:
```yaml
- alias: "Gazebo: Set Away Mode at Time"
  description: "Automatically set presence to away at configured time"
  trigger:
    - platform: time
      at: input_datetime.gazebo_away_mode_time
  condition: []
  action:
    - service: select.select_option
      target:
        entity_id: select.gazebo_presence_mode
      data:
        option: "away"
    - service: system_log.write
      data:
        level: info
        message: "Gazebo: Automatic away mode enabled at {{ now().strftime('%H:%M') }}"
  mode: single
```

**Validation**: Automation triggers at configured time and sets Presence Mode to "away"

---

3.4. **Create Dashboard Schedule Card**

**File**: Home Assistant dashboard YAML (or via UI)

**Add Card**:
```yaml
type: entities
title: Gazebo Stove - Heating Schedule
entities:
  - entity: input_datetime.gazebo_home_mode_time
    name: "Home Mode Begins"
    icon: mdi:home-clock
  - entity: input_datetime.gazebo_away_mode_time
    name: "Away Mode Begins"
    icon: mdi:clock-out
  - entity: select.gazebo_presence_mode
    name: "Current Mode"
  - type: section
  - entity: switch.gazebo_manual_run
    name: "Emergency Heat"
  - entity: switch.gazebo_manual_stop
    name: "Force Off"
```

**Validation**: Dashboard card displays schedule controls and current status

---

3.5. **Update ESPHome Presence Mode Logic**

**Location**: [includes/automation.yaml:44-50](../includes/automation.yaml:44-50)

**Rename and Document**:
- Variable `away_mode` → `schedule_standby` (more descriptive)
- Add comment explaining this is controlled by schedule automations
- Update all log messages for clarity

**Validation**: ESPHome logs show "Schedule: Standby mode" instead of "Away mode"

---

### Step 4: Correct Thermostat Hysteresis Calculation (HIGH PRIORITY)

**Objective**: Change from symmetric (±H/2) to asymmetric (±H) hysteresis

**Duration**: 30 minutes
**Risk**: MEDIUM (affects heating cycles)
**Testing Required**: YES (extensive monitoring required)

**Actions**:

4.1. **Modify Hysteresis Calculation**

**Location**: [includes/automation.yaml:90-91](../includes/automation.yaml:90-91)

**Current (INCORRECT)**:
```cpp
float lower_threshold = desired - hysteresis/2.0;
float upper_threshold = desired + hysteresis/2.0;
```

**Corrected (REQUIRED)**:
```cpp
float lower_threshold = desired - hysteresis;
float upper_threshold = desired + hysteresis;
```

**Example Impact** (T_desired=22°C, H=2°C):
- Before: Heat ON < 21°C, OFF > 23°C (2°C total deadband)
- After:  Heat ON < 20°C, OFF > 24°C (4°C total deadband)

**Validation Criteria**:
- ✅ With T_desired=22°C, H=2°C: Relay turns ON at 19.9°C, OFF at 24.1°C
- ✅ Heating cycles are less frequent
- ✅ Temperature oscillations are wider but within acceptable range

**User Testing Required**:
- Monitor heating behavior for 48-72 hours
- Adjust hysteresis value if needed (may need to reduce from 2°C to 1°C)
- User should confirm comfort level with new deadband

---

4.2. **Update Hysteresis Number Entity Range**

**Location**: [includes/entities.yaml:69-84](../includes/entities.yaml:69-84)

**Current**: `min_value: 1, max_value: 5, step: 0.5`

**Recommendation**: Reduce max_value to 3°C for asymmetric hysteresis
- With asymmetric logic, H=5°C means ±5°C (10°C total deadband) - too wide!
- Suggested range: `min_value: 0.5, max_value: 3, step: 0.25`

**Validation**: User can adjust hysteresis and observe heating cycle changes

---

### Step 5: Add Priority Status Indicator (RECOMMENDED)

**Objective**: Display which priority level is currently controlling the relay

**Duration**: 1 hour
**Risk**: LOW (informational only)
**Testing Required**: Minimal

**Actions**:

5.1. **Create Text Sensor for Active Priority**

**Location**: [includes/entities.yaml](../includes/entities.yaml) (add to text_sensor section)

**Add**:
```yaml
- platform: template
  name: "Active Control Mode"
  id: active_control_mode
  icon: "mdi:priority-high"
  update_interval: 5s
  lambda: |-
    if (id(manual_stop_active)) {
      return std::string("Force Off (Priority 1)");
    } else if (id(manual_run_active)) {
      return std::string("Emergency Heat (Priority 2)");
    } else if (id(presence_mode).state == "away") {
      return std::string("Schedule: Standby (Priority 3)");
    } else {
      return std::string("Thermostat (Priority 4)");
    }
```

**Validation**:
- ✅ Text sensor shows "Force Off" when Manual Stop active
- ✅ Changes to "Emergency Heat" when Manual Run active
- ✅ Shows "Schedule: Standby" during away mode
- ✅ Shows "Thermostat" during normal operation

---

5.2. **Update Dashboard Card**

Add active control mode indicator to main dashboard card

**Validation**: User can see which mode is controlling the stove at a glance

---

### Step 6: Documentation Updates

**Objective**: Update all documentation to reflect corrected behavior

**Duration**: 1 hour
**Risk**: NONE
**Testing Required**: NO

**Actions**:

6.1. **Update FEATURE_MAPPING.md**
- Correct priority hierarchy documentation
- Add schedule implementation details
- Update hysteresis calculation description

6.2. **Update PROJECT_STATUS.md**
- Mark Phase 4 as "Complete"
- Add "Correction Phase" section
- Document changes made

6.3. **Create THERMOSTAT_USER_GUIDE.md**
- Explain priority hierarchy to end users
- Provide usage scenarios
- Include troubleshooting section

6.4. **Update PHASE_3_USER_GUIDE.md**
- Correct priority order
- Add schedule usage instructions

**Validation**: All documentation is consistent and accurate

---

### Step 7: Comprehensive Testing & Validation

**Objective**: Verify all corrections work as expected

**Duration**: 2-3 hours (plus 48-hour monitoring)
**Risk**: NONE (read-only)
**Testing Required**: EXTENSIVE

**Test Plan**:

#### 7.1 Priority Hierarchy Tests

**Test 1: Force Off Overrides All**
1. Set Presence Mode to "away"
2. Enable Force Off → Relay should be OFF
3. Try to enable Emergency Heat → Should fail (relay stays OFF)
4. Disable Force Off → Relay should remain OFF (away mode active)
5. Set Presence Mode to "home" → Thermostat should resume

**Expected Result**: Force Off ALWAYS keeps relay OFF

---

**Test 2: Emergency Heat Overrides Schedule**
1. Set Presence Mode to "away" (stove should be OFF)
2. Enable Emergency Heat → Relay should turn ON
3. Wait 15 minutes → Relay should turn OFF automatically
4. Verify Presence Mode still shows "away"

**Expected Result**: Emergency Heat temporarily overrides away mode

---

**Test 3: Schedule Overrides Thermostat**
1. Set desired temp to 30°C (force heating demand)
2. Set Presence Mode to "away" → Relay should be OFF despite demand
3. Set Presence Mode to "home" → Relay should turn ON (if temp < threshold)

**Expected Result**: Schedule prevents thermostat from running during away mode

---

**Test 4: Thermostat Normal Operation**
1. Disable all manual modes
2. Set Presence Mode to "home"
3. Set desired temp to 25°C (above current temp)
4. Relay should remain OFF (no heating needed)
5. Lower desired temp to 15°C (below current temp - H)
6. Relay should turn ON
7. Raise actual temp above desired + H → Relay should turn OFF

**Expected Result**: Thermostat follows hysteresis logic correctly

---

#### 7.2 Schedule Automation Tests

**Test 5: Home Mode Time Trigger**
1. Set Home Mode Time to 2 minutes from now
2. Verify Presence Mode is currently "away"
3. Wait for trigger time
4. Verify Presence Mode changes to "home"
5. Check Home Assistant logs for automation execution message

**Expected Result**: Automation runs at configured time

---

**Test 6: Away Mode Time Trigger**
1. Set Away Mode Time to 2 minutes from now
2. Verify Presence Mode is currently "home"
3. Wait for trigger time
4. Verify Presence Mode changes to "away"
5. Verify relay turns OFF if heating was active

**Expected Result**: Automation runs and disables heating

---

**Test 7: Schedule Across Midnight**
1. Set Home Mode Time to 23:50 (late evening)
2. Set Away Mode Time to 00:10 (after midnight)
3. Wait for both triggers
4. Verify correct mode changes

**Expected Result**: Schedule handles midnight boundary correctly

---

#### 7.3 Hysteresis Tests

**Test 8: Asymmetric Hysteresis Behavior**
1. Set desired temp = 22°C
2. Set hysteresis = 2°C
3. Heat room to 25°C
4. Let temp drop slowly
5. Record temperature when relay turns ON (should be ~20°C, not 21°C)
6. Let heating run until relay turns OFF
7. Record temperature when relay turns OFF (should be ~24°C, not 23°C)

**Expected Result**: ON at (desired - H), OFF at (desired + H)

---

#### 7.4 Force Off Emergency Heat Interaction Test

**Test 9: Emergency Heat with Force Off Active**
1. Enable Force Off → Relay OFF
2. Press Emergency Heat switch
3. Verify switch does NOT stay ON
4. Verify relay remains OFF
5. Check logs for "blocked by Force Off" message

**Expected Result**: Emergency Heat is rejected when Force Off is active

---

#### 7.5 Long-Term Stability Tests

**Test 10: 48-Hour Continuous Operation**
1. Enable all corrections
2. Configure realistic schedule (e.g., home 6am-10pm)
3. Monitor for 48 hours
4. Record:
   - Number of heating cycles per day
   - Temperature min/max/avg during home mode
   - Any unexpected relay activations
   - Schedule automation success rate
   - Manual mode interventions

**Expected Result**:
- No unexpected behavior
- Heating cycles within normal range
- Schedule automations trigger reliably
- System remains responsive

---

**Test 11: Edge Case Stress Test**
1. Rapidly toggle Force Off on/off (10 times)
2. Activate Emergency Heat while toggling Force Off
3. Change Presence Mode during Emergency Heat countdown
4. Adjust desired temp while heating is active
5. Disconnect/reconnect Home Assistant API
6. Reboot ESP32 with Force Off enabled

**Expected Result**: System remains stable, no crashes, relay state consistent

---

### Step 8: Performance Optimization (OPTIONAL)

**Objective**: Fine-tune system performance based on test results

**Duration**: 1-2 hours
**Risk**: LOW
**Testing Required**: YES

**Potential Optimizations**:

8.1. **Adjust Sensor Update Interval**
- Current: 10 seconds
- Consider: 15-30 seconds to reduce processing load
- Trade-off: Slower response vs. lower power consumption

8.2. **Tune Hysteresis Default Value**
- Current: 2°C
- User preference: May need 1°C or 1.5°C based on comfort
- Recommendation: User configurable, no code change needed

8.3. **Add Heating Cycle Counter**
- Track number of heating cycles per day
- Expose as sensor for monitoring
- Help identify excessive cycling

8.4. **Implement Min Run Time / Min Off Time**
- Prevent rapid relay cycling
- Protect propane stove igniter
- Typical values: 3-5 minutes minimum

**Validation**: Monitor for another 48 hours after optimization

---

## 7. Detailed Component Specifications

### 7.1 Priority Control Flow Diagram

```
┌─────────────────────────────────────────────────────────┐
│           THERMOSTAT CONTROL DECISION TREE              │
└─────────────────────────────────────────────────────────┘

START: manage_heating script called
│
├─ CHECK 1: Is Manual Stop (Force Off) active?
│  │
│  ├─ YES → Turn relay OFF
│  │         Log: "Force Off: Relay OFF (Priority 1)"
│  │         RETURN (exit script)
│  │
│  └─ NO → Continue to Check 2
│
├─ CHECK 2: Is Manual Run (Emergency Heat) active?
│  │
│  ├─ YES → Check if timeout expired
│  │         │
│  │         ├─ Timeout expired?
│  │         │  ├─ YES → Disable Manual Run, Continue to Check 3
│  │         │  └─ NO  → Turn relay ON
│  │         │           Log: "Emergency Heat: Relay ON (Priority 2)"
│  │         │           RETURN (exit script)
│  │
│  └─ NO → Continue to Check 3
│
├─ CHECK 3: Is Presence Mode "away" (Schedule)?
│  │
│  ├─ YES → Turn relay OFF
│  │         Log: "Schedule: Standby mode (Priority 3)"
│  │         RETURN (exit script)
│  │
│  └─ NO → Continue to Check 4
│
└─ CHECK 4: Run Thermostat Algorithm (Priority 4)
   │
   ├─ Calculate thresholds:
   │  Lower = Desired - Hysteresis
   │  Upper = Desired + Hysteresis
   │
   ├─ Is Current Temp < Lower Threshold?
   │  ├─ YES → Turn relay ON
   │  │         Log: "Heating: Start (Priority 4)"
   │  │
   │  └─ NO → Is Current Temp > Upper Threshold?
   │            ├─ YES → Turn relay OFF
   │            │         Log: "Heating: Stop (Priority 4)"
   │            │
   │            └─ NO → Maintain current state (deadband)
   │                    Log: "In deadband (no change)"
   │
   RETURN
```

### 7.2 Schedule State Diagram

```
┌─────────────────────────────────────────────────────────┐
│              DAILY HEATING SCHEDULE                     │
└─────────────────────────────────────────────────────────┘

                Home Mode Time         Away Mode Time
                     ↓                       ↓
    00:00 ───────── 06:00 ────────────── 22:00 ───────── 24:00
      │               │                     │               │
      │   STANDBY     │    OPERATIONAL      │   STANDBY     │
      │   (Away)      │     (Home)          │   (Away)      │
      │               │                     │               │
      │  Relay OFF    │  Thermostat Active  │  Relay OFF    │
      │  (unless      │  (unless manual     │  (unless      │
      │   manual      │   overrides)        │   manual      │
      │   overrides)  │                     │   overrides)  │
      │               │                     │               │
      └───────────────┴─────────────────────┴───────────────┘

State Transitions:
  - At Home Mode Time (06:00): Presence Mode → "home"
  - At Away Mode Time (22:00): Presence Mode → "away"

Manual Overrides:
  - Force Off: Disables heating in ANY state
  - Emergency Heat: Enables heating in ANY state (15 min)
  - Manual mode change: User can override schedule anytime
```

### 7.3 Priority Override Matrix

| Current State | Force Off | Emergency Heat | Schedule Away | Thermostat Demand | Relay State | Active Priority |
|---------------|-----------|----------------|---------------|-------------------|-------------|-----------------|
| Normal Home   | OFF       | OFF            | NO (home)     | YES (heat needed) | **ON**      | Priority 4 (Thermostat) |
| Normal Home   | OFF       | OFF            | NO (home)     | NO (temp OK)      | **OFF**     | Priority 4 (Thermostat) |
| Scheduled Away| OFF       | OFF            | YES (away)    | YES (heat needed) | **OFF**     | Priority 3 (Schedule) |
| Emergency Heat| OFF       | ON             | YES (away)    | N/A               | **ON**      | Priority 2 (Emergency) |
| Force Off     | ON        | OFF            | NO (home)     | YES (heat needed) | **OFF**     | Priority 1 (Force Off) |
| Force Off     | ON        | ATTEMPTED      | NO (home)     | N/A               | **OFF**     | Priority 1 (Force Off) |
| Emergency + Force | ON    | ATTEMPTED      | YES (away)    | N/A               | **OFF**     | Priority 1 (Force Off) |
| All Active    | ON        | ATTEMPTED      | YES (away)    | YES (heat needed) | **OFF**     | Priority 1 (Force Off) |

**Key Insight**: Force Off ALWAYS results in relay OFF, regardless of other conditions.

### 7.4 Hysteresis Comparison

**Symmetric (Current - INCORRECT)**:
```
Temperature (°C)
      ↑
  25  │
      │     ┌─────────┐ Relay OFF
  24  │     │         │
      │     │         │
  23  │─────┤  DEAD   ├───── Upper Threshold (Desired + H/2)
      │     │  BAND   │
  22  │═════╪═ ★ ═════╪═══── Desired Setpoint ★
      │     │         │
  21  │─────┤         ├───── Lower Threshold (Desired - H/2)
      │     │         │
  20  │     │         │
      │     └─────────┘ Relay ON
      │
      └────────────────────→ Time

Hysteresis = 2°C
Deadband Width = 2°C (±1°C around setpoint)
```

**Asymmetric (Corrected - REQUIRED)**:
```
Temperature (°C)
      ↑
  26  │
      │
  25  │
      │     ┌─────────┐
  24  │─────┤         ├───── Upper Threshold (Desired + H)
      │     │         │       Relay turns OFF here
  23  │     │         │
      │     │  DEAD   │
  22  │═════╪═ ★ ═════╪═══── Desired Setpoint ★
      │     │  BAND   │
  21  │     │         │
      │     │         │
  20  │─────┤         ├───── Lower Threshold (Desired - H)
      │     │         │       Relay turns ON here
  19  │     └─────────┘
      │
      └────────────────────→ Time

Hysteresis = 2°C
Deadband Width = 4°C (±2°C around setpoint)
```

**Practical Example**:
- Desired Setpoint: 22°C
- Hysteresis: 2°C

| Implementation | Turn ON Temp | Turn OFF Temp | Deadband Width | Cycle Frequency |
|----------------|--------------|---------------|----------------|-----------------|
| Symmetric (current) | 21°C | 23°C | 2°C | Higher (more cycles) |
| Asymmetric (correct) | 20°C | 24°C | 4°C | Lower (fewer cycles) |

**Recommendation**: Use asymmetric hysteresis as specified by user

---

## 8. Testing & Validation Requirements

### 8.1 Acceptance Criteria

Before declaring corrections complete, ALL of the following must be verified:

#### Priority Hierarchy
- [ ] Manual Stop (Force Off) overrides Away Mode
- [ ] Manual Stop overrides Emergency Heat
- [ ] Manual Stop overrides Thermostat
- [ ] Emergency Heat overrides Schedule
- [ ] Emergency Heat overrides Thermostat
- [ ] Schedule overrides Thermostat
- [ ] Thermostat runs only when no higher priority is active

#### Schedule Functionality
- [ ] Home Mode Time automation triggers at configured time
- [ ] Away Mode Time automation triggers at configured time
- [ ] Presence Mode changes to "home" at Home Mode Time
- [ ] Presence Mode changes to "away" at Away Mode Time
- [ ] Relay turns OFF when schedule enters away mode
- [ ] Thermostat resumes when schedule enters home mode
- [ ] Schedule handles midnight boundary correctly
- [ ] Times are user-configurable without device recompilation

#### Hysteresis Behavior
- [ ] Relay turns ON when temp < (desired - hysteresis)
- [ ] Relay turns OFF when temp > (desired + hysteresis)
- [ ] Deadband is (2 × hysteresis) wide, not just hysteresis
- [ ] Temperature oscillates within expected range
- [ ] Heating cycles are appropriate frequency (not excessive)

#### Emergency Heat / Force Off Interaction
- [ ] Emergency Heat cannot activate when Force Off is enabled
- [ ] Emergency Heat switch returns to OFF when blocked by Force Off
- [ ] Log message indicates Emergency Heat was blocked
- [ ] Emergency Heat works normally when Force Off is disabled

#### System Stability
- [ ] 48-hour continuous operation without crashes
- [ ] ESPHome device remains responsive
- [ ] Home Assistant connection stable
- [ ] All entity states accurate
- [ ] Logs show expected priority decisions

#### User Interface
- [ ] All switches and controls work as expected
- [ ] Active control mode indicator shows correct priority
- [ ] Dashboard displays all necessary information
- [ ] Time pickers for schedule are functional
- [ ] Manual modes are clearly labeled

### 8.2 Test Environment Setup

**Hardware Requirements**:
- GazeboStove device (ESP32 with DS18B20 and relay)
- Home Assistant instance (version 2024.x or later)
- Propane stove (or test relay load)
- Temperature probe calibrated
- Stable WiFi connection

**Software Requirements**:
- ESPHome CLI or Home Assistant ESPHome integration
- Text editor for YAML files
- Git for version control (recommended)
- Home Assistant Companion app for notifications (optional)

**Safety Precautions**:
- Test with propane stove OFF (gas valve closed) initially
- Monitor first 24 hours of operation closely
- Have rollback plan ready (backup YAML files)
- Ensure manual gas shutoff is accessible

### 8.3 Test Data Collection

**Log Data to Collect**:
- ESPHome device logs (priority decisions, relay state changes)
- Home Assistant automation history
- Sensor readings (temperature, WiFi signal)
- Switch state changes
- Script execution times

**Metrics to Track**:
- Heating cycles per day (target: 8-15 cycles for typical operation)
- Average cycle duration (ON time and OFF time)
- Temperature min/max/average during home mode
- Schedule automation success rate (100% expected)
- Manual mode intervention frequency
- API disconnection events (should be zero)

**Documentation**:
- Test execution date/time
- Configuration values used (desired temp, hysteresis, schedule times)
- Observed vs. expected behavior
- Any anomalies or unexpected results
- User feedback on comfort level

### 8.4 Rollback Procedure

If issues arise during testing:

1. **Immediate Rollback**:
   - Copy `includes/automation.yaml.backup` to `includes/automation.yaml`
   - Copy any other modified files from backup
   - Recompile and upload to ESP32
   - Disable Home Assistant schedule automations

2. **Verify Rollback Success**:
   - Device boots without errors
   - Relay responds to manual controls
   - Temperature sensor reading correctly
   - Home Assistant connection restored

3. **Root Cause Analysis**:
   - Review logs for errors
   - Identify which correction step caused issue
   - Re-test that specific step in isolation

4. **Incremental Re-Implementation**:
   - Apply corrections one at a time
   - Test each step thoroughly before proceeding
   - Document what works and what doesn't

---

## 9. Appendices

### Appendix A: File Locations Reference

| File | Purpose | Lines to Modify |
|------|---------|-----------------|
| `includes/automation.yaml` | Priority logic, heating control | 28-115 (manage_heating script) |
| `includes/hardware.yaml` | Manual Run switch, Force Off check | 64-79 (turn_on_action) |
| `includes/entities.yaml` | Hysteresis range, status indicators | 69-84 (heating_hysteresis) |
| Home Assistant `configuration.yaml` | Time input entities | Add input_datetime section |
| Home Assistant `automations.yaml` | Schedule triggers | Add 2 automations |

### Appendix B: Entity ID Reference

**ESPHome Entities** (exposed to Home Assistant):
- `switch.gazebo_propane_stove_relay` - Physical relay control
- `switch.gazebo_manual_run` - Emergency Heat (15 min timer)
- `switch.gazebo_manual_stop` - Force Off (toggle)
- `select.gazebo_presence_mode` - Home/Away mode
- `number.gazebo_desired_temperature` - Setpoint (5-35°C)
- `number.gazebo_heating_hysteresis` - Deadband (1-5°C)
- `number.gazebo_temperature_offset` - Calibration (-5 to +5°C)
- `sensor.gazebo_temperature` - Actual temperature (DS18B20)
- `binary_sensor.gazebo_sensor_malfunction` - Sensor health alarm
- `text_sensor.gazebo_system_status` - Status message
- `text_sensor.gazebo_active_control_mode` - Priority indicator (new)

**Home Assistant Entities** (to be created):
- `input_datetime.gazebo_home_mode_time` - Home mode start time
- `input_datetime.gazebo_away_mode_time` - Away mode start time
- `automation.gazebo_set_home_mode_at_time` - Home mode trigger
- `automation.gazebo_set_away_mode_at_time` - Away mode trigger

### Appendix C: Glossary of Terms

- **Force Off**: Manual Stop switch, highest priority, disables all heating
- **Emergency Heat**: Manual Run switch, 15-minute timer, overrides schedule
- **Schedule**: Time-based home/away automation (Phase 4 feature)
- **Standby**: Heating disabled (during away mode or outside schedule)
- **Operational**: Heating enabled (during home mode or within schedule)
- **Presence Mode**: Home Assistant select entity (home/away)
- **Priority**: Hierarchy determining which control mode is active
- **Hysteresis**: Temperature deadband to prevent rapid cycling
- **Deadband**: Range where relay state doesn't change
- **Setpoint**: Desired temperature target
- **Asymmetric Hysteresis**: ±H on both sides of setpoint (±H total = 2H deadband)
- **Symmetric Hysteresis**: ±H/2 on both sides of setpoint (±H/2 total = H deadband)

### Appendix D: Log Message Reference

**Priority Decision Logs** (manage_heating script):
```
"Force Off: Relay OFF (Priority 1)"
"Emergency Heat: Relay ON (Priority 2, XX ms remaining)"
"Schedule: Standby mode (Priority 3)"
"Heating: Start (Priority 4)"
"Heating: Stop (Priority 4)"
"In deadband: XX-XX, Relay=ON/OFF (no change)"
```

**Manual Mode Logs**:
```
"Manual Run enabled - 15 minute emergency heating"
"Manual Run: Activated - 15-min timer started at XXXXX ms"
"Manual Run: Deactivated by user"
"Manual Run: Timeout check: XX:XX remaining"
"Manual Run: 15 minutes reached - auto-disabling"
"Manual Stop enabled - Heating disabled"
"Manual Stop disabled - Restoring normal control"
"Emergency Heat blocked by Force Off"
```

**Schedule Logs** (Home Assistant):
```
"Gazebo: Automatic home mode enabled at HH:MM"
"Gazebo: Automatic away mode enabled at HH:MM"
```

### Appendix E: Related Documentation

1. **FEATURE_MAPPING.md** - Original Arduino to ESPHome feature mapping
2. **PHASE_4_SIMPLE.md** - Schedule implementation guide
3. **PROJECT_STATUS.md** - Overall project status and phase tracking
4. **PHASE_3_USER_GUIDE.md** - Manual modes usage guide (to be updated)
5. **NEXTION_DISPLAY.md** - Display integration (may need priority indicator update)

### Appendix F: Change Log

| Date | Version | Author | Changes |
|------|---------|--------|---------|
| 2025-11-14 | 1.0 | Analysis | Initial specification created |

---

## Summary

This specification document identifies **four critical issues** in the GazeboStove thermostat control system:

1. **Priority Hierarchy Inversion** (CRITICAL): Force Off must be highest priority, not third
2. **Missing Schedule Feature** (CRITICAL): Home Mode Time / Away Mode Time not implemented
3. **Incorrect Hysteresis Logic** (HIGH): Symmetric vs. asymmetric calculation
4. **Missing Force Off Override** (MEDIUM): Emergency Heat doesn't check Force Off state

**Sequential correction steps** have been provided to address each issue in the proper order, with comprehensive testing and validation requirements.

**Estimated Total Effort**: 6-8 hours implementation + 48-72 hours monitoring

**Next Steps**:
1. Review this specification with stakeholders
2. Obtain approval to proceed with corrections
3. Schedule implementation window (low-usage time)
4. Execute Step 1 (Priority Hierarchy Correction)
5. Execute remaining steps sequentially
6. Conduct 48-hour stability test
7. Document results and user feedback

---

**Document Status**: FINAL - Ready for Implementation
**Approval Required**: YES
**Implementation Risk**: MEDIUM (core control logic changes)
**Rollback Strategy**: Documented and tested

---

**END OF SPECIFICATION DOCUMENT**
