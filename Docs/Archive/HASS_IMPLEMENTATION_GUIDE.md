# Home Assistant Implementation Guide

## Overview

This guide explains which Home Assistant YAML files from the `homeassistant/` folder should be migrated to your HASS implementation and which are now obsolete due to ESPHome's native features.

## Quick Answer

**What to integrate into HASS:**
1. ✅ `configuration.yaml` - Template climate entity (thermostat card)
2. ✅ `dashboard_card.yaml` - UI card for schedule control
3. ❌ `automations.yaml` - **NOT needed** (ESPHome now handles scheduling)
4. ❌ Schedule helpers - **NOT needed** (ESPHome now handles them)

## Detailed File Analysis

### 1. configuration.yaml - **MOVE TO HASS**

**Status:** ✅ **REQUIRED** - Move this to your HASS implementation

**What it does:**
- Creates template climate entity for thermostat UI card
- Displays current temperature, desired temperature, heating status
- Allows users to adjust setpoint and mode from HA UI
- Shows preset modes (home/away)

**Content:**
```yaml
template:
  - climate:
      name: "Propane Stove Thermostat"
      current_temperature: "{{ states('sensor.gazebo_temperature') | float }}"
      target_temperature: "{{ states('number.desired_temperature') | float }}"
      # ... more configuration
```

**Why it's needed:**
- Provides nice thermostat card visualization in HASS
- Unifies temperature control UI
- Allows HA automations to interact if needed

**How to implement:**
1. Copy the `template:` section from the file
2. Add it to your HASS `configuration.yaml` (or split config)
3. Update entity IDs to match your device naming
4. Restart Home Assistant

**Entity ID updates needed:**
- `sensor.gazebo_temperature` → Your actual temperature sensor entity
- `number.desired_temperature` → Your desired temp entity
- `switch.propane_stove_relay` → Your relay entity
- `select.presence_mode` → Your presence mode entity
- `switch.manual_stop` → Your manual stop entity

---

### 2. automations.yaml - **DO NOT USE**

**Status:** ❌ **OBSOLETE** - Schedule now handled by ESPHome

**Why it's obsolete:**
- ESPHome now implements the schedule natively (see `SCHEDULE_IMPLEMENTATION_ANALYSIS.md`)
- Device no longer depends on Home Assistant for scheduling
- Automations would conflict with ESPHome's 1-minute interval checks
- Would cause redundant mode changes (HA + ESPHome triggering same actions)

**Old content (for reference):**
```yaml
- alias: "Gazebo: Set Home Mode at Time"
  trigger:
    platform: time
    at: input_datetime.home_mode_time  # From HA helper
  action:
    service: select.select_option
    target:
      entity_id: select.gazebo_stove_presence_mode
    data:
      option: "home"
```

**Why this is now handled by ESPHome:**
```yaml
# ESPHome automation.yaml (1-minute interval)
interval:
  - interval: 1min
    then:
      - if:
          condition:
            - lambda: |-
                // Check if current time matches home_mode_hour/minute
                // (from number entities, not HA helpers)
          then:
            - presence_mode.publish_state("home")
```

**Decision:** Delete or disable these automations in HASS.

---

### 3. dashboard_card.yaml - **MOVE TO HASS** (Optional)

**Status:** ✅ **OPTIONAL** - Provides convenient UI card

**What it does:**
Creates a nice dashboard card with:
- Heating Schedule toggle
- Home Mode Time display
- Away Mode Time display
- Current Presence Mode
- Emergency Heat switch
- Force Off switch

**Example card:**
```yaml
type: entities
title: Gazebo Stove - Heating Schedule
entities:
  - entity: switch.heating_schedule       # From ESPHome now!
  - entity: number.home_mode_time_hour    # From ESPHome now!
  - entity: number.home_mode_time_minute  # From ESPHome now!
  # ... etc
```

**Why it's useful:**
- One convenient place to see all controls
- Easy to toggle schedule on/off
- Easy to view/adjust schedule times
- Nice visual organization

**How to implement:**
1. Edit your HASS dashboard
2. Add Manual card
3. Copy the YAML from `dashboard_card.yaml`
4. Update entity IDs to match your entities

