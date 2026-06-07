# Gazebo Thermostat ESPHome Implementation Status

Date reviewed: 2026-06-06

Source of truth reviewed:

- Active ESPHome configuration: `gazebo-thermostat.yaml`
- Project documentation: `/Users/michel/Documents/Electronics/Projects/GazeboStove/Docs`
- Validation command: `./venv/bin/esphome config gazebo-thermostat.yaml`

## Executive Summary

The active ESPHome file, `gazebo-thermostat.yaml`, implements a valid but minimal ESPHome thermostat skeleton. It has working network configuration, Home Assistant API exposure, OTA, web server, SNTP time, UART pins for the Nextion display, a GPIO relay on GPIO12, a template temperature sensor, a temperature correction number entity, diagnostics, and an ESPHome thermostat climate entity that can switch the relay on and off.

It does not yet implement most of the documented Gazebo Stove feature set. The largest gap is that the active configuration uses a synthetic template temperature of `20.0 + temperature_correction`; it does not read the DS18B20 sensor on GPIO4. Because the thermostat is driven by this synthetic sensor, automatic heating behavior can compile and expose correctly, but it is not yet controlling from real gazebo temperature.

There is also a documentation/workspace mismatch. Several newer docs describe a modular ESPHome implementation with files such as `includes/core.yaml`, `includes/hardware.yaml`, `includes/sensors.yaml`, `includes/entities.yaml`, `includes/automation.yaml`, PID climate migration, autonomous scheduling, and priority logic. Those modules are not present in the active `/Users/michel/Documents/Esphome` workspace, and the active `gazebo-thermostat.yaml` does not contain that logic. This report therefore treats `gazebo-thermostat.yaml` as the implemented system and treats the modular/PID/schedule docs as design or historical implementation notes that have not been integrated into the active file.

## Current Active Configuration Health

ESPHome validation result:

- Status: valid
- ESPHome version used: 2026.5.2
- Warning: GPIO12 is a strapping pin and should be used with care. External pullups or pulldowns on that relay circuit can cause boot failures.

No compile or upload was performed during this review.

## What Is Implemented In `gazebo-thermostat.yaml`

### Core ESPHome Device

Implemented:

- Device name: `gazebo-thermostat`
- Board: `esp32dev`
- Framework: ESP-IDF recommended version
- Boot automation logs startup and updates the temperature component after 2 seconds

Status:

- Implemented and config-valid.

Notes:

- Docs sometimes refer to `gazebo_stove.yaml`, `gazebo-stove`, or `esp32doit-devkit-v1`. The active file uses `gazebo-thermostat.yaml`, `gazebo-thermostat`, and `esp32dev`.

### WiFi, Fallback AP, Captive Portal

Implemented:

- WiFi credentials through `!secret wifi_ssid` and `!secret wifi_password`
- Static IP: `192.168.0.242`
- Gateway: `192.168.0.1`
- Fallback AP: `Gazebo-Thermostat-Fallback`
- Fallback AP password through `!secret ap_password`
- Captive portal enabled
- WiFi power save disabled
- Fast connect enabled

Status:

- Implemented and config-valid.

Gap versus docs:

- Some docs specify fallback AP name `Gazebo-Wx Fallback`.
- Some docs specify `192.168.2.x` network examples.
- Constitution requires WiFi auto-reconnect with fallback AP; fallback AP exists, but resilience behavior has not been tested.

### Home Assistant Integration

Implemented:

- Native ESPHome API enabled.
- `reboot_timeout: 0s`, so the device should not reboot solely because Home Assistant is unavailable.

Status:

- Implemented and config-valid.

Gap versus docs:

- The docs expect all user modes and settings to be exposed through Home Assistant. Only the climate entity, relay switch, temperature correction number, diagnostic sensors, and status entities are currently exposed.
- No presence mode, manual override mode, schedule controls, hysteresis control, or sensor malfunction entity exists in the active file.

### OTA Updates

Implemented:

- ESPHome OTA enabled with `!secret ota_password`.
- Web server is also enabled, and ESPHome validation expands this to include the web server OTA platform.

Status:

- Implemented and config-valid.

Gap versus docs:

- OTA success has not been tested.
- Settings persistence after OTA has not been tested.

### Web Server

Implemented:

- ESPHome web server on port 80.

Status:

- Implemented and config-valid.

Notes:

- This is useful for diagnostics and local access, though it is not a major feature in the original Blynk mapping.

### Time Synchronization

Implemented:

- SNTP time source.
- Timezone configured as `America/New_York`.
- Server: `us.pool.ntp.org`.

Status:

- Implemented and config-valid.

Gap versus docs:

- Weather docs specify Magog, Quebec and `America/Toronto`.
- Constitution and schedule docs require time-driven schedule behavior; time sync exists, but no schedule logic uses it.
- No time display is pushed to Nextion.

### UART For Nextion Hardware

Implemented:

- UART TX pin: GPIO17
- UART RX pin: GPIO16
- Baud rate: 9600

Status:

- Hardware communication channel is declared and config-valid.

Gap versus docs:

- No `display:` / `platform: nextion` component exists.
- No Nextion component ID exists.
- No commands update display fields.
- No touch events or slider handling are implemented.
- No page updates, status button updates, icon updates, or display initialization script exists.

### Temperature Sensor

Implemented:

- A template sensor named `Gazebo Temperature`.
- Entity ID in YAML: `gazebo_temp`.
- Update interval: 10 seconds.
- Reported value: `20.0 + id(temp_correction).state`.
- Temperature value is logged on update.
- An interval also explicitly updates this component every 10 seconds.

Status:

- Implemented and config-valid as a synthetic test sensor.

Critical gap:

- This is not the DS18B20. The documented hardware sensor on GPIO4 is not configured.

Gap versus docs:

- No `one_wire:` bus on GPIO4.
- No `dallas_temp:` sensor.
- No DS18B20 address.
- No `throttle_average: 10s` filter on real readings.
- No clamp filter.
- No delta/rate-limiting filter.
- No bad read counter.
- No sensor malfunction alarm.
- No relay safety shutdown on sensor failure.

Impact:

- The thermostat currently controls against a constant synthetic temperature. Automatic heating cannot reflect real gazebo conditions until the DS18B20 is restored.

### Temperature Correction

Implemented:

- Template number named `Temperature Correction`.
- ID: `temp_correction`.
- Range: -10 to +10 deg C.
- Step: 0.1 deg C.
- Restore value enabled.
- Optimistic mode enabled.

Status:

- Implemented and config-valid.

Gap versus docs:

- Docs usually specify -5 to +5 deg C.
- Correction is applied only to the synthetic temperature. It is ready conceptually, but not connected to a real DS18B20 reading.

### WiFi And Uptime Diagnostics

Implemented:

- WiFi signal sensor, 60 second interval.
- Uptime sensor.
- Status binary sensor.
- WiFi IP address text sensor.
- WiFi SSID text sensor.

Status:

- Implemented and config-valid.

Gap versus docs:

- No BSSID or MAC address text sensor, although newer modular docs mention them.
- No low WiFi signal alert.
- No API connection status handling beyond ESPHome defaults.

### Relay Control

Implemented:

- GPIO switch on GPIO12.
- Name: `Gazebo Relay`.
- ID: `gazebo_relay`.
- Restore mode: `RESTORE_DEFAULT_OFF`.
- Logs relay on/off transitions.

Status:

- Implemented and config-valid.

Important warning:

- ESPHome reports GPIO12 is an ESP32 strapping pin. The relay circuit must not pull the pin into a boot-invalid state.

Gap versus docs:

- The docs expect relay control to be governed by priority logic: Force Off, Emergency Heat, Schedule, Thermostat.
- The active file lets the climate entity and direct Home Assistant switch control both operate on the same relay. There is no priority manager or safety interlock.

### Climate / Thermostat Control

Implemented:

- ESPHome `climate: platform: thermostat`.
- Name: `Gazebo Thermostat`.
- Sensor: `gazebo_temp`.
- Visual range: 10 to 35 deg C, step 0.5 deg C.
- Heat deadband: 0.5 deg C.
- Heat overrun: 0.5 deg C.
- Heat action turns on `gazebo_relay`.
- Idle action turns off `gazebo_relay`.
- Home preset with low target temperature 20 deg C.

Status:

- Implemented and config-valid.

Major gaps versus docs:

- Minimum heating off/run/idle times are all 1 second, while the Constitution and plan require 60 seconds.
- No separate configurable heating hysteresis entity.
- No Home/Away gating.
- No manual run or manual stop priority.
- No schedule/timer gating.
- No summer/inactive mode.
- No sensor fault gating.
- It uses the synthetic template temperature, not real DS18B20 data.

Documentation mismatch:

- `THERMOSTAT_PID_MIGRATION.md` says the project migrated to `climate: platform: pid` and hardware output sync logic. The active file uses `climate: platform: thermostat`, not PID, and has no template output safeguard.

### HTTP Request Component

Implemented:

- `http_request:` base configuration with user agent and timeout.

Status:

- Declared and config-valid.

Gap versus docs:

- No weather API sensors use it.
- No OpenMeteo endpoint is configured.
- No JSON parsing.
- No forecast cache.
- No Nextion weather updates.

## Feature-by-Feature Status

| Feature | Required By Docs | Active YAML Status | Assessment |
|---|---|---|---|
| ESPHome device baseline | Phase 1, Constitution | Implemented | Present and valid |
| WiFi with secrets | Phase 1 | Implemented | Present |
| Fallback AP | Phase 1, FR-026 | Implemented | Present, different SSID than some docs |
| Captive portal | Phase 1 | Implemented | Present |
| Home Assistant native API | Constitution, FR-025 | Implemented | Present |
| OTA | FR-027 | Implemented | Present, not hardware-tested |
| Web server | Useful diagnostic | Implemented | Present |
| SNTP time | FR-031 | Implemented | Present, not used by schedule |
| GPIO12 relay | FR-035 | Implemented | Present, GPIO12 strapping warning |
| Relay safe startup | Constitution | Implemented | `RESTORE_DEFAULT_OFF` present |
| UART GPIO16/17 at 9600 | FR-037 | Partially implemented | UART exists, no Nextion component |
| DS18B20 on GPIO4 | FR-036 | Not implemented | Critical missing hardware sensor |
| OneWire bus | Phase 1 | Not implemented | Missing |
| Real temperature filtering | FR-011, FR-005 | Not implemented | Synthetic value only |
| Temperature correction | FR-012 | Partially implemented | Number exists, applied to synthetic sensor |
| Thermostat climate entity | FR-001 | Partially implemented | Exists, but driven by synthetic sensor |
| 60 second anti-short-cycle timers | Constitution | Not implemented | Active file uses 1 second timers |
| Configurable heating hysteresis | FR-004 | Not implemented | Only fixed deadband/overrun |
| Home/Away mode | FR-002 | Not implemented | No select or logic |
| Schedule/timer mode | FR-003, FR-032, FR-033 | Not implemented | No schedule entities or interval logic |
| Manual Run | FR-007, FR-009 | Not implemented | No switch, no timeout |
| Manual Stop / Force Off | FR-008 | Not implemented | No switch, no priority |
| Priority hierarchy | Reference priority docs | Not implemented | No manager script/output safeguard |
| Sensor malfunction detection | FR-013 | Not implemented | No bad read counter/alarm |
| Relay off on sensor failure | FR-013 | Not implemented | No safety fallback |
| Nextion display updates | FR-016, FR-019, FR-020 | Not implemented | UART only |
| Nextion touch slider | Display guide | Not implemented | No Nextion component/events |
| Weather forecast | FR-017 | Not implemented | HTTP component only |
| OpenMeteo Magog endpoints | Weather docs | Not implemented | No resources/sensors |
| Perceived temperature/humidity | Historical docs | Not implemented | Later spec says not applicable |
| DHT humidity sensor | Clarification item | Not implemented | Hardware unclear |
| Home Assistant full settings persistence | FR-021, FR-022 | Partially implemented | Temp correction restores; climate target likely handled by climate, but most settings missing |
| WiFi offline operation | FR-028 | Partially implemented | API reboot disabled; thermostat can run locally, but only against synthetic temp and missing cached modes |
| Testing/validation suite | Phase 9 | Not implemented | No test log found in active workspace |
| Production migration/rollback | Phase 10 | Not implemented in active workspace | Docs describe need, not active implementation |

