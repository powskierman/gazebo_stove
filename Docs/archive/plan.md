# GazeboWx ESPHome Conversion - Implementation Plan

**Date Created**: 2025-11-10
**Target Release**: 2025-12-31
**Status**: Planning Phase
**Related Documents**: CONSTITUTION.md, SPECIFICATION.md

---

## Phased Implementation Roadmap

### Phase 1: Foundation & Core Integration (Weeks 1-2)
*Goal: Establish ESPHome baseline with hardware connectivity and Home Assistant integration*

**1.1 Initial ESPHome Configuration**
- Create production `gazebo_stove.yaml` based on existing skeleton
- Configure ESP32 board, WiFi (ssid/password from secrets.yaml), API to Home Assistant
- Set up OTA updates via ESPHome dashboard
- Test device discovery and connection to Home Assistant

**1.2 GPIO & Hardware Mapping**
- Configure GPIO12 relay switch component (heating control)
- Configure GPIO4 OneWire bus for DS18B20 temperature sensor
- Configure GPIO16/17 UART for Nextion display communication (9600 baud)
- Add initial placeholder display component for Nextion
- Verify all GPIO pins respond correctly to test signals

**1.3 Sensor Integration**
- Implement Dallas temperature sensor with 10-second throttle_average filter
- Add temperature range validation and NaN detection
- Configure WiFi signal strength sensor (60-second update)
- Create raw temperature sensor entity for debugging

**Deliverables**:
- Working gazebo_stove.yaml with functional GPIO, sensors, and Home Assistant API
- Device visible in Home Assistant with temperature entity updating
- Relay control functional via Home Assistant switch

**Success Criteria**:
- Device connects to Home Assistant automatically
- Temperature readings display in Home Assistant within 2 seconds
- Relay can be toggled on/off from Home Assistant UI
- OTA updates available in ESPHome dashboard

---

### Phase 2: Thermostat & Heating Control (Weeks 3-4)
*Goal: Implement propane stove heating algorithm (heating-only, no cooling)*

**2.1 Climate Entity Setup**
- Create ESPHome climate component (thermostat)
- Link to DS18B20 sensor as current_temperature source
- Configure heat_deadband: 0.5°C (prevents oscillation)
- Configure heat_overrun: 0.5°C (overshoot margin)
- Set min_heating_run_time: 60s (prevent short-cycling)
- Set min_heating_off_time: 60s (compressor protection)
- Set min_idle_time: 60s (stability buffer)
- Link heat_action to GPIO12 relay (turn on when heating needed)
- Link idle_action to GPIO12 relay (turn off when idle)

**2.2 Mode Management**
- System operates in heating-only mode (propane stove cannot cool)
- Stove is inactive during summer season
- Home/Away mode controls heating permission (Away = always off)

**2.3 Heating Hysteresis Implementation**
- Create single number entity for Heating_Hysteresis (range 1-5°C)
- Implement heating algorithm:
  - Turn ON when temp < setpoint
  - Turn OFF when temp ≥ (setpoint + hysteresis)
- Store hysteresis value in Home Assistant database (persist across reboot)
- Default value: 2°C

**2.4 Temperature Offset & Calibration**
- Create number entity for TempCorrection (range -5 to +5°C, step 0.1°C)
- Apply offset in thermostat calculation: `adjusted_temp = raw_temp + offset`
- Display both raw and adjusted temperatures to Home Assistant
- Store offset in Home Assistant (persist across reboot)

**2.5 Perceived Temperature (Humidity Adjustment)**
- Create binary select entity for UsePerceivedTemp (on/off)
- In Summer mode only: if temp > 20°C AND humidity > 40%, apply humidity factor
- Humidity-adjusted increase: `(humidity - 40) / 10` degrees per 10% above 40%
- Example: temp=25°C, humidity=70% → perceived_temp = 25 + (70-40)/10 = 28°C
- Note: Original code shows DHT humidity reading but hardware uses DS18B20; may need to implement humidity estimation or leave disabled

**Deliverables**:
- Thermostat entity in Home Assistant with setpoint control
- Temperature curves logged showing hysteresis behavior
- Mode switching documented and verified

