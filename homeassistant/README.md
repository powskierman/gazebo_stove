# Home Assistant Configuration - Gazebo Stove Schedule

**Priority 3: Schedule (Home Mode Time / Away Mode Time)**

This directory contains Home Assistant configuration files for implementing the schedule feature. These files must be added to your Home Assistant instance, not the ESPHome device.

---

## Files

- **`configuration.yaml`** - Time input entities, sensor platforms, thermostat template
  - Includes: Schedule time helpers, date/time sensors, weather integration, thermostat UI
- **`automations.yaml`** - Two automations for time-based mode switching
- **`dashboard_card.yaml`** - Dashboard card for schedule control
- **`SENSOR_ENTITIES_GUIDE.md`** - Guide for date/time and weather sensors (NEW)
- **`README.md`** - This file

---

## Sensor Entities

This configuration includes date/time and weather sensors:

### Date/Time Sensors (time_date platform)
- **`sensor.date_time`** - Current date and time (YYYY-MM-DD HH:MM)
- **`sensor.date`** - Current date only (YYYY-MM-DD)
- **`sensor.time`** - Current time only (HH:MM)
- And 4 more format variations

**Setup:** No additional configuration needed - uses Home Assistant's system time

### Weather Sensors (openmeteo platform)
- **`weather.openmeteo`** - Weather forecast (5-14 days)
- **`sensor.openmeteo_temperature`** - Current outdoor temperature (°C)
- **`sensor.openmeteo_humidity`** - Current outdoor humidity (%)
- **`sensor.openmeteo_wind_speed`** - Wind speed (m/s)
- And more weather-related attributes

**Setup:** After adding to YAML, configure via Settings → Integrations → Open-Meteo (free API, no key needed)

**See:** [SENSOR_ENTITIES_GUIDE.md](SENSOR_ENTITIES_GUIDE.md) for detailed setup instructions

---

## Installation Instructions

### Option 1: Via Home Assistant UI (Recommended)

#### Step 1: Create Time Helpers

1. Open Home Assistant
2. Go to **Settings → Devices & Services → Helpers**
3. Click **+ CREATE HELPER** button
4. Select **Time**
5. Create two helpers:
   - **Name**: `Gazebo Home Mode Time`
   - **Icon**: `mdi:home-clock`
   - **Initial Value**: `06:00`
   
   - **Name**: `Gazebo Away Mode Time`
   - **Icon**: `mdi:clock-out`
   - **Initial Value**: `22:00`

#### Step 2: Create Automations

1. Go to **Settings → Automations & Scenes → Automations**
2. Click **+ CREATE AUTOMATION**
3. Create first automation:
   - **Name**: `Gazebo: Set Home Mode at Time`
   - **Trigger**: Time → At → Select `input_datetime.gazebo_home_mode_time`
   - **Action**: Call Service → `select.select_option`
     - **Target**: `select.presence_mode` (update to match your ESPHome device)
     - **Option**: `home`
4. Create second automation:
   - **Name**: `Gazebo: Set Away Mode at Time`
   - **Trigger**: Time → At → Select `input_datetime.gazebo_away_mode_time`
   - **Action**: Call Service → `select.select_option`
     - **Target**: `select.presence_mode` (update to match your ESPHome device)
     - **Option**: `away`

#### Step 3: Add Dashboard Card

1. Go to your Dashboard
2. Click **Edit Dashboard** (pencil icon)
3. Click **+ ADD CARD**
4. Choose **Manual** or **Entities**
5. Copy contents from `dashboard_card.yaml` and paste
6. Update entity IDs to match your ESPHome device names

---

### Option 2: Via YAML Files

#### Step 1: Add Time Input Entities

Add the contents of `configuration.yaml` to your Home Assistant `configuration.yaml` file:

