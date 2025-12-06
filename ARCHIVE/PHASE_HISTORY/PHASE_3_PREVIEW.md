# Phase 3: Home/Away & Manual Modes - What Will Be Accomplished

**Timeline**: Weeks 5-6 (approximately 2-3 weeks from now)
**Status**: Ready for implementation after Phase 2 completion
**Date**: 2025-11-11

---

## Overview

Phase 3 adds advanced control modes to the basic thermostat. It enables:
- Presence-based heating control (Home vs Away)
- Emergency manual override modes
- Priority-based control logic
- Real-time mode feedback on Nextion display

---

## 3.1: Presence Mode (Home/Away) Control

### What It Does
Creates a **Presence Mode** selector in Home Assistant that lets you tell the system whether the house is occupied.

### Features
- **Two states**: "home" or "away"
- **Persistent**: Setting survives power cycles (stored in Home Assistant)
- **Default**: Starts in "home" mode on device boot
- **Visual feedback**: Nextion display shows current presence state

### How It Works
```
When Presence = "away":
  ├─ Relay forced OFF (immediately)
  ├─ Thermostat algorithm disabled
  └─ Heating will not resume until Presence = "home"

When Presence = "home":
  └─ Normal thermostat operation (temperature-based control)
```

### User Interface Changes
- New dropdown entity in Home Assistant: `select.presence_mode`
- Options: "home" / "away"
- Can be set manually via Home Assistant dashboard
- Can be automated (e.g., via location tracking, time of day)

### Example Scenario
- You leave house: Set Presence Mode to "away" → Heating turns off
- House cools to 15°C while you're gone
- You return home: Set Presence Mode to "home" → Heating turns on if temp < setpoint
- Temperature returns to comfortable level (e.g., 22°C)

---

## 3.2: Manual Run Override

### What It Does
Creates an emergency **Manual Run** mode that forces the heating on immediately, regardless of temperature or settings.

### Features
- **Immediate activation**: Relay turns ON as soon as enabled
- **15-minute timer**: Automatically disables after 15 minutes
- **User cancelable**: Can turn off manually anytime before timer expires
- **Thermostat disabled**: Thermostat algorithm doesn't interfere
- **Visual countdown**: Nextion display shows "MANUAL MODE ACTIVE - XY minutes remaining"

### How It Works
```
When Manual Run enabled:
  ├─ Relay turns ON immediately
  ├─ Heating algorithm bypassed
  ├─ Start 15-minute countdown
  ├─ Every minute, check if 15 minutes elapsed
  ├─ At 15 minutes: Automatically disable
  └─ Display shows countdown timer

While Manual Run active:
  └─ User can disable anytime by turning off switch
```

### Implementation Details
- New switch entity: `switch.manual_run`
- Timer managed in Home Assistant automation (not device)
- Uses `time_delta` to calculate elapsed time
- Automatic timeout via Home Assistant automation

### Example Scenario
- Stove won't light up (emergency situation)
- Enable Manual Run → Heating turns on immediately
- Troubleshoot the problem
- Manually turn off when fixed, OR
- Wait 15 minutes and it auto-disables (safety timeout)

---

## 3.3: Manual Stop Override

### What It Does
Creates a **Manual Stop** mode that forces the heating off indefinitely.

### Features
- **Immediate shutdown**: Relay turns OFF when enabled
- **Indefinite hold**: Stays off until user manually disables
- **Override thermostat**: Temperature changes won't restart heating
- **Respects Away mode**: Away mode takes priority (even if Manual Stop is off)
- **Visual feedback**: Nextion shows "MANUAL STOP - System halted"

### How It Works
```
When Manual Stop enabled:
  ├─ Relay turns OFF immediately
  ├─ Heating algorithm disabled
  ├─ Stays off until user disables
  └─ Display shows "MANUAL STOP"

While Manual Stop active:
  ├─ Temperature changes ignored
  ├─ Thermostat can't override it
  ├─ Away mode still takes priority
  └─ Only manual disable restores control
```

### Use Cases
- Maintenance work on stove
- Emergency shutdown
- Preventing heating during hot weather
- System troubleshooting

### Example Scenario
- Need to service the heating system
- Enable Manual Stop → Heating disabled
- Work on stove safely (relay definitely OFF)
- When done: Disable Manual Stop → Normal operation resumes

---

## 3.4: Override Priority Logic

### Priority Hierarchy
The system evaluates these in order. The first "true" condition wins:

```
1. Away Mode (HIGHEST PRIORITY)
   └─ If Presence = "away" → Relay OFF (always)
      Even if Manual Run is on, relay stays OFF
      Even if temperature is freezing

2. Manual Stop
   └─ If Manual Stop enabled → Relay OFF
      Unless Away mode overrides it

3. Manual Run
   └─ If Manual Run enabled → Relay ON
      For exactly 15 minutes, then auto-disable

4. Thermostat Algorithm (LOWEST PRIORITY)
   └─ If none of above apply → Use temperature-based control
      Normal hysteresis control (the Phase 2 algorithm)
```