**Success Criteria**:
- Relay turns on when temp drops 1°C below setpoint in Winter mode
- Relay turns off when temp reaches (setpoint + hysteresis_winter)
- Setpoint can be changed in Home Assistant; relay responds within 10 seconds
- Mode switching (Winter/Summer) changes thermostat behavior immediately
- Calibration offset applies correctly (test: offset +2°C → displayed temp 2°C higher)

---

### Phase 3: Home/Away & Manual Modes (Weeks 5-6)
*Goal: Implement all override modes and presence-based control*

**3.1 Presence Mode (Home/Away)**
- Create binary select entity for Presence: ["home", "away"]
- Store presence mode in Home Assistant (persist across reboot)
- Default: "home" on startup
- Home Assistant integration: can set via UI or automation (e.g., based on device presence)

**3.2 Home/Away Logic Implementation**
- When Presence = "away": force relay OFF regardless of temperature or timer
- When Presence = "home": allow thermostat algorithm to control relay
- Visual feedback on Nextion display showing current presence state

**3.3 Manual Run Override**
- Create switch entity: "Manual_Run" (on/off)
- When enabled: relay turns ON immediately, thermostat algorithm disabled
- When enabled: start 15-minute countdown timer in Home Assistant
- Countdown: Store activation time, check every minute if 15 minutes elapsed
- After 15 minutes: automatically disable Manual_Run switch
- Visual feedback: "MANUAL MODE ACTIVE - XY minutes remaining"
- User can cancel anytime by flipping switch off

**Implementation approach**: Use Home Assistant automation with `binary_condition: time_delta` to track timeout
```
automation:
  - alias: "Manual Run Timeout"
    trigger:
      entity_id: switch.manual_run
      from: "on"
    condition: template
      value_template: '{{ (now() - states.switch.manual_run.last_changed).total_seconds() > 900 }}'
    action:
      service: switch.turn_off
      entity_id: switch.manual_run
```

**3.4 Manual Stop Override**
- Create switch entity: "Manual_Stop" (on/off)
- When enabled: relay turns OFF immediately, thermostat algorithm disabled
- When enabled: last indefinitely until user disables
- Home/Away mode takes priority over Manual Stop (Away always forces off)
- Visual feedback: "MANUAL STOP - System halted"

**3.5 Override Priority Logic**
- Priority order (highest to lowest):
  1. Away mode (always off)
  2. Manual Stop (off if enabled)
  3. Manual Run (on if enabled, with 15-min timeout)
  4. Thermostat algorithm (normal operation)
  5. Timer schedule (prevents operation outside window)

**Deliverables**:
- Home/Away and Manual mode switches in Home Assistant
- 15-minute timer implementation in Home Assistant automation
- Logic diagram showing priority order and state transitions

**Success Criteria**:
- Relay off when Away mode active (even if temp is low)
- Relay on immediately when Manual Run activated
- Manual Run timeout automatic after exactly 15 minutes
- Manual Stop halts heating until switched off
- User can override any mode without system restart

---

### Phase 4: Timer & Scheduling (Weeks 7-8)
*Goal: Implement time-based heating windows with IFTTT integration*

**4.1 NTP Time Synchronization**
- Configure SNTP time platform in gazebo_stove.yaml:
  ```yaml
  time:
    - platform: sntp
      id: sntp_time
      servers:
        - us.pool.ntp.org
        - time.nist.gov
      timezone: America/New_York
  ```
- Sync time on startup and every 6 hours
- Display current time on Nextion (HH:MM:SS format)

**4.2 Timer Schedule Setup**
- Create number entities:
  - BeginTimer: hour:minute format (0-23:0-59)
  - EndTimer: hour:minute format (0-23:0-59)
- Store times in Home Assistant database (persist across reboot)
- Default: BeginTimer=8:00, EndTimer=22:00 (example)

**4.3 Timer Evaluation Logic**
- Implement Home Assistant automation to evaluate timer every minute:
  ```
  automation:
    - alias: "Timer Schedule Check"
      trigger:
        platform: time_pattern
        minutes: '*'
      action:
        service: input_boolean.turn_on/off
        target:
          entity_id: binary_sensor.timer_active
        data:
          state: '{{ (now().hour * 60 + now().minute >= begin_hour * 60 + begin_min) and (now().hour * 60 + now().minute < end_hour * 60 + end_min) }}'
  ```
