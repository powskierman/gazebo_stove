# GazeboStove — Heating-status / Schedule Bug (handoff)

> Purpose: record the diagnosis + fix so it survives moving the session workspace.
> Applied edit: `includes/automation.yaml` (priority-5 "Thermostat" branch).

## Symptom
- The thermostat UI / `sensor.gazebo_stove_heating_status` reports **"Heating"** while the stove is **idle**.
- Happens specifically **when the schedule becomes active** (flipping "Schedule Enabled" ON, or a home/away transition).
- Net effect: the heating element never actually powers on, yet the status says heating.

## Root cause
File: `includes/automation.yaml`, `manage_heating` script, priority‑5 (Thermostat) branch, the
status‑label block (original ~lines 244–249).

The `"Heating"` / `"Idle"` label published to `sensor.gazebo_stove_heating_status` was derived from
the **stale global `heater_output_level`**:

```yaml
// Update status based on thermostat output
if (pwm_output > 0.5) {      // pwm_output = id(heater_output_level)
  id(heating_status).publish_state("Heating");
} else {
  id(heating_status).publish_state("Idle");
}
```

`heater_output_level` is only updated by the `slow_pwm` output's `turn_on`/`turn_off` actions. It is
**never reset when the schedule transitions** (away→home, or when the schedule is enabled). After a
heating cycle it keeps holding the previous output level (e.g. `1.0`), so the display flips to
"Heating" even though the relay is physically off.

**Trigger chain** ("when the schedule becomes active"):
1. Flip "Schedule Enabled" ON → `schedule_enabled.turn_on_action` → `script.execute: evaluate_schedule`
   (`includes/hardware.yaml:185-188`).
2. `evaluate_schedule` changes `presence_mode` (home/away) → the select's `set_action` calls
   `climate.control: HEAT` then `script.execute: manage_heating`.
3. `manage_heating` reaches the thermostat branch and reads the stale `heater_output_level`,
   mislabeling the state.

## Fix
Read the thermostat's **actual** action instead of the stale global (current file, ~lines 244–257):

```yaml
if (id(gazebo_thermostat).action == climate::CLIMATE_ACTION_HEATING) {
  id(heating_status).publish_state("Heating");
} else {
  id(heating_status).publish_state("Idle");
}
```

`gazebo_thermostat.action` (ESPHome `ClimateAction`) reflects the thermostat controller's normal
automatic demand, so this label cannot get stuck on a stale output level. Manual Run and the
service relay pulse directly command PWM while the thermostat is OFF; those priority branches
publish their own status and are not represented by this climate action.

## To verify
1. Deploy the updated YAML (OTA / `esphome deploy`).
2. Flip "Schedule Enabled" ON and watch the `priority` / `thermostat` log tags.
3. In normal thermostat operation, `sensor.gazebo_stove_heating_status` should be **Heating** only
   while `gazebo_thermostat.action == HEATING`. Priority overrides report their own status.

## Still to watch (out of scope of this fix)
If the element still fails to energize despite genuine demand (current below the user-selected target),
look at:
- `min_heating_off_time` / `min_heating_run_time` (30s lockout) in `includes/climate.yaml`;
- the priority‑0 "RESTORE HEAT mode" guard at the end of `manage_heating` — it only re-asserts HEAT
  when `gazebo_thermostat.mode == CLIMATE_MODE_OFF`.

## Rollback
Revert the status-label block back to the `if (pwm_output > 0.5)` form.