```yaml
input_datetime:
  gazebo_home_mode_time:
    name: "Gazebo Home Mode Time"
    has_date: false
    has_time: true
    icon: mdi:home-clock
    initial: "06:00:00"

  gazebo_away_mode_time:
    name: "Gazebo Away Mode Time"
    has_date: false
    has_time: true
    icon: mdi:clock-out
    initial: "22:00:00"
```

Then restart Home Assistant or reload configuration.

#### Step 2: Add Automations

Add the contents of `automations.yaml` to your Home Assistant `automations.yaml` file.

**IMPORTANT**: Update the `entity_id` in both automations to match your ESPHome device name:
- Check your Home Assistant entities list for the correct name
- Common names: `select.gazebo_stove_presence_mode`, `select.presence_mode`
- The entity name depends on your ESPHome device name

Then restart Home Assistant or reload automations.

#### Step 3: Add Dashboard Card

Add the card from `dashboard_card.yaml` to your dashboard YAML or via the UI.

**IMPORTANT**: Update all entity IDs in the card to match your ESPHome device names.

---

## Finding Your Entity IDs

To find the correct entity IDs:

1. Go to **Settings → Devices & Services**
2. Find your ESPHome device (e.g., "Gazebo Stove" or "gazebo-stove")
3. Click on the device
4. Look for entities:
   - `select.presence_mode` or `select.gazebo_stove_presence_mode`
   - `switch.manual_run` or `switch.gazebo_stove_manual_run`
   - `switch.manual_stop` or `switch.gazebo_stove_manual_stop`

Update the entity IDs in the automations and dashboard card accordingly.

---

## How It Works

### Schedule Timeline

```
00:00 ─────────── 06:00 ───────────── 22:00 ───────── 24:00
      STANDBY           OPERATIONAL          STANDBY
      (Away)           (Home)               (Away)
```

### Priority Hierarchy

1. **Priority 1**: Force Off (Manual Stop) - HIGHEST
2. **Priority 2**: Emergency Heat (Manual Run)
3. **Priority 3**: Schedule (Home Mode Time / Away Mode Time) ← This feature
4. **Priority 4**: Thermostat - LOWEST

### Behavior

- **Home Mode Time** (e.g., 6:00 AM): Automatically sets Presence Mode to "home"
  - Stove becomes operational
  - Thermostat can control heating
  
- **Away Mode Time** (e.g., 10:00 PM): Automatically sets Presence Mode to "away"
  - Stove goes to standby
  - Heating disabled (unless Emergency Heat is active)

### Manual Overrides

- User can manually change Presence Mode anytime
- Manual changes override schedule until next scheduled time
- Force Off and Emergency Heat still take precedence

---

## Testing

### Quick Test

1. Set Home Mode Time to 2 minutes from now
2. Set Presence Mode to "away"
3. Wait for automation to trigger
4. Verify Presence Mode changes to "home"
5. Repeat for Away Mode Time

### Real Test

1. Configure realistic times (e.g., 6:00 AM / 10:00 PM)
2. Monitor for 24 hours
3. Verify automations trigger at correct times
4. Verify relay behavior matches schedule

---

## Troubleshooting

**Q: Automation doesn't trigger**
- Check time helper is set correctly
- Check automation trigger shows correct time helper
- Check Home Assistant logs for errors
- Verify entity IDs are correct

**Q: Presence Mode doesn't change**
- Check automation target entity ID matches ESPHome device
- Check option is exactly `"home"` or `"away"` (case-sensitive)
- Try testing automation manually

**Q: Schedule doesn't override thermostat**
- Verify Priority 3 is correctly implemented in ESPHome
- Check `manage_heating` script priority order
- Verify Away Mode turns relay OFF

---

## Notes

- Schedule automations run in Home Assistant, not ESPHome
- No ESPHome device recompilation needed
- Times are user-configurable without code changes
- Schedule respects Priority 1 (Force Off) and Priority 2 (Emergency Heat)

---

**Status**: Ready for deployment  
**Priority**: 3 (Schedule)  
**Dependencies**: ESPHome Priority 1 and Priority 2 must be implemented first

