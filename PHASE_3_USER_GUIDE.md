# Phase 3 User Guide - Quick Start

**For the impatient**: Just toggle switches in Home Assistant. Everything else is automatic.

---

## Three Ways to Control Your Stove Now

### 1. Temperature Control (Phase 2 - Still Works)
- Adjust the "Desired Temperature" slider: 5-35°C
- Set "Heating Hysteresis": 1-5°C (wider = fewer on/off cycles)
- System automatically heats when temp falls below setpoint
- Works 24/7 when both Presence and overrides are disabled

**Best for**: Normal day-to-day heating

---

### 2. Presence Mode (NEW - Energy Saving)
- **Off (AWAY)**: Heating completely off, even if it's cold
- **On (HOME)**: Normal temperature-based heating

**Use cases**:
- Away: Leaving the house? Switch to "away" → No heating, saves energy
- Home: Coming back? Switch to "home" → Heating resumes
- Can be automated: Tie to Home Assistant presence sensors

**Safety**: Away mode is the HIGHEST priority - nothing can override it

**Toggle location**: Home Assistant → Select entity "Presence Mode"

---

### 3. Emergency Overrides (NEW - For Emergencies)

#### Manual Run (Emergency Heat - 15 minutes)
**What it does**: Forces heating ON for exactly 15 minutes, ignoring temperature

**Use when**:
- Stove won't light and you need heat NOW
- Troubleshooting: Need to test relay manually
- Temperature sensor broken: Need manual control

**How to use**:
1. Toggle "Manual Run" switch ON
2. Relay immediately turns ON
3. Set a timer for 15 minutes in your head
4. Fix the problem
5. Option A: Toggle OFF when done
6. Option B: Wait 15 min, it auto-disables

**Important**: 15 minutes is the hard limit. Won't run longer even if you forget to turn it off.

**Don't use for**: Regular heating (that's what Desired Temp is for!)

---

#### Manual Stop (Emergency Shutdown - Indefinite)
**What it does**: Forces heating completely OFF, locked until you unlock it

**Use when**:
- Servicing the stove (maintenance safety)
- Overheating and need to cool down
- Stove malfunction and need time to diagnose

**How to use**:
1. Toggle "Manual Stop" switch ON
2. Relay immediately turns OFF
3. Stove is locked off - temperature changes won't turn it back on
4. Do your maintenance/diagnosis
5. Toggle "Manual Stop" switch OFF when done
6. Normal heating resumes

**Important**: Temperature won't turn on heat while Manual Stop is on.

**Don't use for**: If you just want to stop heating (toggle Presence to "away" instead)

---

## Control Priority Reference Card

**Which control wins when multiple are active?**

```
AWAY MODE
└─ Overrides EVERYTHING
   (Even if Manual Run, Manual Stop, or Temp say turn on)

MANUAL STOP
└─ If AWAY is off, this wins
   (Blocks Manual Run and Temp)

MANUAL RUN (15 minutes only)
└─ If AWAY and STOP are off, this wins
   (Overrides Temp, but has time limit)

TEMPERATURE CONTROL
└─ Only works if all overrides are disabled
   (This is your normal day-to-day operation)
```

**Real-world examples**:

| Situation | Away | Stop | Run | What Happens |
|-----------|------|------|-----|--------------|
| Normal | Off | Off | Off | Temp controls (thermostat) |
| Leaving | **On** | Off | Off | Relay OFF (energy saving) |
| Service | Off | **On** | Off | Relay OFF (maintenance mode) |
| Emergency | Off | Off | **On** | Relay ON for 15 min max |
| Override test | **On** | Off | **On** | Relay OFF (Away wins) |

---

## Dashboard Layout

**Where to find everything in Home Assistant:**

```
Control Panel
├── Presence Mode (dropdown: home / away)
├── Manual Run (toggle: 15-min emergency heat)
├── Manual Stop (toggle: indefinite shutdown)
│
Display Info
├── Relay Status (on/off indicator)
├── Current Temperature (°C)
├── Desired Temperature (slider: 5-35°C)
├── Heating Hysteresis (slider: 1-5°C)
└── System Status (OK / Display Offline / Alarm)
```

---

## Common Scenarios

### Scenario A: "I'm leaving the house"
1. Switch Presence Mode to "away"
2. Done! Heating is off, energy is saved
3. When you return: Switch back to "home"

---

### Scenario B: "Stove won't light"
1. Keep Presence Mode = "home"
2. Toggle Manual Run ON
3. Relay turns on immediately (heating enabled)
4. Try lighting the stove again
5. Once lit: Toggle Manual Run OFF
6. If you forget: It auto-disables after 15 minutes

---

### Scenario C: "I need to fix the stove"
1. Toggle Manual Stop ON
2. Relay turns OFF and STAYS OFF
3. Safely work on the stove
4. When done: Toggle Manual Stop OFF
5. Heating resumes normal operation

---

### Scenario D: "It's too hot!"
1. Quick fix: Set Presence Mode to "away" (full shutdown)
2. Or: Toggle Manual Stop ON (indefinite shutdown)
3. Or: Lower Desired Temperature to minimum
4. Then: Adjust as needed (away → home, or increase temp)