- Create binary_sensor entity: "TimerActive" (on=within schedule, off=outside schedule)

**4.4 Timer & Thermostat Integration**
- Thermostat only operates when (TimerActive == true AND Home == true AND Auto mode)
- Outside timer window: relay OFF regardless of temperature
- Visual feedback: Display timer status on Nextion ("TIMER ON 8:00-22:00" or "TIMER OFF")

**4.5 IFTTT Integration for Home/Away**
- Support existing IFTTT automation hooks:
  - URL: `http://device-ip:8123/api/services/switch/turn_on` (Home assistant service)
  - Trigger IFTTT location event → calls webhook to set Presence mode
  - Alternative: Use Home Assistant automation with mobile device_tracker (modern approach)

**Deliverables**:
- Timer begin/end time entities in Home Assistant
- Automation showing timer active/inactive status
- IFTTT integration documentation (or Home Assistant device_tracker alternative)

**Success Criteria**:
- Relay off when outside timer window (even in Home mode)
- Relay respects thermostat when inside timer window
- Timer times persist across power cycle
- Timer evaluation happens within 2 minutes of schedule boundary
- Current time displays correctly on Nextion

---

### Phase 5: Sensor Monitoring & Alarms (Weeks 9-10)
*Goal: Implement fault detection and user notifications*

**5.1 Sensor Malfunction Detection**
- Implement NaN check in temperature sensor filter
- Increment BadRead counter on each failed read (NaN)
- When BadRead >= 10:
  - Turn off relay (safety fallback)
  - Set alarm flag (binary_sensor entity)
  - Display "SENSOR MALFUNCTION" on Nextion
  - Log error in Home Assistant

**5.2 Alarm Clearing**
- Monitor BadRead counter
- When sensor returns valid reading: reset BadRead to 0
- Clear alarm flag automatically
- Restore normal thermostat operation

**5.3 User Notifications**
- Nextion display message: "<<< SENSOR MALFUNCTION >>>" (red background if possible)
- Home Assistant notification (if configured)
- Entity: binary_sensor.sensor_malfunction (on=alarm active)

**5.4 Other Sensor Monitoring**
- WiFi signal strength: log drops below -70dBm
- NTP sync status: alert if time is not synced after startup
- API connection: log Home Assistant connection drops/reconnects

**Deliverables**:
- BadRead counter logic in ESPHome
- binary_sensor.sensor_malfunction in Home Assistant
- Alarm message display on Nextion
- Event logging in Home Assistant

**Success Criteria**:
- Alarm triggers within 10 minutes of first bad read (10 reads at 10s interval = ~100s)
- Relay deactivates within 1 minute of alarm
- Alarm clears automatically when sensor recovers
- Home Assistant notification sent to admin

---

### Phase 6: Nextion Display Integration (Weeks 11-12)
*Goal: Implement full Nextion touchscreen UI mirroring original*

**6.1 Nextion Protocol Understanding**
- Examine original Nextion command structure from printCurrentWx.cpp
- Commands format: `.txt` for text, `.val` for numeric values, `.vis` for visibility
- Baud rate: 9600, 8N1 (no parity, 1 stop bit)
- Implement UART communication via ESPHome's nextion component

**6.2 Display Pages & Components**
- Page 0: Main dashboard
  - Current temperature (number field)
  - Desired temperature (number field)
  - Mode indicators (Winter/Summer, Home/Away, Manual status)
  - Relay status indicator (red=on, blue=off)
  - Timer status (on/off indicator, times)

- Page 1: Settings menu
  - Hysteresis adjustment (slider or +/- buttons)
  - Temperature offset (slider or +/- buttons)
  - Mode selection (buttons)

- Page 2: Weather forecast
  - Current weather icon and condition
  - 5-day forecast tiles with temps and icons
  - Humidity indicator

- Page 3: System status
  - WiFi SSID and signal strength
  - IP address
  - NTP sync status
  - Firmware version

