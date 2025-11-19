# Implementation Plan: Gazebo Propane Stove Heating Control System

**Branch**: `001-gazebo-stove-heating-control` | **Date**: 2025-01-27 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-gazebo-stove-heating-control/spec.md`

## Summary

Convert Arduino/Blynk-based gazebo heating control system to ESPHome + Home Assistant, eliminating cloud dependency while maintaining 100% feature parity. System monitors temperature via DS18B20 sensor, controls propane stove relay, displays status on Nextion touchscreen, and provides remote control through Home Assistant. Heating-only system with priority-based control: Force Off (highest) → Emergency Heat → Schedule → Thermostat (lowest).

## Technical Context

**Language/Version**: ESPHome YAML configuration (2024.x+), C++ lambdas for custom logic, Home Assistant YAML for automations  
**Primary Dependencies**: ESPHome framework, Home Assistant Native API, ESP32 Arduino framework, Nextion display library  
**Storage**: Home Assistant entity state persistence (replaces EEPROM), ESPHome restore_value for device-side persistence  
**Testing**: Manual hardware verification, ESPHome config validation, Home Assistant state history analysis, UART logging for Nextion debug  
**Target Platform**: ESP32 (esp32doit-devkit-v1), Home Assistant (local deployment)  
**Project Type**: Single embedded device project (ESPHome YAML + Home Assistant automations)  
**Performance Goals**: 
- Relay response: <10 seconds from sensor reading to relay state change
- Display updates: <2 seconds for state changes
- Remote control latency: <2 seconds from Home Assistant command to device response
- WiFi resilience: Continue operation ≥60 minutes without connectivity

**Constraints**: 
- Heating-only system (no cooling capability)
- Must operate offline (WiFi-independent thermostat control)
- Safety-first: Force Off cannot be overridden
- OTA updates blocked when relay is ON
- Sensor failure must deactivate relay (safety requirement)

**Scale/Scope**: 
- Single device deployment
- ~500 lines ESPHome YAML configuration
- ~200 lines Home Assistant automation YAML
- 11 user stories (3 P1, 6 P2, 2 P3)
- 40+ functional requirements

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### Pre-Research Gates

✅ **I. Hardware Abstraction First**: ESPHome YAML configuration replaces Arduino C++ code - all hardware interfaces declarative  
✅ **II. Home Assistant Integration Native**: ESPHome native API replaces Blynk cloud - local-first operation  
✅ **III. Configuration Over Code**: Settings migrated to Home Assistant entities - no EEPROM, no Blynk virtual pins  
✅ **IV. Functional Preservation**: 100% feature parity maintained - all original features accessible via Home Assistant  
✅ **V. Declarative Sensor & Display Management**: Sensors and displays defined in YAML - minimal custom C++ lambdas  

**Gate Status**: ✅ ALL GATES PASSED

### Post-Design Re-evaluation

✅ **I. Hardware Abstraction First**: All hardware interfaces defined in YAML (data-model.md confirms entities)  
✅ **II. Home Assistant Integration Native**: ESPHome native API used (contracts/README.md confirms protocol)  
✅ **III. Configuration Over Code**: All settings as Home Assistant entities (data-model.md confirms number/select entities)  
✅ **IV. Functional Preservation**: All user stories preserved (spec.md confirms 11 stories)  
✅ **V. Declarative Sensor & Display Management**: Sensors/display defined in YAML (data-model.md confirms sensor entities)  

**Gate Status**: ✅ ALL GATES PASSED (no violations introduced by design)

## Project Structure

### Documentation (this feature)

```text
specs/001-gazebo-stove-heating-control/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
gazebo_stove.yaml        # Main ESPHome configuration (single file)
includes/                # Modular ESPHome includes
├── automation.yaml      # Priority control script, thermostat logic
├── buttons.yaml         # Physical button handlers (if any)
├── core.yaml           # ESPHome core config (wifi, api, ota, logger)
├── display.yaml        # Nextion display integration
├── entities.yaml        # Climate, number, select, switch entities
├── hardware.yaml       # GPIO, relay, sensors, one-wire bus
└── sensors.yaml        # Temperature sensor configuration

homeassistant/           # Home Assistant configuration
├── automations.yaml    # Schedule automations, manual mode timeouts
├── configuration.yaml  # Template climate entity, input_datetime helpers
└── dashboard_card.yaml # UI card configuration

secrets.yaml            # WiFi credentials, API keys (gitignored)
```

**Structure Decision**: Single ESPHome YAML file with modular includes for maintainability. Home Assistant configuration separate for clarity. No custom C++ components required - all logic via ESPHome automations and Home Assistant automations.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

*No violations - all constitution principles satisfied*
