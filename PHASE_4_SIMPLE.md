# Phase 4: Simple Time-Based Automations

**Status**: Planning - Ready to implement

**Scope**: Two simple automations for home/away mode scheduling

**Estimated Time**: 1-2 hours

---

## Overview

Phase 4 adds just two automations to enable simple time-based home/away control:

1. **"Set Home Mode" at a specific time** (e.g., 6:00 AM)
2. **"Set Away Mode" at a specific time** (e.g., 10:00 PM)

No complex scheduling. No temperature adjustments. Just these two times.

---

## Implementation Steps

### Step 1: Create Time Helpers in Home Assistant

In Home Assistant UI, go to: **Settings → Devices & Services → Helpers → Create Automation**

Or add to `automations.yaml`:

```yaml
input_datetime:
  home_mode_time:
    name: "Home Mode Time"
    has_date: false
    has_time: true
    icon: mdi:home

  away_mode_time:
    name: "Away Mode Time"
    has_date: false
    has_time: true
    icon: mdi:car
```

**Result**: Two time pickers in Home Assistant where user can set:
- When to automatically switch to Home mode (default: 6:00 AM)
- When to automatically switch to Away mode (default: 10:00 PM)

---

### Step 2: Create "Home Mode" Automation

**Automation Name**: "Gazebo: Set Home Mode at Time"

```yaml
automation:
  - alias: "Gazebo: Set Home Mode at Time"
    description: "Automatically set presence to home at configured time"

    trigger:
      - platform: time
        at: input_datetime.home_mode_time

    condition: []

    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "home"

      - service: logger.log
        data:
          level: INFO
          message: "Gazebo: Automatic home mode enabled"

      - service: notify.notify
        data:
          title: "Gazebo Stove"
          message: "Home mode enabled automatically"

    mode: single
```

**What it does**:
- At the configured time, sets Presence Mode to "home"
- Logs the action
- Sends notification to user

---

### Step 3: Create "Away Mode" Automation

**Automation Name**: "Gazebo: Set Away Mode at Time"

```yaml
automation:
  - alias: "Gazebo: Set Away Mode at Time"
    description: "Automatically set presence to away at configured time"

    trigger:
      - platform: time
        at: input_datetime.away_mode_time

    condition: []

    action:
      - service: select.select_option
        target:
          entity_id: select.presence_mode
        data:
          option: "away"

      - service: logger.log
        data:
          level: INFO
          message: "Gazebo: Automatic away mode enabled"

      - service: notify.notify
        data:
          title: "Gazebo Stove"
          message: "Away mode enabled automatically"

    mode: single
```

**What it does**:
- At the configured time, sets Presence Mode to "away"
- Logs the action
- Sends notification to user

---

## User Interface

In Home Assistant, create a dashboard card for easy access:

```yaml
type: entities
title: Gazebo Stove - Schedule
entities:
  - entity: input_datetime.home_mode_time
    name: "Automatic Home Mode"
  - entity: input_datetime.away_mode_time
    name: "Automatic Away Mode"
  - entity: select.presence_mode
    name: "Current Mode"
```

**Result**: User sees three controls:
- Time picker for home mode (e.g., 6:00 AM)
- Time picker for away mode (e.g., 10:00 PM)
- Current mode indicator

Users can change the times anytime without restarting anything.

---

## How It Works

**Daily Schedule Example**:
```
6:00 AM  → Automation triggers → Presence Mode = "home"
          → Heating resumes to thermostat control
          → Notification: "Home mode enabled"

10:00 PM → Automation triggers → Presence Mode = "away"
          → Heating turns off (energy saving)
          → Notification: "Away mode enabled"
```

**Manual Override**:
- If user toggles Presence Mode manually → automation still runs at scheduled time
- If user enables Manual Stop → automation still runs, but Manual Stop takes priority
- If user enables Manual Run → automation still runs at its time

---

## Safety Features

✅ **Automations never override Manual Stop** - Highest priority for maintenance safety

✅ **Manual modes always work** - User can toggle anytime