**6.3 Lambda Updates**
- Implement lambda functions in gazebo_stove.yaml to push state changes to Nextion:
  ```yaml
  on_state:
    then:
      - lambda: |-
          id(gazebo_nextion).send_command_printf("three.n0.val=%d", int(id(gazebo_temp).state));
  ```

**6.4 Touch Event Handling**
- Map Nextion touch events to Home Assistant service calls
- Example: Long-press Settings button → trigger settings menu display
- Button debounce: 500ms to prevent multiple triggers

**6.5 Display Update Timing**
- Temperature/mode updates: every 2 seconds
- Weather forecast: every 30 minutes
- Time display: every 1 second
- Optimize bandwidth to prevent UART congestion

**Deliverables**:
- Nextion YAML configuration with lambda updates
- Display page layout specifications
- Touch event handling documentation

**Success Criteria**:
- All state changes reflected on Nextion within 2 seconds
- Nextion responds to touch input without lag
- No UART communication errors in logs
- Display survives 24+ hours of operation without corruption

---

### Phase 7: Weather Forecast & Remote Data (Weeks 13-14)
*Goal: Integrate OpenWeatherMap API and display forecast*

**7.1 OpenWeatherMap Integration**
- Verify API key is configured in secrets.yaml
- Current code already uses Bodmer/OpenWeather library
- Define weather location (latitude/longitude) - clarify in settings
- Fetch forecast on schedule: every 30 minutes
- Cache forecast data in case of API downtime

**7.2 Weather Data Flow**
- ESPHome polls OpenWeatherMap via HTTP request
- Parse JSON response (current conditions, 5-day forecast)
- Extract: temperature, condition, icon ID, humidity, wind
- Store in Home Assistant as weather entity
- Push forecast display data to Nextion

**7.3 Weather Display on Nextion**
- Current conditions: icon, temp, condition text, humidity
- 5-day forecast: daily high/low, icon, condition
- Update interval: 30 minutes (prevents API quota issues)
- Fallback: display last-known forecast if API unavailable

**7.4 Icon Mapping**
- Map OpenWeatherMap icon IDs to Nextion display icons (or text fallback)
- Example: sunny=sun icon, cloudy=cloud icon, rainy=rain icon
- Implement fallback to text if icon display unavailable

**Deliverables**:
- Weather API integration in gazebo_stove.yaml
- JSON parsing logic for forecast data
- Icon mapping table and Nextion display commands

**Success Criteria**:
- Weather updates appear on Nextion within 1 minute of API fetch
- API quota not exceeded (30-minute update interval = 48 calls/day, well under limits)
- Fallback forecast displays if API unreachable
- No UART errors from weather data transmission

---

### Phase 8: Configuration & Settings Persistence (Weeks 15-16)
*Goal: Migrate all EEPROM settings to Home Assistant database*

**8.1 Settings Inventory**
Migrate from original EEPROM layout:
- TempDes (desired temperature) → number entity
- Hysteresis_W (Winter) → number entity
- Hysteresis_S (Summer) → number entity
- TempCorrection (sensor offset) → number entity
- BeginTimer / EndTimer (schedule) → time entities
- Presence mode (Home/Away) → select entity
- Season mode (Winter/Summer) → select entity
- UsePerceivedTemp (humidity adjust) → binary select
- NormalWidgetColor (Blynk color pref) → color select (optional)

**8.2 Home Assistant Storage**
- Use Home Assistant `input_number`, `input_select`, `input_boolean` for user settings
- These entities automatically persist in Home Assistant database
- On device boot: query Home Assistant API for current settings
- On settings change in Home Assistant: device receives update via API

**8.3 Backup & Recovery**
- Home Assistant database backup (built-in)
- Export settings to JSON for manual backup
- Implement factory reset option (revert to defaults)

**8.4 Settings UI**
- Create Home Assistant dashboard card with all settings
- Include descriptions and valid ranges for each setting
- Manual test: change setting → verify device applies change

**Deliverables**:
- Complete inventory of all settings
- Home Assistant entities for each setting
- Settings dashboard in Home Assistant
- Backup/recovery documentation

**Success Criteria**:
- All settings persist across power cycle
- Settings changes apply to device within 2 seconds
- Home Assistant API availability required (graceful degradation if offline)
- Backup contains all configuration