## Documentation Inconsistencies And Their Meaning

### File Naming And Architecture

The docs refer to multiple architectures:

- `gazebo_stove.yaml` as a modular entry point.
- `includes/*.yaml` modules totaling about 517 lines.
- `gazebo-thermostat.yaml` as the latest active file in the ESPHome workspace.

The active workspace contains `gazebo-thermostat.yaml` and does not contain the documented `includes` module tree. Therefore the modular architecture is not currently active here.

### Climate Platform

The docs contain three different climate/control approaches:

- ESPHome `thermostat` climate platform.
- Custom `manage_heating` script with hysteresis.
- PID climate platform with template output safeguard.

The active file uses ESPHome `thermostat`. It does not use PID and does not use a `manage_heating` priority script.

### Schedule Status

`SCHEDULE_IMPLEMENTATION.md` and `Docs/reference/scheduling.md` say autonomous scheduling was implemented in ESPHome with datetime entities and 60 second evaluation. The active file has SNTP but no datetime entities, no schedule enable switch, no presence mode select, and no schedule interval logic.

### Nextion Status

The newer Nextion reference doc claims Page 0 and Page 3 are operational or ready for testing. The active file only declares UART. It has no Nextion display component or commands, so from the active YAML perspective Nextion integration is not implemented.

### Weather API

The plan mentions OpenWeatherMap in places, while newer docs use OpenMeteo for Magog, Quebec. The active file has neither. It only declares `http_request`.

### Perceived Temperature

Early docs list perceived temperature/humidity adjustment as a parity feature. The later specification says perceived temperature and humidity adjustment must not be used for this heating-only stove. The safer current interpretation is: do not implement perceived temperature unless the requirement is explicitly reopened.

## What Has Yet To Be Done

### 1. Restore Real Temperature Hardware

Priority: critical

Tasks:

- Add `one_wire:` bus on GPIO4.
- Add `sensor: platform: dallas_temp` using the real DS18B20 address.
- Apply the temperature correction to the DS18B20 reading.
- Add `throttle_average: 10s`.
- Decide whether to add `delta: 1.0`; note that a strict delta filter can delay control updates unless periodic control evaluation exists.
- Remove or rename the synthetic template temperature so it is not mistaken for real hardware.

Acceptance:

- Home Assistant shows actual gazebo temperature.
- Temperature changes track a reference thermometer.
- Climate entity uses the real corrected sensor.

### 2. Fix Thermostat Safety Timing

Priority: high

Tasks:

- Change `min_heating_off_time`, `min_heating_run_time`, and `min_idle_time` from 1 second to the documented 60 seconds unless hardware testing proves another value is required.
- Confirm the relay module behavior on GPIO12 at boot, given strapping-pin warning.
- Consider moving relay drive to a non-strapping GPIO if hardware allows.

Acceptance:

- Relay does not short-cycle.
- ESP32 boots reliably with relay hardware connected.

### 3. Decide Final Control Architecture

Priority: high

Current conflict:

- Active YAML: ESPHome thermostat platform.
- Later docs: PID platform with output-level safety logic.
- Modular docs: custom `manage_heating` script.

Recommendation:

- Pick one architecture before adding more features.
- For a propane stove with simple on/off relay, either ESPHome `thermostat` with explicit gating or a custom `manage_heating` script can be simpler than PID.
- If retaining direct Home Assistant relay switch access, add a priority layer so manual relay toggles cannot leave the climate state and hardware state inconsistent.