### Truth Table
```
Away  ManualStop  ManualRun  Result
─────────────────────────────────────────
away   (any)      (any)      → RELAY OFF (Away overrides all)
home   on         (any)      → RELAY OFF (Manual Stop)
home   off        on         → RELAY ON  (Manual Run, 15-min timer)
home   off        off        → (Thermostat controls based on temp)
```

### Example Scenarios

**Scenario 1: Away + Manual Run**
- Status: You try to enable Manual Run while in Away mode
- Result: Relay stays OFF (Away mode has priority)
- Logic: Safety first - never heat an empty house

**Scenario 2: Normal operation**
- Status: Presence="home", ManualStop=off, ManualRun=off
- Result: Thermostat controls heating normally
- Logic: Uses Phase 2 heating algorithm

**Scenario 3: Emergency manual heat**
- Status: Presence="home", enable Manual Run
- Result: Relay ON for 15 minutes (regardless of temperature)
- Logic: Override everything for emergency heating

---

## 3.5: Implementation Architecture

### ESPHome Changes (Minor)
The device itself just monitors and responds to commands:
```yaml
switch:
  - platform: template
    name: "Manual Run"
    id: manual_run
    # Home Assistant controls the timer
    # Device just sets relay based on overall priority

  - platform: template
    name: "Manual Stop"
    id: manual_stop
    # Same approach
```

### Home Assistant Automation (New)
The heavy lifting happens in Home Assistant:

```yaml
automation:
  # Manual Run Timeout - runs every minute
  - alias: "Manual Run Timeout"
    trigger:
      platform: time_pattern
      minutes: /1
    condition:
      - condition: state
        entity_id: switch.manual_run
        state: "on"
    action:
      - condition: template
        value_template: '{{ (now() - states.switch.manual_run.last_changed).total_seconds() > 900 }}'
      - service: switch.turn_off
        target:
          entity_id: switch.manual_run

  # Update relay based on priority logic
  - alias: "Evaluate Control Priority"
    trigger:
      - platform: state
        entity_id: select.presence_mode
      - platform: state
        entity_id: switch.manual_run
      - platform: state
        entity_id: switch.manual_stop
    action:
      - service: switch.turn_on/off
        target:
          entity_id: switch.propane_stove_relay
        data_template:
          is_on: >-
            {%- if states('select.presence_mode') == 'away' -%}
              false
            {%- elif states('switch.manual_stop') == 'on' -%}
              false
            {%- elif states('switch.manual_run') == 'on' -%}
              true
            {%- else -%}
              {{ states('switch.propane_stove_relay') == 'on' }}
            {%- endif -%}
```

---

## 3.6: User Interface Changes

### New Home Assistant Entities
1. **`select.presence_mode`** (dropdown)
   - Options: "home" / "away"
   - Default: "home"
   - Persistent across reboots

2. **`switch.manual_run`** (toggle switch)
   - State: on/off
   - Shows: 15-minute countdown when active
   - Auto-disables after 15 minutes

3. **`switch.manual_stop`** (toggle switch)
   - State: on/off
   - When on: Heating disabled until manually turned off

### New Nextion Display Updates
The display will show additional status:
- Current presence mode ("HOME" or "AWAY")
- Manual mode status ("MANUAL RUN - 12:30" or "MANUAL STOP")
- Priority indicator showing what's controlling the relay

### Example Home Assistant Dashboard Card
```yaml
type: entities
entities:
  - entity: select.presence_mode
    name: Presence
  - entity: switch.manual_run
    name: Manual Run (15-min timer)
  - entity: switch.manual_stop
    name: Manual Stop (indefinite)
  - entity: switch.propane_stove_relay
    name: Relay Status
  - entity: text_sensor.control_priority
    name: Control Authority
```

---

## 3.7: Testing & Validation

### Test Cases to Verify

**Test 1: Presence Mode Away**
- ✓ Relay OFF when presence = "away"
- ✓ Relay stays OFF even if desired temp > current temp
- ✓ Relay stays OFF if Manual Run enabled
- ✓ Relay OFF if Manual Stop enabled

**Test 2: Manual Run Mode**
- ✓ Relay ON immediately when enabled
- ✓ Relay ignores temperature readings
- ✓ Countdown timer shows 15:00 on Nextion
- ✓ Auto-disables after 15 minutes
- ✓ Manual disable works anytime
- ✓ Away mode overrides (relay stays OFF even if Manual Run on)

**Test 3: Manual Stop Mode**
- ✓ Relay OFF immediately when enabled
- ✓ Relay stays OFF regardless of temperature
- ✓ Manual Run doesn't override it
- ✓ Thermostat doesn't override it
- ✓ Away mode still takes priority
- ✓ Manual disable restores control

**Test 4: Priority Logic**
- ✓ Away > Manual Stop > Manual Run > Thermostat
- ✓ Each level blocks lower levels
- ✓ Mode transitions work smoothly
- ✓ No relay chatter or oscillation