---

### Phase 9: Testing & Validation (Weeks 17-18)
*Goal: Comprehensive end-to-end testing of all features*

**9.1 Hardware Connectivity Testing**
- Relay: Toggle on/off 100 times, verify no failures
- Temperature sensor: Compare readings with reference thermometer, verify ±0.5°C accuracy
- Nextion display: Send all command types, verify no corruption
- WiFi: Connect/disconnect 10 times, verify auto-reconnect
- OTA: Update firmware 3 times, verify settings persist

**9.2 Thermostat Algorithm Testing**
- Set various setpoints; verify relay activates/deactivates within 10s
- Test Winter hysteresis: hysteresis_w=3°C, setpoint=20°C
  - Turn on at 19°C, off at 23°C ✓
- Test Summer hysteresis: hysteresis_s=2°C, setpoint=25°C
  - Turn on at 26°C, off at 23°C ✓
- Test temperature rate limiting: change sensor 5°C in one read
  - Display shows max ±1°C change ✓
- Test Home/Away override: in Away mode, relay stays off regardless of temperature
- Test Manual Run timeout: enable, wait 15 min, verify automatic disable
- Test Manual Stop: verify relay stays off until disabled
- Test timer scheduling: verify relay off outside schedule window

**9.3 Resilience Testing**
- WiFi dropout: Unplug WiFi for 1 hour
  - Thermostat continues working ✓
  - After 1 hour, auto-reconnects ✓
  - All settings preserved ✓
- Power cycle: 10 consecutive power off/on cycles
  - Device boots normally ✓
  - All settings loaded ✓
  - Thermostat resumes operation ✓
- Sensor malfunction: Force 10 bad reads
  - Relay deactivates ✓
  - Alarm displays ✓
  - Clears when sensor recovers ✓

**9.4 User Interaction Testing**
- Home Assistant remote control: Change setpoint via web UI
  - Device receives update within 2s ✓
  - Nextion display updates ✓
  - Relay responds immediately ✓
- Mobile app remote control: Change setpoint via mobile
  - Same as above via mobile app ✓

**9.5 Performance & Stability**
- 72-hour continuous operation test
  - No memory leaks ✓
  - No UART corruption ✓
  - Thermostat maintains accuracy ✓
  - All settings preserved ✓

**9.6 Documentation Testing**
- Verify YAML configuration matches hardware reality
- Verify all Home Assistant entities are accessible and functional
- Create troubleshooting guide based on issues found

**Deliverables**:
- Test execution log with all test cases and results
- Issues found and resolution documentation
- Performance metrics (latency, accuracy, stability)
- Known limitations and workarounds

**Success Criteria**:
- 100% of test cases pass
- 72-hour operation test completes without errors
- All metrics within acceptable ranges
- Ready for production deployment

---

### Phase 10: Migration & Production Deployment (Weeks 19-20)
*Goal: Transition from Blynk to ESPHome with zero downtime*

**10.1 Parallel Operation Setup**
- Original Arduino/Blynk firmware continues running on backup device or during transition
- ESPHome device runs side-by-side for validation
- Both systems can control the same relay (mechanical interlock or control hierarchy)

**10.2 User Validation Period**
- Run ESPHome for 1-2 weeks parallel to Blynk
- Users test new Home Assistant UI
- Compare behavior with original Blynk implementation
- Document any differences and user feedback

**10.3 Cutover Checklist**
- [ ] All functionality verified equivalent to Blynk
- [ ] Users trained on Home Assistant UI
- [ ] Backup plan documented (revert to Blynk firmware)
- [ ] Support team ready to troubleshoot
- [ ] Firmware version tagged and documented
- [ ] Legacy Arduino code archived in git

**10.4 Deployment Steps**
1. Backup current device configuration (EEPROM equivalent)
2. Flash ESPHome firmware to device
3. Verify Home Assistant discovery and connection
4. Restore settings from backup
5. Test all features once more
6. Monitor for 24 hours for any issues
7. Remove Blynk code from production codebase
8. Document any deviations from original behavior