Acceptance:

- One documented control path owns relay decisions.
- Home Assistant status matches physical relay state.
- Manual and schedule states cannot fight the thermostat.

### 4. Implement Manual Overrides

Priority: high

Tasks:

- Add Manual Run / Emergency Heat control.
- Add Manual Stop / Force Off control.
- Add 15 minute timeout for Manual Run.
- Ensure Force Off has highest priority.
- Ensure Manual Run cannot override Force Off.
- Add clear Home Assistant entity names and logs.

Acceptance:

- Force Off immediately turns relay off and keeps it off.
- Emergency Heat turns relay on for 15 minutes unless Force Off is active.
- Timeout works without Home Assistant connectivity if the design goal remains autonomous operation.

### 5. Implement Home/Away And Schedule

Priority: high

Tasks:

- Add a `presence_mode` select or equivalent.
- Add schedule enable/disable.
- Add start/end time controls.
- Evaluate schedule continuously, not only at exact trigger times.
- Handle schedules crossing midnight.
- Decide timezone: docs for Magog imply `America/Toronto`; active file uses `America/New_York`, which is equivalent for most DST behavior but less precise.

Acceptance:

- Away mode prevents heating.
- Home mode allows thermostat operation.
- Schedule corrects mode after boot and after time sync.
- Schedule persists across reboot.

### 6. Implement Sensor Fault Detection

Priority: high

Tasks:

- Detect invalid DS18B20 readings.
- Count consecutive bad reads.
- Shut off relay after documented threshold.
- Expose `sensor_malfunction` binary sensor.
- Clear alarm after valid recovery.
- Push alarm status to Home Assistant and eventually Nextion.

Acceptance:

- Disconnecting or faulting the sensor turns the relay off.
- Alarm state is visible.
- Alarm clears safely when readings return.

### 7. Implement Nextion Component And Display Updates

Priority: medium

Tasks:

- Add `display: platform: nextion` attached to the existing UART.
- Add initialization script.
- Update Page 0 with current temperature, relay/heating status, and status text.
- Update setpoint display.
- Add touch handling for the slider page if Nextion touch events are needed.
- Add display update throttling to avoid UART congestion.

Acceptance:

- Display updates current temperature and setpoint within 2 seconds.
- Relay status display changes within 1 second.
- Display survives extended runtime without UART corruption.

### 8. Implement Weather Forecast

Priority: medium/low

Tasks:

- Use OpenMeteo Magog endpoints from `reference/weather-api.md`.
- Fetch hourly forecast for Page 1 / Ressentie.
- Fetch daily forecast for Page 2 / Previsions.
- Parse JSON.
- Map WMO weather codes to Nextion icon IDs.
- Update forecast rows and precipitation percentages.
- Cache or retain last known values if fetch fails.

Acceptance:

- Forecast pages show real weather data.
- API updates do not exceed intended polling intervals.
- Forecast display degrades gracefully if WiFi/API is unavailable.

### 9. Complete Settings Persistence

Priority: medium

Tasks:

- Persist setpoint, temperature correction, hysteresis/deadband, presence mode, schedule enable, schedule times, and manual state as appropriate.
- Decide which settings live on-device versus in Home Assistant.
- Document restore behavior when Home Assistant is offline.

Acceptance:

- Power cycle does not lose user settings.
- OTA does not reset calibration or schedule.

### 10. Add Testing And Deployment Evidence

Priority: medium

Tasks:

- Add a test checklist or log for GPIO, DS18B20, relay, WiFi, OTA, Nextion, schedule, and overrides.
- Perform at least one hardware validation pass.
- Perform a controlled heating cycle test.
- Document rollback to the original Blynk/Arduino firmware if needed.

Acceptance:

- Each functional requirement has a pass/fail result.
- Production readiness is based on tested behavior, not docs alone.

## Suggested Implementation Order

