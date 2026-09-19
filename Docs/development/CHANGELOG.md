# GazeboStove Change Log

This is the durable, append-only record of meaningful configuration, control,
and deployment changes to the active ESPHome GazeboStove project.

Each new entry should state what changed, why, the related Git commit when one
exists, validation performed, and whether the firmware was actually deployed.
Build success is not deployment; OTA success is not a functional heating test.

## Current architecture record

- Active entry point: `gazebo_stove.yaml`
- Thermostat and relay control owner: ESPHome device `gazebo-stove`
- Relay output: GPIO12 through `heater_pwm`
- Home Assistant is the dashboard and entity interface; it does not own the
  normal thermostat decision or relay output.
- Normal relay writes are arbitrated by `apply_heating_output` in
  `includes/automation.yaml`.

## 2026-09-19 — Relay arbitration and Away-to-Home recovery

Commit: `a50be4f` — `Arbitrate gazebo stove relay control`

Why:

- The stove could remain off after an Away-to-Home transition even while the
  climate entity reported heating.
- Multiple paths could write GPIO12 directly, so reported thermostat demand
  and physical relay state could diverge.

Changed:

- Added a persisted `presence_away_active` state that is updated before the
  Home/Away select invokes heating management. This removes the stale select
  state race on an Away-to-Home transition.
- Added explicit thermostat-demand and effective-heat-request state.
- Made `apply_heating_output` the normal arbiter for `heater_pwm` / GPIO12.
- Routed thermostat heat, idle, and off callbacks back through
  `manage_heating`, including delayed thermostat callbacks after the minimum
  off/run protections.
- Routed Service Relay Override and Safe Shutdown through the arbiter while
  retaining Force Off as the highest-priority safety lockout.
- Expanded diagnostics to show thermostat demand, requested output, and actual
  PWM output separately.

Validation and deployment:

- `git diff --check` passed.
- `./.venv-esphome/bin/esphome config gazebo_stove.yaml` passed.
- `./.venv-esphome/bin/esphome compile gazebo_stove.yaml` passed with ESPHome
  2025.10.3 (RAM 11.5%, flash 57.6%).
- OTA upload to `192.168.0.242` succeeded.
- Post-upload live API logs confirmed a normal boot, Home/HEAT mode, and a
  consistent idle state at 20.5 C actual versus 20.0 C target: demand OFF,
  requested output OFF, actual output 0%.

Still to verify:

- A supervised functional Away-to-Home test with target temperature above the
  current room temperature. Expected log sequence: `ThermostatDemand=true`,
  `RequestedOutput=ON`, then non-zero actual output.

## 2026-09-19 — Preserve the user target during Home recovery

Commit: `79f6a69` — `Preserve thermostat target on Home recovery`

Why:

- Applying the Home preset on Away-to-Home and Force Off recovery reset the
  target to the preset default of 20 C, overriding the temperature selected on
  the Nextion display or Home Assistant dashboard.

Changed:

- Removed `preset: Home` from the Home restoration and Force Off recovery
  climate calls, preserving the current user-selected target.
- Refreshed the gazebo temperature component after restoring Home mode.
- Added `Docs/heating_schedule_fix.md` to separate the status-label correction
  from the target-preservation repair.

Validation and deployment:

- The repair was compiled successfully as part of the later 2026-09-19
  deployment.
- It was installed by the OTA upload recorded in the relay-arbitration entry
  above.

## 2026-09-03 — Event-driven schedule control

Commit: `ad22a4d` — `Add event-driven schedule control`

Why:

- Scheduled Home/Away transitions were previously dependent mainly on periodic
  polling and could miss or delay immediate heating reevaluation.

Changed:

- Added `evaluate_schedule` for normal and overnight schedule windows, with
  valid-time and equal-time protections.
- Re-evaluated the schedule on NTP synchronization, schedule enable changes,
  Home/Away time edits, and the existing one-minute interval.
- Preserved Force Off priority and restored normal heating management when
  scheduling is disabled.

Validation and deployment:

- ESPHome configuration, compilation, and whitespace checks passed.
- Live device verification confirmed the expected Away / Schedule Standby /
  thermostat-off / relay-off state before the scheduled Home time.
- User later confirmed the schedule worked.

## 2026-08-29 — Service controls and thermostat safety

Commits:

- `946d51b` — `Snapshot modular stove thermostat configuration`
- `ca822a8` — `Add ESPHome service pulse and asymmetric hysteresis`
- `ae7cc25` — `Add ESPHome safe mode recovery button`

Changed:

- Consolidated the modular thermostat configuration and dashboard card state.
- Added the dashboard-controlled service heat pulse and an adjustable
  asymmetric hysteresis setting.
- Added the ESPHome Safe Mode recovery button for remote recovery of the
  custom board, which has no USB serial connection available.

Deployment note:

- These commits establish the configuration baseline for the later schedule,
  target-preservation, and output-arbitration repairs. Deployment status for
  individual August commits was not reconstructed from the available notes.

## 2026-06-07 to 2026-06-20 — Display, temperature, and climate evolution

Historical commits:

- `ed0dcc6` — Updated the ESPHome environment.
- `a5a7a00` — Fixed Nextion slider target synchronization.
- `728d9ec` and `fa13045` — Stabilized Nextion startup page selection.
- `948e0e0` and `20e77ae` — Added and corrected current-condition display
  updates.
- `f57812f` — Exposed thermostat tuning controls to Home Assistant.
- `93c88b8` — Removed PID cooling output.
- `7318a4d` — Calibrated temperature and stabilized the Nextion slider.
- `f4510a4` — Replaced the PID controller with the ESPHome thermostat climate
  component.

Note:

- These entries are reconstructed from Git history. Refer to the commit diff
  for exact implementation details and validation evidence.

## 2025-12 — Initial ESPHome stove implementation

Historical work included GPIO relay control, schedule and manual-stop repairs,
Nextion integration, Open-Meteo display data, and early thermostat/PID work.
The exact implementation history is retained in Git commits from
`804912c` through `2f97a3e`; this changelog intentionally summarizes that
foundation rather than duplicating every early incremental commit.

## Entry template

```md
## YYYY-MM-DD — Short change title

Commit: `abcdef0` — `Commit subject`  <!-- omit only if no commit exists -->

Why:

- User-visible symptom, safety concern, or design reason.

Changed:

- Configuration or code changes.

Validation and deployment:

- Static/config validation performed.
- Build result.
- Upload result, if uploaded.
- Live functional test result, if performed.

Still to verify:

- Any intentionally deferred functional or safety test.
```
