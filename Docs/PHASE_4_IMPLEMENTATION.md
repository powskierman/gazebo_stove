# Phase 4 Implementation - Simple Time-Based Automations

**Status**: In Progress

**Date Started**: 2025-11-11

---

## Step 1: Create Time Helpers

### Option A: Via Home Assistant UI (Easiest)

1. Open Home Assistant
2. Go to **Settings → Devices & Services → Helpers**
3. Click **Create Automation → Time**

**Create First Helper: "Home Mode Time"**
- Name: `Home Mode Time`
- Icon: `mdi:home`
- Initial Value: `06:00`

**Create Second Helper: "Away Mode Time"**
- Name: `Away Mode Time`
- Icon: `mdi:car`
- Initial Value: `22:00`

Result: Two time pickers appear in Home Assistant UI where you can adjust times anytime.

---

### Option B: Via YAML (If you edit configuration)

Add this to your `configuration.yaml`:

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

Then restart Home Assistant or reload automations.

---

## Step 2: Create "Set Home Mode" Automation

### Option A: Via Home Assistant UI

1. Go to **Settings → Automations & Scenes → Automations**
2. Click **Create Automation**
3. Name: `Gazebo: Set Home Mode at Time`
4. **Trigger Section**:
   - Type: `Time`
   - At: `input_datetime.home_mode_time`
5. **Action Section**:
   - Service: `Select: Select Option`
   - Target: `select.presence_mode`
   - Option: `home`
6. Click **Create Automation**

---

### Option B: Via YAML

Add this to your `automations.yaml`:

```yaml
- alias: "Gazebo: Set Home Mode at Time"
  description: "Automatically set presence to home at configured time"

  trigger:
    platform: time
    at: input_datetime.home_mode_time

  condition: []

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
```

---

## Step 3: Create "Set Away Mode" Automation

### Option A: Via Home Assistant UI

1. Go to **Settings → Automations & Scenes → Automations**
2. Click **Create Automation**
3. Name: `Gazebo: Set Away Mode at Time`
4. **Trigger Section**:
   - Type: `Time`
   - At: `input_datetime.away_mode_time`
5. **Action Section**:
   - Service: `Select: Select Option`
   - Target: `select.presence_mode`
   - Option: `away`
6. Click **Create Automation**

---

### Option B: Via YAML

Add this to your `automations.yaml`:

```yaml
- alias: "Gazebo: Set Away Mode at Time"
  description: "Automatically set presence to away at configured time"

  trigger:
    platform: time
    at: input_datetime.away_mode_time

  condition: []

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

---

## Step 4: Add Controls to Dashboard

Create a new dashboard card in Home Assistant:

1. Go to your **Dashboard**
2. Click **Edit Dashboard** (pencil icon)
3. Click **Add Card**
4. Choose **Entities**
5. Add these entities:
   - `input_datetime.home_mode_time`
   - `input_datetime.away_mode_time`
   - `select.presence_mode`
6. Save

**Or paste this YAML** in a custom card:

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

Result: Users can see and adjust times easily!

---

## Step 5: Test the Automations

### Quick Test Method

1. Go to **Settings → Automations & Scenes → Automations**
2. Find `Gazebo: Set Home Mode at Time`
3. Click the three-dot menu
4. Select **Test**
   - Automation runs immediately
   - Presence Mode should change to "home"
   - Check logs for confirmation

5. Repeat for `Gazebo: Set Away Mode at Time`

### Real Test Method

1. Set `home_mode_time` to current time + 2 minutes
2. Wait for automation to run
3. Check that Presence Mode changed to "home"
4. Check notification was sent
5. Repeat for away mode

---

## Verification Checklist

- [ ] Time helpers created in Home Assistant
- [ ] Can change times via Home Assistant UI
- [ ] "Set Home Mode" automation exists
- [ ] "Set Away Mode" automation exists
- [ ] Both automations can be tested and run
- [ ] Presence Mode changes when automations run
- [ ] Notifications sent when automations run
- [ ] Controls visible on Home Assistant dashboard
- [ ] Manual mode changes still work (overrides automations)

---

## How to Use

**Daily Operation**:
1. User sets home mode time (e.g., 6:00 AM)
2. User sets away mode time (e.g., 10:00 PM)
3. Automations run automatically at those times
4. User can manually override anytime by toggling Presence Mode

**Adjusting Times**:
1. Go to Home Assistant
2. Find "Home Mode Time" and "Away Mode Time"
3. Click to edit and set new time
4. Changes take effect immediately (no recompile needed)

---

## Troubleshooting

**Q: Automation doesn't run at scheduled time**
- A: Check time helper is set correctly
- A: Check automation trigger shows the time helper
- A: Look at Home Assistant logs for errors

**Q: Presence Mode doesn't change**
- A: Check automation target entity is `select.presence_mode`
- A: Check option is exactly `"home"` or `"away"`
- A: Try testing automation manually

**Q: No notification appears**
- A: Check notification service is available in HA
- A: Verify notify service in automation YAML

---

## Next Steps

Once Phase 4 is complete:

1. **Monitor** for 7 days - ensure automations work daily
2. **Adjust times** as needed based on your routine
3. **Consider Phase 5** (future):
   - Weather-based adjustments
   - Presence sensor integration
   - Usage statistics

---

## Summary

✅ **What You Get**:
- Automatic Home Mode at configurable time
- Automatic Away Mode at configurable time
- User-friendly time pickers in Home Assistant
- Notifications when automations run
- Easy to adjust times anytime

✅ **No Changes Needed To**:
- ESPHome firmware
- Device configuration
- Existing controls (Manual Run/Stop/Thermostat)

✅ **Implementation Time**: 1-2 hours (mostly UI clicks)

---

**You're ready to implement Phase 4!**

Choose Option A (UI) or Option B (YAML) and follow the steps above.

Let me know when you're done with each step!