**Entity ID updates needed:**
- `switch.heating_schedule` ✅ **Now from ESPHome** (was HA input_boolean)
- `number.home_mode_time_hour` ✅ **Now from ESPHome** (was HA input_datetime split)
- `number.home_mode_time_minute` ✅ **Now from ESPHome** (was HA input_datetime split)
- Same for away mode times
- `select.presence_mode` → Your actual entity
- `switch.manual_run` → Your actual entity
- `switch.manual_stop` → Your actual entity

---

### 4. Other Files

**configuration_SIMPLE.yaml** - Older version, skip
**automations_FIXED.yaml** - Fixed version of old automations, still obsolete (ESPHome handles this now)
**automations_UPDATED.yaml** - Updated version, still obsolete
**dashboard_card_FIXED.yaml** - Older card version, use `dashboard_card.yaml` instead

---

## Migration Summary Table

| File | Action | Reason |
|------|--------|--------|
| `configuration.yaml` | ✅ MOVE | Thermostat UI card needed |
| `automations.yaml` | ❌ DELETE | ESPHome handles schedule now |
| `automations_FIXED.yaml` | ❌ DELETE | Obsolete (same issue) |
| `automations_UPDATED.yaml` | ❌ DELETE | Obsolete (same issue) |
| `dashboard_card.yaml` | ✅ MOVE | Optional but recommended |
| `dashboard_card_FIXED.yaml` | ❌ SKIP | Use dashboard_card.yaml instead |
| `configuration_SIMPLE.yaml` | ❌ SKIP | Older version |
| `README.md` | ✓ READ | Reference documentation |
| `QUICK_START.md` | ✓ READ | Setup instructions |

---

## Implementation Steps

### Step 1: Add Template Climate Entity

**File:** Your HASS `configuration.yaml` or split config

```yaml
template:
  - climate:
      - name: "Propane Stove Thermostat"
        unique_id: gazebo_propane_stove_thermostat
        current_temperature: "{{ states('sensor.gazebo_temperature') | float }}"
        target_temperature: "{{ states('number.desired_temperature') | float }}"
        target_temperature_step: 0.5
        min_temp: 5
        max_temp: 35
        hvac_modes:
          - "heat"
          - "off"
        preset_modes:
          - "home"
          - "away"
        preset_mode: "{{ 'home' if states('select.presence_mode') == 'home' else 'away' }}"
        temperature_unit: C
        set_temperature:
          service: number.set_value
          target:
            entity_id: number.desired_temperature
          data:
            value: "{{ temperature }}"
        set_preset_mode:
          service: select.select_option
          target:
            entity_id: select.presence_mode
          data:
            option: "{{ preset_mode }}"
        hvac_action: >-
          {% if states('switch.propane_stove_relay') == 'on' %}
            heating
          {% elif states('select.presence_mode') == 'away' %}
            off
          {% elif states('switch.manual_stop') == 'on' %}
            off
          {% else %}
            idle
          {% endif %}
```

**Required updates:**
- `sensor.gazebo_temperature` - Your temp sensor entity
- `number.desired_temperature` - Your setpoint entity
- `switch.propane_stove_relay` - Your relay entity
- `select.presence_mode` - Your presence mode entity
- `switch.manual_stop` - Your force off entity

### Step 2: Add Thermostat Card to Dashboard

**In HASS Dashboard:**
1. Edit dashboard
2. Add card → Manual
3. Copy `dashboard_card.yaml` content
4. Update entity IDs
5. Save

---

## What Changed and Why

### Before: Schedule in Home Assistant

```
Home Assistant
├─ input_boolean: gazebo_schedule_enabled
├─ input_datetime: gazebo_home_mode_time
├─ input_datetime: gazebo_away_mode_time
└─ automation: Set mode at scheduled time
   └─ Requires HA to be running!
```

### After: Schedule in ESPHome

```
ESPHome
├─ switch.heating_schedule (enable/disable)
├─ number.home_mode_time_hour/minute
├─ number.away_mode_time_hour/minute
└─ interval automation: Check time every minute
   └─ Works even if HA is offline!

Home Assistant
└─ Just displays these entities (no control needed)
```

**Benefit:** Device is now independent - doesn't need HA for scheduling.