**10.5 Rollback Plan**
- If critical issues found: revert to last working Blynk firmware
- Device will show "Blynk mode" in display
- Users switch back to Blynk mobile app
- Schedule review of issues and fixes

**Deliverables**:
- Parallel operation documentation
- User training materials for Home Assistant UI
- Cutover checklist and deployment procedure
- Rollback procedure and risk assessment
- Production deployment report

**Success Criteria**:
- Zero thermal control disruption during cutover
- All users successfully switch to Home Assistant
- No significant issues in 24-hour post-deployment monitoring
- Rollback not required

---

## Detailed Feature Mapping: Arduino → ESPHome

### GPIO Pin Mapping
| Function | Arduino Pin | ESPHome GPIO | Type | Config |
|----------|------------|--------------|------|--------|
| Relay Control | 12 | GPIO12 | OUTPUT | switch platform gpio |
| OneWire Bus | 4 | GPIO4 | BIDIRECTIONAL | one_wire platform gpio |
| Nextion TX | 17 | GPIO17 | OUTPUT | uart tx_pin |
| Nextion RX | 16 | GPIO16 | INPUT | uart rx_pin |
| Reserved | 2 | GPIO2 | RESERVED | DHT sensor (if added) |

### Blynk Virtual Pin → Home Assistant Entity Mapping
| V Pin | Function | Arduino Variable | Home Assistant Entity | Type |
|-------|----------|------------------|----------------------|------|
| V0 | Current Temp | TempAct | sensor.gazebo_temperature | sensor |
| V1 | Desired Temp | TempDes | number.desired_temperature | number |
| V2 | Humidity | Humidity | sensor.humidity_estimate | sensor (calc) |
| V3 | Mode Indicator | MenuItem | select.operation_mode | select |
| V4 | Hysteresis S | Hysteresis_S | number.summer_hysteresis | number |
| V6 | Manual/Echo | ManualRun | switch.manual_run | switch |
| V10 | Menu/Status | Response | text_sensor.status_message | text_sensor |
| V29 | Home LED | Home | select.presence_mode | select |
| V31 | IFTTT Home | Home | select.presence_mode | select |
| (various) | Dashboard colors | NormalWidgetColor | (removed - HA has native styling) | - |

### EEPROM Settings → Home Assistant Entity Mapping
| EEPROM Offset | Variable | Home Assistant Entity | Type | Default |
|---|---|---|---|---|
| 3 | TempDes | number.desired_temperature | number | 22°C |
| 10 | BeginTimer | input_datetime.timer_begin | datetime | 08:00 |
| 16 | EndTimer | input_datetime.timer_end | datetime | 22:00 |
| (various) | Hysteresis_W | number.winter_hysteresis | number | 3°C |
| (various) | Hysteresis_S | number.summer_hysteresis | number | 2°C |
| (various) | TempCorrection | number.temperature_offset | number | 0°C |
| (derived) | UsePerceivedTemp | input_boolean.perceived_temp | boolean | false |

### Algorithm & Logic Mapping
| Arduino Implementation | ESPHome Equivalent |
|---|---|
| BlynkTimer loop | ESPHome climate component + automations |
| EEPROM.write() / EEPROM.read() | Home Assistant input_* entities (persisted) |
| BlynkWifi connections | ESPHome native API to Home Assistant |
| Manual mode 15-min timeout | Home Assistant automation with time_delta |
| Thermostat hysteresis logic | ESPHome climate component (heat_deadband, heat_overrun) |
| WiFiManager fallback AP | ESPHome WiFi fallback_ap |
| ArduinoOTA | ESPHome OTA platform |
| NTP time sync | ESPHome SNTP platform |
| Temperature averaging | ESPHome sensor filter: throttle_average |

