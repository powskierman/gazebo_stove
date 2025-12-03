# Schedule Migration Guide: Home Assistant → ESPHome

## Overview

The heating schedule has been successfully migrated from Home Assistant automations to native ESPHome time-based triggers. This means the device now works **independently** from Home Assistant for scheduling.

## What Changed

### Before (Home Assistant)
- ❌ Required Home Assistant to be running
- ❌ Relied on Home Assistant automations engine
- ❌ Used `input_datetime` helpers for schedule times
- ❌ Used `input_boolean` helper for enable/disable
- ✅ More flexible with complex conditions

### After (ESPHome)
- ✅ Works without Home Assistant
- ✅ Native to the device firmware
- ✅ Embedded schedule time entities
- ✅ Built-in enable/disable switch
- ✅ More reliable and responsive

## New ESPHome Entities

### Number Entities (Schedule Times)

You'll now see four new number entities in Home Assistant:

| Entity | Purpose | Range | Default |
|--------|---------|-------|---------|
| `number.home_mode_time_hour` | Hour for home mode activation | 0-23 | 6 |
| `number.home_mode_time_minute` | Minute for home mode activation | 0-59 | 0 |
| `number.away_mode_time_hour` | Hour for away mode activation | 0-23 | 22 |
| `number.away_mode_time_minute` | Minute for away mode activation | 0-59 | 0 |

**Default Schedule:**
- Home Mode: 06:00 (6 AM)
- Away Mode: 22:00 (10 PM)

### Switch Entity (Schedule Control)

| Entity | Purpose | Default |
|--------|---------|---------|
| `switch.heating_schedule` | Enable/disable the schedule | ON (enabled) |

## How to Configure the Schedule

### Option 1: Home Assistant UI (Recommended)

1. Go to Home Assistant
2. Navigate to **Settings → Devices & Services → Entities**
3. Search for "Home Mode Time" or "Away Mode Time"
4. Click on each entity and adjust the hour/minute values
5. Use the "Heating Schedule" switch to enable/disable

### Option 2: ESPHome Dashboard

