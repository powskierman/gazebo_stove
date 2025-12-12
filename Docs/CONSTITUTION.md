# GazeboWx ESPHome Conversion Constitution

## Core Principles

### I. Hardware Abstraction First
Replace Arduino/Blynk hardware integration with ESPHome's abstraction layer. All hardware interfaces (relays, sensors, displays) must be defined declaratively in YAML configuration rather than imperative C++ code. This ensures hardware changes require only configuration updates, not code recompilation.

### II. Home Assistant Integration Native
Replace Blynk cloud dependency with Home Assistant integration via ESPHome's native API. All user interactions, remote control, and data visualization must flow through Home Assistant rather than a proprietary cloud service. This ensures local-first operation and reduces external service dependencies.

### III. Configuration Over Code
Migrate all user-configurable settings from EEPROM and Blynk virtual pins to Home Assistant entities and ESPHome YAML configuration. Settings include temperature setpoints, hysteresis values, timer schedules, and sensor calibration. Users interact through Home Assistant UI rather than mobile app.

### IV. Functional Preservation
Maintain 100% feature parity with the original Blynk implementation:
- Thermostat control with independent hysteresis
- Temperature scheduling and Home/Away mode
- Manual run/stop overrides with timeout
- Real-time sensor monitoring with fault detection
- Perceived temperature adjustment for humidity conditions
- Weather display integration via Nextion touchscreen
- OTA firmware updates
- WiFi auto-reconnect with fallback AP

### V. Declarative Sensor & Display Management
Define all sensors (DS18B20 temperature, WiFi signal) and display outputs (Nextion screen, climate entity) declaratively in YAML. Avoid custom C++ lambdas; use ESPHome's built-in components and automations instead.

### Hardware Compatibility
- Target board: Custom board with ESP32 WROOM
- Preserved GPIO assignments:
  - GPIO12: Relay control (heating)
  - GPIO4: OneWire bus (DS18B20 temperature sensor)
  - GPIO16/17: UART Nextion display (9600 baud)
  - GPIO2: Reserved for future DHT sensor
- OneWire/Dallas Temperature sensor library replacement with ESPHome's native dallas_temp component

### Sensor Requirements
- DS18B20 temperature sensor: 10-second averaging filter (throttle_average)
- Nextion display: 9600 baud UART, command-based updates
- WiFi signal monitoring: 60-second update interval
- Temperature readings: ±1°C per update (rate-limited for stability)

### Climate Control Logic
- Heat action: GPIO12 relay ON (heating mode activation)
- Idle action: GPIO12 relay OFF (heating deactivation)
- Thermostat component with configurable deadband (0.5°C default)
- Min heating run time: 60 seconds (prevents short-cycling)
- Min heating off time: 60 seconds (compressor protection)
- Min idle time: 60 seconds (stability)

## Development Workflow

### Code Organization
- Main configuration: `gazebo_stove.yaml` (single source of truth)
- Secrets management: `secrets.yaml` (WiFi, API keys, timestamps)
- No custom C++ components required; use ESPHome's component library
- All logic expressed as automations or template sensors

### Validation Gates
1. **Hardware Connectivity**: All GPIO pins mapped correctly, sensor reads verified
2. **Display Integration**: Nextion screen updates via UART, no corruption
3. **Climate Logic**: Thermostat state transitions match original behavior
4. **Home Assistant Sync**: All entities expose correctly, state persistence works
5. **Feature Parity**: All original features accessible through Home Assistant UI

### Testing Strategy
- Manual verification of each GPIO function
- UART communication logging for Nextion debug
- Home Assistant state history validation
- Thermostat mode transitions under various setpoint conditions
- WiFi reconnection and fallback AP behavior
- OTA update success without data loss

## Governance

### Constitution Supersedes All Other Practices
- Any deviations from these core principles require documented justification
- Hardware abstraction is non-negotiable; no imperative GPIO manipulation
- All features must be accessible through Home Assistant

### Migration Approval Process
1. Feature functional equivalence documented
2. All validation gates passed
3. Home Assistant integration tested end-to-end
4. Rollback plan documented if issues found
5. Original Arduino code retained in /Users/michel/Documents/Electronics/Projects/Gazebo/GazeboWx 

### Version Management
- Semantic versioning: MAJOR.MINOR.PATCH
- MAJOR: Hardware changes, new sensors, structural refactors
- MINOR: New features, component additions, new automations
- PATCH: Bug fixes, configuration tuning, calibration updates

**Version**: 1.0.0 | **Ratified**: 2025-11-10 | **Status**: Active Migration Phase