### Code Structure Mapping
```
Arduino PlatformIO Project
├── src/main.cpp                 → gazebo_stove.yaml (main config)
├── src/thermostat.cpp           → climate component + automations
├── src/blynkModule.cpp          → (removed - replaced by Home Assistant)
├── src/ds18b20.cpp              → one_wire + dallas_temp components
├── src/settings.h               → input_* entities in Home Assistant
├── src/eepromPresets.cpp        → Home Assistant database
├── src/printCurrentWx.cpp       → display + weather components
├── src/ap.h / ap.cpp            → wifi fallback_ap
├── src/ntpServer.cpp            → time SNTP component
├── src/nh_ota.cpp               → ota component
└── src/myTime.cpp               → (removed - ESPHome time component used)

ESPHome Project
└── gazebo_stove.yaml              (single source of truth for hardware config)
    ├── esphome:                (device definition)
    ├── esp32:                  (board config)
    ├── wifi:                   (network + fallback)
    ├── api:                    (Home Assistant integration)
    ├── ota:                    (firmware updates)
    ├── time:                   (NTP sync)
    ├── one_wire:               (sensor bus)
    ├── dallas_temp:            (temperature sensor)
    ├── uart:                   (Nextion serial)
    ├── display:                (Nextion updates)
    ├── switch:                 (relay control)
    ├── sensor:                 (temperature, WiFi signal)
    ├── climate:                (thermostat algorithm)
    ├── number:                 (setpoint, hysteresis, offset)
    ├── select:                 (mode, season, presence)
    ├── binary_sensor:          (alarms, sensor malfunction)
    ├── text_sensor:            (status messages)
    └── scripts:                (manual mode timeout, timer evaluation)
```

---

## Technology Decisions & Justifications

### ESPHome vs. Arduino/PlatformIO
- **Decision**: Migrate to ESPHome
- **Justification**:
  - Eliminates proprietary Blynk cloud dependency
  - Native Home Assistant integration reduces external services
  - YAML configuration reduces need for recompilation on settings changes
  - Large component library reduces custom code (thermostat, climate, SNTP built-in)
  - Community support for IoT and smart home use cases

### Home Assistant vs. Blynk
- **Decision**: Replace Blynk with Home Assistant
- **Justification**:
  - Local-first operation (no cloud dependency)
  - User owns data; no vendor lock-in
  - Integrates with other home automation systems (Z-Wave, Zigbee, etc.)
  - Richer UI and automation options
  - Free, open-source alternative

### Climate Component vs. Custom Thermostat Logic
- **Decision**: Use ESPHome native climate component
- **Justification**:
  - Built-in hysteresis and deadband support
  - Reduced custom code = fewer bugs
  - Familiar interface to Home Assistant users
  - Supports both heating and cooling modes natively

### YAML Configuration vs. C++
- **Decision**: Maximize YAML; minimize custom C++ lambdas
- **Justification**:
  - No recompilation needed for settings changes
  - Easier for non-developers to understand
  - Consistent with ESPHome philosophy
  - Custom lambdas only for complex display logic

---

## Known Constraints & Workarounds

### Constraint: No Built-in Perceived Temperature in Climate
- **Workaround**: Implement as template sensor or Home Assistant automation
- **Status**: Deferred to Phase 3 or later if humidity sensor added

### Constraint: Nextion Display Protocol Differs from Original
- **Workaround**: Verify UART communication in Phase 1; may need custom lambda functions
- **Status**: Early testing required (Week 1)

### Constraint: OpenWeatherMap Forecast Coordinates Hardcoded
- **Workaround**: Parameterize location in secrets.yaml or Home Assistant configuration
- **Status**: Implement in Phase 7

### Constraint: Original Uses Both DHT Humidity and DS18B20 Temperature
- **Workaround**: Clarify which sensor is actually deployed; current YAML only supports DS18B20
- **Status**: Clarify before implementation; may add DHT later

---

## Risk Mitigation Strategies

| Risk | Likelihood | Severity | Mitigation |
|------|-----------|----------|-----------|
| Nextion UART incompatibility | Medium | High | Test UART early (Week 1); have fallback to no display |
| Home Assistant unavailability | Low | Medium | Store last-known settings locally; operate standalone if needed |
| WiFi connection drops during critical heating | Low | High | Implement watchdog timer; verify relay defaults to safe state |
| Settings not persisting | Low | High | Test persist across 10 power cycles early |
| Temperature control overshoots | Low | Medium | Tune deadband and rate limiting during testing |
| Perceived temperature calculation incorrect | Low | Low | Disable feature if issues found; not critical for heating |

---

## Success Criteria & Completion Checklist