---

### Scenario E: "I want no heating at night"
1. At bedtime: Switch Presence Mode to "away"
2. In the morning: Switch Presence Mode to "home"
3. Optional: Automate this with Home Assistant automations
   (Every day at 22:00 → away, Every day at 7:00 → home)

---

## Safety Features Built In

✅ **Relay always starts OFF** (heating disabled until you enable it)

✅ **Away mode blocks everything** (impossible to heat empty house)

✅ **15-minute max for Manual Run** (can't accidentally heat for hours)

✅ **Manual Stop is indefinite** (requires explicit unlock)

✅ **No dangerous combinations possible** (system prevents conflicting states)

---

## Monitoring and Diagnostics

### Check what's controlling the relay:

**Look at system logs**:
```
[PRIORITY] Away mode: Relay OFF (priority 1)
→ Away mode is active, overriding everything

[PRIORITY] Manual Run: Relay ON (priority 3)
→ Manual Run is controlling (15-min timer active)

[HEATING] Start heating: 21.1°C < 24.0°C (priority 4)
→ Thermostat is controlling (no overrides active)
```

### Manual Run timer countdown:

```
[MANUAL_RUN] Timeout check: 14:30 remaining
[MANUAL_RUN] Timeout check: 14:00 remaining
...
[MANUAL_RUN] 15-minute timeout reached - auto-disabling
```

---

## Troubleshooting

### Q: "Manual Run is on but relay doesn't turn on"
**A**: Check Presence Mode. If it's set to "away", that blocks everything.
- Solution: Change Presence Mode to "home"

### Q: "Manual Stop won't let anything turn on"
**A**: That's the point! It's the emergency lock.
- Solution: Toggle Manual Stop OFF to restore control

### Q: "I toggled Manual Run but forgot about it"
**A**: After 15 minutes it auto-disables automatically.
- If you want to turn it off sooner: Just toggle the switch OFF

### Q: "Desired temperature slider doesn't work"
**A**: Check if Manual Stop is on (overrides temp control).
- Solution: Toggle Manual Stop OFF, then temp will work

### Q: "Presence mode changed but nothing happened"
**A**: Presence mode only affects relay when it's the deciding factor.
- If Manual Stop is on: Manual Stop takes priority
- If Manual Run is on: Manual Run takes priority
- Thermostat still runs (you just see different control priority in logs)

---

## Automation Ideas

Use Home Assistant automations to make it even smarter:

### Automatic Away When Nobody Home
```yaml
automation:
  - alias: "Set Away when everyone leaves"
    trigger:
      platform: state
      entity_id: binary_sensor.everyone_home
      to: "off"
    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "away"
```

### Automatic Home When Someone Arrives
```yaml
automation:
  - alias: "Set Home when anyone arrives"
    trigger:
      platform: state
      entity_id: binary_sensor.everyone_home
      to: "on"
    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "home"
```

### Notify When Manual Run Activated
```yaml
automation:
  - alias: "Alert when manual heating started"
    trigger:
      platform: state
      entity_id: switch.manual_run
      to: "on"
    action:
      - service: notify.notify
        data:
          message: "Manual heating activated (15-min timer)"
```

---

## Technical Details (For the Curious)

### How the Priority System Works

Every time any control changes, the system asks these questions IN ORDER:

1. **Is Presence Mode set to away?** → YES: Relay OFF, STOP here
2. **Is Manual Stop enabled?** → YES: Relay OFF, STOP here
3. **Is Manual Run enabled?** → YES: Relay ON, STOP here (unless timed out)
4. **None of the above?** → Let thermostat control it (temp-based)

The first "YES" wins. Everything else is ignored.

### Why Three Separate Controls?

- **Presence Mode**: Saves energy automatically, can be triggered automatically
- **Manual Run**: Emergency only, has automatic timeout to prevent accidents
- **Manual Stop**: Maintenance safety, indefinite (requires manual re-enable)
- **Thermostat**: Normal day-to-day heating based on temperature

Each solves a different problem. Together, they cover every use case.

---

## Quick Reference

| What you want | What to do |
|---------------|-----------|
| Normal heating | Set desired temp, enjoy |
| Stop wasting energy | Toggle Presence to "away" |
| Emergency heat NOW | Toggle Manual Run ON |
| Safe maintenance | Toggle Manual Stop ON |
| Stop everything | Toggle Presence to "away" |

---

## Support

**For detailed technical information**, see:
- `PHASE_3_IMPLEMENTATION.md` - Complete implementation guide
- `Docs/PHASE_2_THERMOSTAT.md` - Thermostat algorithm details
- Device logs - Available in Home Assistant → Settings → Devices & Services → Gazebo Stove

**For configuration changes**, see:
- `includes/entities.yaml` - Control definitions
- `includes/automation.yaml` - Priority logic
- `QUICK_REFERENCE.md` - Configuration quick lookup

---

**Version**: Phase 3 - Implemented 2025-11-11

**Status**: ✅ Production Ready

**Last Updated**: 2025-11-11