**Test 5: Persistence**
- ✓ Settings survive Home Assistant restart
- ✓ Settings survive device restart
- ✓ Presence mode remembered after power cycle

---

## 3.8: Advanced Automations (Bonus)

Once Phase 3 is done, these automations become possible in Home Assistant:

### Automatic Home/Away Detection
```yaml
automation:
  - alias: "Set Away when last person leaves"
    trigger:
      platform: state
      entity_id: binary_sensor.anyone_home
      to: "off"
    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "away"

  - alias: "Set Home when first person arrives"
    trigger:
      platform: state
      entity_id: binary_sensor.anyone_home
      to: "on"
    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "home"
```

### Emergency Manual Run Notification
```yaml
automation:
  - alias: "Notify when Manual Run activated"
    trigger:
      platform: state
      entity_id: switch.manual_run
      to: "on"
    action:
      - service: notify.notify
        data:
          message: "Manual heating activated - will auto-disable in 15 minutes"
```

### Time-based Auto-Away
```yaml
automation:
  - alias: "Away at night"
    trigger:
      platform: time
      at: "22:00:00"
    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "away"
```

---

## 3.9: Comparison with Phase 2

| Feature | Phase 2 | Phase 3 |
|---------|---------|---------|
| Temperature control | ✓ Hysteresis-based | ✓ Same + overridable |
| Presence sensitivity | No | ✓ Away = always off |
| Manual emergency mode | No | ✓ Manual Run (15-min) |
| Emergency off | No | ✓ Manual Stop |
| Priority logic | Simple | ✓ 4-level hierarchy |
| User control | Basic sliders | ✓ Multiple override modes |
| Safety features | Sensor alarm | ✓ + Away mode safety |
| Nextion display | Basic status | ✓ + Mode indicators + countdown |

---

## 3.10: Files to be Modified

### ESPHome Configuration
- **includes/automation.yaml** - Add priority evaluation logic
- **includes/entities.yaml** - Add manual mode switches
- **includes/display.yaml** - Update Nextion to show modes

### Home Assistant Configuration (New)
- **automations.yaml** - Manual Run timer + priority logic
- **configuration.yaml** - Add input helpers if needed

### Documentation
- **PHASE_3_IMPLEMENTATION.md** - Detailed implementation guide
- **PHASE_3_THERMOSTAT.md** - Updated with all Phase 3 features
- **TROUBLESHOOTING.md** - Mode-specific troubleshooting

---

## 3.11: Success Criteria

✓ **Heating Control**
- [ ] Relay OFF when presence = "away"
- [ ] Relay ON for exactly 15 minutes when Manual Run enabled
- [ ] Relay OFF indefinitely when Manual Stop enabled
- [ ] All modes respond immediately (< 100ms)

✓ **User Interface**
- [ ] Presence selector visible and functional
- [ ] Manual Run shows 15-minute countdown
- [ ] Manual Stop toggles correctly
- [ ] All modes visible on Nextion display

✓ **Automation**
- [ ] Manual Run auto-disables after 15 minutes
- [ ] Priority logic evaluated correctly
- [ ] Mode transitions smooth (no relay chatter)
- [ ] Settings persist across power cycles

✓ **Safety**
- [ ] Away mode always forces relay OFF
- [ ] No dangerous state combinations possible
- [ ] Failsafe: device boots safely in "home" mode
- [ ] Emergency override works in all conditions

---

## 3.12: Estimated Effort

| Task | Effort | Notes |
|------|--------|-------|
| ESPHome modifications | 4 hours | Simple logic changes |
| Home Assistant automation | 6 hours | Priority logic, timers |
| Nextion display updates | 3 hours | Status indicators |
| Testing | 4 hours | Validation of all modes |
| Documentation | 2 hours | Update guides |
| **Total** | **~19 hours** | ~2.5 days intensive work |

---

## 3.13: Deliverables

After Phase 3 completion, you'll have:

1. **Presence-aware heating system**
   - Automatic away-mode for energy savings
   - Home mode for normal operation

2. **Emergency override modes**
   - Manual Run: Force heating for 15 minutes
   - Manual Stop: Force heating off indefinitely

3. **Smart priority logic**
   - 4-level hierarchy preventing conflicts
   - Impossible to reach dangerous states

4. **Enhanced user experience**
   - Multiple control methods
   - Real-time feedback on control authority
   - Safety without complexity

5. **Foundation for Phase 4**
   - Priority logic ready for timer integration
   - Flexible enough for any future additions

---

## Next Steps After Phase 2

1. **Week 4 end**: Phase 2 fully tested and stable ✓ (current status)
2. **Week 5-6**: Phase 3 implementation
3. **Week 7**: Phase 3 testing and validation
4. **Week 8**: Preparation for Phase 4 (timer/scheduling)

---

**Ready to start Phase 3 when you give the go-ahead!**

For detailed implementation instructions, see PHASE_3_IMPLEMENTATION.md (will be created at start of Phase 3).

---

**Document Version**: 1.0.0
**Status**: Phase 3 Ready (Phase 2 Complete)
**Date**: 2025-11-11