### Phase Completion Criteria
- ✓ Phase 1: Device connects to Home Assistant; temperature reads correctly
- [ ] Phase 2: Heating control works; hysteresis tuned
- [ ] Phase 3: Home/Away and Manual modes implemented
- [ ] Phase 4: Timer schedule working; NTP synced
- [ ] Phase 5: Sensor alarm functional; detection within 10 min
- [ ] Phase 6: Nextion display updates all values correctly
- [ ] Phase 7: Weather forecast displays; no API quota issues
- [ ] Phase 8: All settings persist across reboot
- [ ] Phase 9: All tests pass; 72-hour stability test complete
- [ ] Phase 10: Production deployment successful; rollback plan ready

### Final Sign-Off Criteria
- [ ] 100% feature parity with original Blynk implementation
- [ ] All Home Assistant entities functional
- [ ] Nextion display operational
- [ ] WiFi resilience tested (1-hour dropouts)
- [ ] OTA updates working
- [ ] Settings persistence verified (10 power cycles)
- [ ] Documentation complete and tested
- [ ] User training materials ready
- [ ] Rollback procedure documented
- [ ] Support team trained on ESPHome debugging

---

## Timeline & Resource Allocation

**Total Duration**: 20 weeks (5 months)
**Team**: 1 firmware developer (primary), 1 QA/tester (part-time)

### Milestone Schedule
- **Week 2**: Phase 1 complete (core hardware functional)
- **Week 4**: Phase 2 complete (thermostat working)
- **Week 6**: Phase 3 complete (all manual modes)
- **Week 8**: Phase 4 complete (scheduling)
- **Week 10**: Phase 5 complete (sensor monitoring)
- **Week 12**: Phase 6 complete (display functional)
- **Week 14**: Phase 7 complete (weather integration)
- **Week 16**: Phase 8 complete (settings persistence)
- **Week 18**: Phase 9 complete (all testing done)
- **Week 20**: Phase 10 complete (production deployment)

### Buffer & Contingency
- 2-week buffer for issues and rework
- 1-week validation period before production
- 1-week rollback contingency if critical issues found

---

## Appendix: Example Configuration Snippets

### Basic gazebo_stove.yaml Structure
```yaml
esphome:
  name: gazebo-wx
  friendly_name: Gazebo Weather & Heating

esp32:
  board: wemos_d1_mini32

logger:

api:

ota:
  - platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "Gazebo-Wx Fallback"
    password: "gazebo1234"

# ... (rest of components per phase)
```

### Climate Entity Configuration (Phase 2)
```yaml
climate:
  - platform: thermostat
    name: Gazebo Thermostat
    sensor: gazebo_temp
    heat_deadband: 0.5
    heat_overrun: 0.5
    min_heating_off_time: 60s
    min_heating_run_time: 60s
    min_idle_time: 60s
    heat_action:
      - switch.turn_on: relay
    idle_action:
      - switch.turn_off: relay
```

### Manual Mode Timeout Automation (Home Assistant)
```yaml
automation:
  - alias: Manual Run Timeout
    trigger:
      platform: time_pattern
      minutes: /1  # Check every minute
    condition:
      condition: state
      entity_id: switch.manual_run
      state: "on"
    action:
      - condition: template
        value_template: '{{ (now() - states.switch.manual_run.last_changed).total_seconds() > 900 }}'
      - service: switch.turn_off
        target:
          entity_id: switch.manual_run
```

---

## Version Control & Documentation References

**Document Version**: 1.0.0
**Date**: 2025-11-10
**Author**: Code Analysis
**Status**: Planning Phase (Ready for implementation review)

**Related Documents**:
- `CONSTITUTION.md` - Core principles and governance
- `SPECIFICATION.md` - Detailed feature requirements
- `GAZEBO_WX.YAML` - Main configuration (to be created)
- `ARCHITECTURE.md` - System design (to be created during Phase 1)

**Next Steps**:
1. Review and approve CONSTITUTION.md
2. Review and approve SPECIFICATION.md
3. Review and approve PLAN.md
4. Obtain stakeholder sign-off on architecture
5. Begin Phase 1 implementation (Week 1)