✅ **Notifications keep user informed** - Knows when automation ran

✅ **Easy to disable** - Just delete automations if not needed

✅ **Easy to change times** - No recompile needed, just adjust time pickers

---

## Testing Checklist

- [ ] Home mode automation runs at configured time
- [ ] Away mode automation runs at configured time
- [ ] Presence mode changes to home/away
- [ ] Notifications received when automations run
- [ ] Manual mode overrides still work
- [ ] Times can be changed in HA UI without issue

---

## Example Scenarios

**Scenario 1: Normal Day**
```
6:00 AM  → Home mode enabled → Thermostat can heat house
6:30 AM  → User wakes up in warm house
10:00 PM → Away mode enabled → Heating shuts off
          → Saves energy overnight
```

**Scenario 2: Manual Override**
```
6:00 AM  → Home mode enabled automatically
8:00 AM  → User toggles to "away" manually (early departure)
          → Overrides scheduled home time
10:00 PM → Away mode automation still runs (no change, already away)
```

**Scenario 3: Emergency**
```
2:00 PM  → User toggles Manual Run (emergency heat)
3:00 PM  → Timer expires, Manual Run disabled
3:00 PM  → Away mode automation runs (if it's 10 PM, or doesn't if not)
          → Normal operation resumes
```

---

## Files to Modify

### Option A: In Home Assistant UI
1. Create automations via UI: Settings → Automations → Create
2. Create helpers via UI: Settings → Devices & Services → Helpers

### Option B: In YAML
Add to your `automations.yaml`:
```yaml
# Copy the two automations from Step 2 & 3 above
```

Add to your `configuration.yaml`:
```yaml
input_datetime:
  home_mode_time:
    name: "Home Mode Time"
    has_date: false
    has_time: true

  away_mode_time:
    name: "Away Mode Time"
    has_date: false
    has_time: true
```

---

## Deployment Steps

1. **Create time helpers** in Home Assistant
2. **Create automation 1** - Home mode at time
3. **Create automation 2** - Away mode at time
4. **Test** - Wait for scheduled times or manually trigger
5. **Adjust times** as needed via Home Assistant UI
6. **Done** - No device recompile needed!

---

## Complete Example (Copy-Paste Ready)

Add this to your Home Assistant `automations.yaml`:

```yaml
- alias: "Gazebo: Set Home Mode at Time"
  description: "Automatically set presence to home at configured time"
  trigger:
    platform: time
    at: input_datetime.home_mode_time
  action:
    - service: select.select_option
      target:
        entity_id: select.presence_mode
      data:
        option: "home"
    - service: notify.notify
      data:
        title: "Gazebo Stove"
        message: "Home mode enabled automatically"

- alias: "Gazebo: Set Away Mode at Time"
  description: "Automatically set presence to away at configured time"
  trigger:
    platform: time
    at: input_datetime.away_mode_time
  action:
    - service: select.select_option
      target:
        entity_id: select.presence_mode
      data:
        option: "away"
    - service: notify.notify
      data:
        title: "Gazebo Stove"
        message: "Away mode enabled automatically"
```

And this to your `configuration.yaml`:

```yaml
input_datetime:
  home_mode_time:
    name: "Home Mode Time"
    has_date: false
    has_time: true
    icon: mdi:home
    initial_value: "06:00:00"

  away_mode_time:
    name: "Away Mode Time"
    has_date: false
    has_time: true
    icon: mdi:car
    initial_value: "22:00:00"
```

---

## Summary

**What You Get**:
- ✅ Automatic home mode at 6:00 AM (configurable)
- ✅ Automatic away mode at 10:00 PM (configurable)
- ✅ Notifications when automations run
- ✅ Easy time adjustment via Home Assistant UI
- ✅ No ESPHome firmware changes needed
- ✅ No complicated scheduling logic

**Implementation Time**: 1-2 hours

**Complexity**: Minimal - just two time-based automations

---

**Ready to implement?** Just add these automations to Home Assistant and you're done!