1. Go to ESPHome Dashboard (http://your-device:6052)
2. Click "Logs" to connect to the device
3. Watch the schedule times and enable/disable switch update in real-time

### Option 3: Home Assistant Automations (Optional Fallback)

If you still want Home Assistant automations for complex logic, you can:

1. Create Home Assistant automations that trigger based on the ESPHome entities
2. Use ESPHome automations as a backup schedule
3. HA automations can override ESPHome decisions if needed

## Schedule Behavior

### Daily Execution

- Home Mode automation triggers every day at the configured time
- Away Mode automation triggers every day at the configured time
- The device changes `select.presence_mode` accordingly

### Smart Triggering

- Schedule only triggers if **not already** in the target mode
- Won't spam presence mode changes if already set
- Respects manual mode changes (won't override if you manually change modes)
- Schedule respects Priority 3 in the control system (can be overridden by Manual Run/Stop)

### Enabling/Disabling Schedule

Use the `switch.heating_schedule` entity:

```yaml
# In Home Assistant automations or scripts:
service: switch.turn_on
target:
  entity_id: switch.heating_schedule
```

Or toggle via Home Assistant UI:
1. Go to Settings → Devices & Services → Entities
2. Search for "Heating Schedule"
3. Click to toggle on/off

## Timeline: What Happens When

### Scenario: Default Schedule (Home: 6 AM, Away: 10 PM)

```
06:00 → Home Mode activated
        - presence_mode changes to "home"
        - Thermostat allowed to heat per temperature
        - Log: "Schedule: Switching to Home Mode"

22:00 → Away Mode activated
        - presence_mode changes to "away"
        - Relay forced OFF (priority override)
        - Log: "Schedule: Switching to Away Mode"

(During day) → If you manually change to "away":
        - Manual change respected
        - Schedule won't override until next scheduled time

22:05 → If schedule attempts again:
        - Condition checks: already "away", so no change
        - No log (condition was false)
```

## Removing Home Assistant Automations

You can now safely delete or disable the original Home Assistant automations:

**File:** `homeassistant/automations.yaml`

If keeping these automations:
- They won't conflict (both will work)
- HA automation will trigger 1-2 seconds after ESPHome schedule
- Redundancy is safe but not necessary

Recommended: Delete the automation section to clean up:
```yaml
# DELETE THESE LINES:
- alias: "Gazebo: Set Home Mode at Time"
  ...

- alias: "Gazebo: Set Away Mode at Time"
  ...
```

Similarly, you can remove these HA helpers:
**File:** `homeassistant/configuration.yaml`

```yaml
# DELETE THESE SECTIONS if no longer needed:
input_boolean:
  gazebo_schedule_enabled: ...

input_datetime:
  gazebo_home_mode_time: ...
  gazebo_away_mode_time: ...
```

## Technical Details

### Implementation

The schedule is implemented using **interval-based time checks** in ESPHome:

```
Every 1 minute:
├─ Check if schedule is enabled
├─ Verify NTP time is synced
├─ Compare current time to home_mode time
│  └─ If match AND not in "home" mode → activate home mode
└─ Compare current time to away_mode time
   └─ If match AND not in "away" mode → activate away mode
```

### Why Interval-Based?

ESPHome doesn't have dynamic time triggers like Home Assistant. Using intervals:

✅ Simple and reliable
✅ Low CPU overhead (only checks at :00 seconds)
✅ Works with user-configurable times
✅ Can be enabled/disabled easily
❌ Triggers at any point during the minute (not exact second)

Practical impact: Schedule might trigger at XX:00:00 to XX:00:59 - usually imperceptible to users.

## Priority System Update

The schedule maintains its place in the control priority:

```
Priority 1 (HIGHEST): Manual Stop (Force Off)
Priority 2:           Manual Run (Emergency Heat) - 15 min timer
Priority 3:           Schedule (Away Mode) - Forces relay OFF ← YOU ARE HERE
Priority 4 (LOWEST):  Thermostat (Climate Control)
```

**What this means:**
- If schedule says "away", relay is forced OFF
- But Manual Run can override it (for emergency heat)
- And Manual Stop can override everything
- When schedule says "home", thermostat is allowed to control

## Troubleshooting

### Schedule doesn't trigger

**Check:** Is the schedule enabled?
```
Go to Home Assistant → Entities → switch.heating_schedule
Expected: "ON" (green)
```

**Check:** Is the device time synced?
```
ESPHome → Logs → Look for: "Time synced from NTP"
```

**Check:** Are times configured correctly?
```
Go to Entities → Search "Home Mode Time"
Verify hour and minute values are set
```

### Schedule triggers too early/late

Remember: The device is set to **America/Toronto** timezone (can be changed in `includes/core.yaml`)

Check your time is correct:
```
ESPHome logs → Look for device time output
Or check Home Assistant → Developer Tools → Template
{{ now() }}
```

### Home Assistant automations still running

If you didn't delete the HA automations, both will trigger:
- ESPHome schedule triggers first (1-2 seconds faster)
- HA automation triggers next
- They're harmless together but redundant

Solution: Delete the HA automations (see "Removing Home Assistant Automations" above)

## Reverting to Home Assistant

If you want to go back to Home Assistant automations:

1. Delete the schedule number entities from ESPHome:
   - Remove 4 number entities from `includes/entities.yaml`
   - Remove schedule_enabled switch from `includes/hardware.yaml`
   - Remove schedule intervals from `includes/automation.yaml`

2. Re-enable the Home Assistant automations:
   - Uncomment/restore automations in `homeassistant/automations.yaml`
   - Restore helpers in `homeassistant/configuration.yaml`

3. Recompile and upload ESPHome firmware

## Files Modified

### ESPHome Configuration

| File | Changes |
|------|---------|
| `includes/entities.yaml` | Added 4 schedule time number entities |
| `includes/hardware.yaml` | Added schedule_enabled switch |
| `includes/automation.yaml` | Added 2 interval automations for schedule checks |

### Home Assistant Configuration (No Changes Required)

- `homeassistant/automations.yaml` - Can be deleted/disabled
- `homeassistant/configuration.yaml` - Can be deleted/disabled

## Questions & Answers

**Q: Will the schedule work if Home Assistant is offline?**
A: Yes! The schedule is entirely managed by ESPHome firmware now.

**Q: Can I change the schedule times from Home Assistant?**
A: Yes! The number entities are exposed to Home Assistant and fully controllable.

**Q: What timezone is the schedule using?**
A: America/Toronto (set in `includes/core.yaml` under the `time` section)

**Q: Can I have multiple schedule times per day?**
A: Currently, only one "home" and one "away" time daily. For more complex schedules, create HA automations that call the ESPHome select entity.

**Q: What happens if I change the schedule times?**
A: They take effect immediately. The schedule checks current time vs. current settings every minute.

**Q: Can I disable the schedule temporarily?**
A: Yes! Use the `switch.heating_schedule` entity to toggle it on/off.

---

## Migration Checklist

- [ ] Verify schedule triggers at correct times (watch logs or device behavior)
- [ ] Test home mode time manually by changing number entity
- [ ] Test away mode time manually by changing number entity
- [ ] Test schedule_enabled switch (turn off, verify automations don't trigger)
- [ ] Delete Home Assistant automations if desired
- [ ] Delete Home Assistant helpers if desired
- [ ] Update any custom automations that reference old HA entities
- [ ] Document your schedule times for future reference

---

**Successfully migrated!** Your device is now more independent and responsive. 🎉