1. Replace synthetic temperature with real DS18B20 on GPIO4.
2. Set safe thermostat timing and verify relay boot behavior on GPIO12.
3. Choose and document one final control architecture.
4. Add Force Off and Emergency Heat with timeout.
5. Add Home/Away and schedule gating.
6. Add sensor malfunction shutdown.
7. Add Nextion display updates for temperature, setpoint, relay, and alarms.
8. Add weather forecast pages.
9. Run hardware validation and update docs to match the active YAML.

## Bottom Line

The current `gazebo-thermostat.yaml` is a valid ESPHome starting point, not a complete migration of the original Blynk propane stove thermostat. The project has strong documentation for the target state, but the active implementation currently covers only the foundation and a basic thermostat shell. The most urgent next step is to connect the climate logic to the real DS18B20 sensor and then add the documented safety/priority controls before relying on it for unattended stove operation.

## Addendum: Review Of `/Users/michel/Documents/Electronics/Projects/GazeboStove`

Date reviewed: 2026-06-06

The separate GazeboStove project directory does contain the modular implementation described by the documentation. This is materially different from the simplified active file at `/Users/michel/Documents/Esphome/gazebo-thermostat.yaml`.

Reviewed project entry point:

- `/Users/michel/Documents/Electronics/Projects/GazeboStove/gazebo_stove.yaml`

Reviewed active includes:

- `includes/core.yaml`
- `includes/hardware.yaml`
- `includes/sensors.yaml`
- `includes/entities.yaml`
- `includes/automation.yaml`
- `includes/display.yaml`
- `includes/climate.yaml`
- `includes/buttons.yaml`
- `includes/weather.yaml`

Validation command attempted:

```bash
/Users/michel/Documents/Esphome/venv/bin/esphome config gazebo_stove.yaml
```

Validation result:

- Status: failed.
- ESPHome version used: 2026.5.2.
- Warnings: GPIO2 and GPIO12 are ESP32 strapping pins.
- Blocking error: obsolete `ha_acurite_temp` and `ha_acurite_humidity` IDs are referenced in `includes/automation.yaml`, but no matching IDs are defined in the active configuration.

Current hardware/source assumptions provided after review:

- The previous AcuRite Home Assistant entities are no longer valid.
- They should be replaced by:
  - `sensor.nhmeteo_temperature`
  - `sensor.nhmeteo_humidity`
- The DHT22 sensor path is obsolete for the current target.
- The intended local thermostat sensor is DS18B20.
- The test stand does not yet have the DS18B20 installed; it will be installed when needed for hardware testing.

### What This Project Copy Implements

Compared with `gazebo-thermostat.yaml`, the GazeboStove project copy is much further along:

- Modular package-based ESPHome entry point.
- ESP32 DOIT DevKit v1 board using Arduino framework.
- WiFi, API, OTA, mDNS, and SNTP.
- PID climate controller named `Gazebo Stove`.
- Slow PWM output on GPIO12 for the heater relay.
- Manual Run / Emergency Heat switch with 15 minute timeout.
- Manual Stop / Force Off switch.
- Presence Mode select with `home` / `away`.
- Autonomous schedule controls using ESPHome `datetime` entities.
- Priority script: Force Off, Emergency Heat, Schedule, PID.
- DHT22 temperature and humidity sensor on GPIO4.
- Commented-out DS18B20 path.
- Sensor malfunction binary sensor and bad-read counter.
- Nextion display component on UART.
- Nextion touch handling and custom slider packet handling.
- Weather forecast display fed by Home Assistant imported sensors.
- Restart, safe shutdown, and PID autotune buttons.

### Main Issues Found

1. The modular config does not currently validate.

`includes/automation.yaml` references `id(ha_acurite_temp)` and `id(ha_acurite_humidity)` in the Nextion initialization, wake refresh, and 5 second display refresh paths. Those IDs are not defined. ESPHome suggests only `gazebo_humidity` as a similar ID.

Affected locations:

- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:69`
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:74`
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:129`
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:132`
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:340`
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/includes/automation.yaml:345`

Required fix:

- Replace the obsolete AcuRite references with Home Assistant imported sensors backed by:
  - `sensor.nhmeteo_temperature`
  - `sensor.nhmeteo_humidity`
- Use stable ESPHome IDs such as `ha_nhmeteo_temp` and `ha_nhmeteo_humidity`, then update `includes/automation.yaml` to reference those IDs.
- Keep the local DS18B20 sensor separate as `gazebo_temp` for thermostat control.

2. A Home Assistant long-lived token is committed in `check_schedule.py`.

`/Users/michel/Documents/Electronics/Projects/GazeboStove/check_schedule.py:5` contains a hardcoded bearer token. Treat it as compromised: revoke it in Home Assistant, create a new one only if still needed, and load it from an environment variable or ignored local config file.

3. The selected sensor does not match the current target hardware.

`includes/core.yaml` sets:

```yaml
sensor_type: "dht22"
sensor_pin: "GPIO4"
```

The DHT22 path is now obsolete. The current target is DS18B20 on GPIO4. The test stand does not yet have the DS18B20 installed, so the firmware may need a temporary test-mode sensor until hardware installation, but the production configuration should be DS18B20-based.

4. The `sensor_type` substitution is only documentary right now.

The DHT22 block is active and the DS18B20 block is commented out. Changing `sensor_type` alone will not switch implementations. The comments imply a selectable abstraction, but the YAML does not implement conditional inclusion.

Required fix:

- Re-enable the `one_wire:` bus in `includes/hardware.yaml`.
- Re-enable the `dallas_temp` sensor in `includes/sensors.yaml`.
- Disable or remove the DHT22 block.
- If the test stand needs to compile before the DS18B20 is installed, use an explicit temporary test sensor or package overlay instead of leaving DHT22 as the apparent target sensor.

5. The project still contains competing schedule models.

The firmware includes autonomous ESPHome schedule logic using `datetime` entities. `homeassistant/automations.yaml` still contains older Home Assistant automations using different helper names. This can confuse setup and could produce duplicate or conflicting scheduling if both are installed.

6. Timezone is inconsistent.

`includes/core.yaml` uses `America/Edmonton`, while the OpenMeteo docs and Magog location reference use `America/Toronto`. If this stove is in Quebec/Magog, the active firmware timezone should be changed to `America/Toronto`, or the docs should be corrected.

7. Nextion baud rate differs by source.

The active modular display config uses `115200`, while older docs and test files often show `9600`. This may be correct for the flashed Nextion HMI, but it should be verified against the actual display firmware and documented as the current standard.

8. GPIO2 and GPIO12 are both strapping pins.

Validation warns about both. GPIO12 controls the relay through slow PWM, and GPIO2 is the status LED. The hardware should be checked so neither external circuit pulls the ESP32 into a bad boot state.

9. Fallback AP password is hardcoded.

`includes/core.yaml` hardcodes `gazebo1234` for the fallback AP. It should move to `!secret ap_password`, matching the safer pattern used in the simplified active file.

### Updated Interpretation

The earlier conclusion that the modular implementation was missing was correct only for `/Users/michel/Documents/Esphome`. It is not correct for `/Users/michel/Documents/Electronics/Projects/GazeboStove`.

Current state across both locations:

- `/Users/michel/Documents/Esphome/gazebo-thermostat.yaml`: validates, but is minimal and uses synthetic temperature.
- `/Users/michel/Documents/Electronics/Projects/GazeboStove/gazebo_stove.yaml`: has the richer modular implementation, but currently fails ESPHome validation because of missing AcuRite sensor IDs.

The next practical step is to decide which tree is the deployment source of truth. If the modular GazeboStove project is intended to be the real firmware, replace the obsolete AcuRite references with NHMeteo Home Assistant imports, switch the target local sensor path from DHT22 to DS18B20, then rerun ESPHome validation before copying or replacing the simplified `gazebo-thermostat.yaml`.