---

## Automations: Why They're Obsolete

### Old Problem (HA automations)
```
Time arrives
  ↓
HA automation triggers (if HA is running)
  ↓
Sends command to ESPHome
  ↓
ESPHome receives and acts
```

**Issues:**
- ❌ Requires Home Assistant to be running
- ❌ Network latency between HA and device
- ❌ What if HA is updating or restarting?

### New Solution (ESPHome automations)
```
Time arrives
  ↓
ESPHome 1-minute interval checks
  ↓
Detects match with configured time
  ↓
Immediately updates presence_mode
```

**Advantages:**
- ✅ Works independently (no HA required)
- ✅ Instant response (no network latency)
- ✅ Always running (even if HA restarts)

---

## Entity ID Mapping

The following entities are now coming from ESPHome instead of Home Assistant:

| Entity | Old Source | New Source | Still in HASS? |
|--------|-----------|-----------|---|
| `switch.heating_schedule` | `input_boolean.gazebo_schedule_enabled` | ESPHome | ❌ Remove |
| `number.home_mode_time_hour` | Part of `input_datetime.gazebo_home_mode_time` | ESPHome | ❌ Remove |
| `number.home_mode_time_minute` | Part of `input_datetime.gazebo_home_mode_time` | ESPHome | ❌ Remove |
| `number.away_mode_time_hour` | Part of `input_datetime.gazebo_away_mode_time` | ESPHome | ❌ Remove |
| `number.away_mode_time_minute` | Part of `input_datetime.gazebo_away_mode_time` | ESPHome | ❌ Remove |
| Schedule automations | `automations.yaml` | ESPHome interval | ❌ Delete |

**Action:** Remove or ignore the old HA helpers/automations. ESPHome now provides all schedule functionality.

---

## Climate Entity Notes

The `template: climate:` entity is still needed in HASS for:
- ✅ Nice thermostat card UI
- ✅ Using HA's climate automations (if desired)
- ✅ Voice assistant integration
- ✅ Dashboard visualization

It reads from ESPHome entities but doesn't control them:
- ESPHome climate platform handles actual temperature control
- HASS template just displays and routes user input

**Example flow:**
```
User adjusts setpoint in HASS thermostat card
  ↓
Calls number.set_value on desired_temperature
  ↓
ESPHome receives update
  ↓
Triggers priority system evaluation
  ↓
Takes appropriate relay action
```

---

## Checklist for HASS Implementation

- [ ] Copy `configuration.yaml` template climate section to your HASS config
- [ ] Update all entity IDs in climate template to match your setup
- [ ] Restart Home Assistant
- [ ] Verify thermostat card appears and shows correct temp
- [ ] Test adjusting setpoint in HA card
- [ ] Delete or disable old schedule automations from HASS
- [ ] Delete old HA schedule helpers (input_boolean, input_datetime)
- [ ] (Optional) Add dashboard card with updated entity IDs
- [ ] Test that ESPHome schedule works independently
- [ ] (Optional) Create HA automations based on presence_mode if desired

---

## If You Want HA-Based Scheduling (Optional)

If you prefer to keep Home Assistant in control of scheduling:

1. Don't delete the automations
2. Disable ESPHome schedule (turn off `switch.heating_schedule`)
3. Keep the old HA helpers
4. Continue using `automations.yaml`

**But why not recommended:**
- Requires HA to be running
- Extra complexity (HA + ESPHome both managing presence)
- Defeats the purpose of independent device

---

## Summary

| What | Status | Action |
|------|--------|--------|
| **Thermostat UI card** | ✅ Move | Add template climate from `configuration.yaml` |
| **Dashboard card** | ✅ Optional | Add from `dashboard_card.yaml` |
| **Schedule helpers** | ❌ Remove | ESPHome now provides these |
| **Schedule automations** | ❌ Delete | ESPHome handles scheduling natively |

---

## Questions?

Refer to:
- `SCHEDULE_MIGRATION_GUIDE.md` - Why schedule moved to ESPHome
- `SCHEDULE_IMPLEMENTATION_ANALYSIS.md` - Why this is the optimal approach
- `EVENT_DRIVEN_PRIORITY_SYSTEM.md` - How priority system works
