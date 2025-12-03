# Quick Start Guide - Priority 3 Schedule Implementation

**Estimated Time**: 15-30 minutes  
**Difficulty**: Easy (UI method) or Medium (YAML method)

---

## Prerequisites

✅ ESPHome device is connected to Home Assistant  
✅ Priority 1 (Force Off) is implemented  
✅ Priority 2 (Emergency Heat) is implemented  
✅ Presence Mode entity exists in Home Assistant

---

## Method 1: UI Method (Easiest - 15 minutes)

### Step 1: Create Time Helpers (5 min)

1. **Settings → Devices & Services → Helpers**
2. Click **+ CREATE HELPER**
3. Select **Time**
4. Create:
   - **Name**: `Gazebo Home Mode Time`
   - **Icon**: `mdi:home-clock`
   - **Initial**: `06:00`
5. Repeat for:
   - **Name**: `Gazebo Away Mode Time`
   - **Icon**: `mdi:clock-out`
   - **Initial**: `22:00`

### Step 2: Create Home Mode Automation (5 min)

1. **Settings → Automations & Scenes → Automations**
2. Click **+ CREATE AUTOMATION**
3. **Name**: `Gazebo: Set Home Mode at Time`
4. **Trigger**: 
   - Type: `Time`
   - At: `input_datetime.gazebo_home_mode_time`
5. **Action**:
   - Service: `select.select_option`
   - Target: `select.presence_mode` (find your ESPHome device's presence mode)
   - Option: `home`
6. **Save**

### Step 3: Create Away Mode Automation (5 min)

1. **+ CREATE AUTOMATION**
2. **Name**: `Gazebo: Set Away Mode at Time`
3. **Trigger**: 
   - Type: `Time`
   - At: `input_datetime.gazebo_away_mode_time`
4. **Action**:
   - Service: `select.select_option`
   - Target: `select.presence_mode`
   - Option: `away`
5. **Save**

### Step 4: Test (Optional - 5 min)

1. Set Home Mode Time to 2 minutes from now
2. Set Presence Mode to "away"
3. Wait and verify it changes to "home"
4. Repeat for Away Mode Time

**Done!** ✅

---

## Method 2: YAML Method (Faster if you know YAML)

### Step 1: Add to configuration.yaml

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

**Restart Home Assistant**

### Step 2: Add to automations.yaml

```yaml
- alias: "Gazebo: Set Home Mode at Time"
  description: "Automatically set presence to home at configured time"
  trigger:
    - platform: time
      at: input_datetime.gazebo_home_mode_time
  action:
    - service: select.select_option
      target:
        entity_id: select.presence_mode  # UPDATE THIS!
      data:
        option: "home"
  mode: single

- alias: "Gazebo: Set Away Mode at Time"
  description: "Automatically set presence to away at configured time"
  trigger:
    - platform: time
      at: input_datetime.gazebo_away_mode_time
  action:
    - service: select.select_option
      target:
        entity_id: select.presence_mode  # UPDATE THIS!
      data:
        option: "away"
  mode: single
```

**IMPORTANT**: Update `select.presence_mode` to match your ESPHome device name!

**Restart Home Assistant**

**Done!** ✅

---

## Finding Your Entity ID

1. **Settings → Devices & Services**
2. Find your ESPHome device
3. Click on it
4. Look for "Presence Mode" entity
5. Copy the full entity ID (e.g., `select.gazebo_stove_presence_mode`)

---

## Verification Checklist

- [ ] Time helpers created and visible in UI
- [ ] Can adjust times via Home Assistant
- [ ] Home Mode automation exists and is enabled
- [ ] Away Mode automation exists and is enabled
- [ ] Automations can be tested manually
- [ ] Presence Mode changes when automations run
- [ ] Relay turns OFF when Away Mode is set
- [ ] Relay can turn ON when Home Mode is set (if thermostat demands)

---

## Expected Behavior

**Timeline Example** (6:00 AM / 10:00 PM):
```
00:00 ─────────── 06:00 ───────────── 22:00 ───────── 24:00
      STANDBY           OPERATIONAL          STANDBY
      Relay OFF        Thermostat Active    Relay OFF
```

**Priority Order**:
1. Force Off (always wins)
2. Emergency Heat (15 min override)
3. **Schedule** ← This feature
4. Thermostat (normal operation)

---

## Troubleshooting

**Automation doesn't trigger?**
- Check time helper is set
- Check automation is enabled
- Check logs: Settings → System → Logs

**Presence Mode doesn't change?**
- Verify entity ID is correct
- Check option is exactly `"home"` or `"away"`
- Test automation manually

**Schedule doesn't work?**
- Verify Priority 1 and 2 are implemented in ESPHome
- Check ESPHome logs for priority decisions
- Verify `manage_heating` script checks `away_mode`

---

**Need Help?** See `README.md` for detailed instructions.

