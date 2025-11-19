# Research: Gazebo Propane Stove Heating Control System

**Date**: 2025-01-27  
**Feature**: 001-gazebo-stove-heating-control

## Research Tasks

### R1: Humidity Sensor Deployment Status

**Question**: Original code mentions both DHT22 and DHT11 sensor in comments; which sensor is actually deployed?

**Decision**: NO humidity sensor deployed. DS18B20 is the only temperature sensor.

**Rationale**: 
- Hardware inventory confirms only DS18B20 OneWire sensor on GPIO4
- Code comments reference DHT sensors but they are not integrated in current hardware
- Perceived temperature feature (User Story 11) requires humidity data, so it cannot be implemented

**Alternatives considered**:
- Add DHT22 sensor: Rejected - hardware not available, adds complexity
- Estimate humidity: Rejected - inaccurate, not reliable for perceived temperature calculation

**Impact**: User Story 11 (Perceived Temperature) marked as P3 (low priority) and may be deferred or removed.

---

### R2: Weather API Location Configuration

**Question**: OpenWeatherMap integration - what location/coordinates to fetch forecast for?

**Decision**: Use OpenMeteo API (free, no key required) for Magog, Quebec, Canada (45.2617° N, -72.1393° W).

**Rationale**:
- OpenMeteo is free and doesn't require API key (simpler than OpenWeatherMap)
- Location confirmed: Magog, QC, Canada (gazebo location)
- Coordinates: 45.2617° N, -72.1393° W
- Timezone: America/Toronto (EST/EDT)

**Alternatives considered**:
- OpenWeatherMap: Requires API key, rate limits, more complex setup
- Hardcode location in ESPHome: Accepted - single device deployment, location doesn't change

**Impact**: Weather forecast integration (User Story 5) uses OpenMeteo API with hardcoded Magog coordinates.

---

### R3: IFTTT Integration Replacement

**Question**: IFTTT integration mentioned for Home/Away via location triggers - replace with Home Assistant presence detection or maintain IFTTT hooks?

**Decision**: Use Home Assistant native presence detection (device_tracker) - no IFTTT hooks needed.

**Rationale**:
- Home Assistant has built-in presence detection via device_tracker entities
- Eliminates external service dependency (IFTTT)
- Simpler automation logic (all in Home Assistant)
- Local-first approach aligns with constitution principle

**Alternatives considered**:
- Maintain IFTTT hooks: Rejected - adds external dependency, violates local-first principle
- Manual Home/Away toggle only: Accepted as fallback - user can manually set presence mode

**Impact**: Home/Away mode (User Story 2) uses Home Assistant device_tracker or manual select entity, no IFTTT integration required.

---

### R4: ESPHome Climate Platform vs Custom Thermostat Script

**Question**: ESPHome thermostat platform uses symmetric deadband (±H/2), but user expects asymmetric (±H). Should we use custom script or accept platform limitation?

**Decision**: Use ESPHome climate platform for UI compatibility, accept symmetric deadband limitation. Custom script exists but disabled to avoid conflicts.

**Rationale**:
- ESPHome climate platform provides native Home Assistant thermostat card UI
- Custom script would require template climate entity (more complex)
- Symmetric deadband (±H/2) is acceptable - still provides hysteresis control
- Priority script (`manage_heating`) handles edge cases (current > desired = turn off immediately)

**Alternatives considered**:
- Custom thermostat script with asymmetric hysteresis: Rejected - conflicts with climate platform, requires template entity
- Accept platform limitation: Accepted - simpler, better UI integration

**Impact**: Thermostat control (User Story 1) uses ESPHome climate platform with symmetric deadband. User-configurable hysteresis entity exists but platform uses its own internal calculation.

---

### R5: Priority Control Implementation Pattern

**Question**: How to implement priority-based control (Force Off → Emergency Heat → Schedule → Thermostat) in ESPHome?

**Decision**: Use ESPHome script (`manage_heating`) with explicit priority checks, called on state changes and periodic intervals.

**Rationale**:
- ESPHome scripts provide clean state management
- Explicit priority checks ensure safety (Force Off always wins)
- Periodic interval (10s) ensures edge cases handled (e.g., setpoint lowered below current temp)
- Home Assistant automations handle schedule timing (Priority 3)

**Alternatives considered**:
- Home Assistant automations only: Rejected - too much latency, requires constant API polling
- ESPHome automations only: Partially accepted - used for priority control, but schedule timing better in Home Assistant

**Impact**: Priority control logic implemented in `includes/automation.yaml` as `manage_heating` script. Schedule timing handled by Home Assistant automations.

---

## Summary

All research questions resolved. No blocking technical unknowns remain. Implementation can proceed with:
- DS18B20 sensor only (no humidity)
- OpenMeteo API for weather (Magog, QC coordinates)
- Home Assistant presence detection (no IFTTT)
- ESPHome climate platform (symmetric deadband accepted)
- Priority script pattern for control logic

