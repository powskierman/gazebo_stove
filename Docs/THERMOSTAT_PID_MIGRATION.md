# Thermostat PID Migration & Sync Logic Fixes

**Date**: 2025-12-07
**Feature**: Migration from `generic_thermostat` to `pid` platform
**Status**: ✅ Implemented and Verified

---

## Overview

This document details the critical architectural change from the standard `generic_thermostat` (and failing `thermostat`) platforms to a robust **PID Thermostat** implementation. This change resolved persistent "Platform not found" errors and fixed the "split brain" issue where the UI status did not match the actual heating state.

## Problem Statement

### 1. Missing Climate Entity
- **Issue**: The `generic_thermostat` and `thermostat` platforms failed to compile in ESPHome 2025.10.3 due to missing dependencies and linker errors.
- **Impact**: The device had no climate entity, making it impossible to control via Home Assistant despite the underlying hardware working.

### 2. "Split Brain" Status
- **Issue**: The previous automation logic controlled the relay *directly*, bypassing the thermostat entity.
- **Impact**: The thermostat UI would show "Off" even when the stove was actively heating (e.g., due to "Home" mode), causing confusion and lack of visibility.

---

## Solution Architecture

### 1. The "Brain Transplant" (PID Platform)
We migrated to the standard `pid` climate platform, which proved stable and compatible with the current environment.

**Configuration**:
```yaml
climate:
  - platform: pid
    name: "Propane Stove Thermostat"
    id: gazebo_thermostat
    sensor: gazebo_temp
    default_target_temperature: 22.0
    heat_output: thermostat_output
    control_parameters:
      kp: 10.0  # High proportional term for "bang-bang" like behavior
      ki: 0.0
      kd: 0.0
```

### 2. Hardware Output Safeguards ("Sync Logic")
To ensure the "Status Off vs Heating" discrepancy never recurs, we moved the priority enforcement down to the **Hardware Output** level. The `thermostat_output` (which the PID controller drives) now checks the manual override flags *before* touching the relay.

**Logic**:
- **Manual Run (Emergency Heat)**: Forces Relay **ON**, regardless of PID output.
- **Manual Stop (Force Off)**: Forces Relay **OFF**, regardless of PID output.
- **PID Demand**: Controls relay only if no manual overrides are active.

```yaml
# includes/hardware.yaml
output:
  - platform: template
    id: thermostat_output
    type: float
    write_action:
      - if:
          condition:
            lambda: |-
              // Priority Logic Enforced Here
              bool should_run = (state > 0.5 || id(manual_run_active));
              bool force_stop = id(manual_stop_active);
              return should_run && !force_stop; 
          then:
            - switch.turn_on: relay
          else:
            - switch.turn_off: relay
```

### 3. Automation Refactor
The `manage_heating` automation script was simplified. Instead of fighting the thermostat for control of the relay, it now **controls the thermostat's mode**.

- **Schedule: Away** → Sets Thermostat Mode to `OFF`.
- **Schedule: Home** → Sets Thermostat Mode to `HEAT`.

This ensures the UI (which reflects the Thermostat Mode) always matches the logic. If the schedule says "Away", the UI shows "Off". If "Home", the UI shows "Heating" (active or idle).

---

## Results

- **Entity Restored**: `climate.propane_stove_thermostat` is available and functional.
- **Accurate Status**: The Home Assistant card now correctly displays "Green/Heating" when the stove is running and "Grey/Off" when in standby or manually stopped.
- **Robust Control**: Manual overrides work identically to before but are now deeply integrated into the output driver, providing a fail-safe layer.

## Future Maintenance

- **PID Tuning**: Currently set to `Kp=10.0` for simple on/off behavior. If smoother control is desired later, `Kp`, `Ki`, and `Kd` can be tuned.
- **Hysteresis**: Now managed implicitly by the PID controller's response to temperature error.
