# Fix: Template Climate Configuration Error

**Error Message:**
```
Invalid config for 'template' at configuration.yaml, line 53:
'climate' is an invalid option for 'template'
```

**Cause:** The `template: - climate:` syntax is only supported in Home Assistant 2024.6+. Earlier versions don't support template climate entities.

---

## Step 1: Check Your Home Assistant Version

1. Go to **Settings → About**
2. Look for "Home Assistant Core" version
3. Note your version number (e.g., "2024.5.0" or "2024.11.0")

---

## Step 2: Choose Your Solution

### If you're running Home Assistant 2024.6 or LATER

**Option A: Use Template Climate (New Feature)**

The template climate code is already in your configuration.yaml - just uncomment it:

1. Edit `homeassistant/configuration.yaml`
2. Find the section starting with `# template:`
3. Remove the `#` from those lines to uncomment
4. Restart Home Assistant
5. Done! The thermostat card will appear

**Lines to uncomment:** 113-151 in configuration.yaml

```yaml
template:
  - climate:
      - name: "Propane Stove Thermostat"
        unique_id: gazebo_propane_stove_thermostat
        # ... rest of configuration
```

---

### If you're running Home Assistant 2024.5 or EARLIER

Choose **ONE** of these options:

#### Option 1: Generic Thermostat Helper (Easiest)

**Setup via Home Assistant UI:**

1. Go to **Settings → Devices & Services → Helpers**
2. Click **+ CREATE HELPER**
3. Select **Generic Thermostat**
4. Fill in:
   - **Name:** "Propane Stove Thermostat"
   - **Heater entity:** `switch.propane_stove_relay` (adjust to match your device)
   - **Target temperature entity:** `number.desired_temperature`
   - **Current temperature sensor:** `sensor.gazebo_temperature`
   - **Min temperature:** 5°C
   - **Max temperature:** 35°C
   - **Target temperature:** 22°C (default)
   - **Cold tolerance:** 0.5°C
   - **Hot tolerance:** 0.5°C
5. Click **CREATE**
6. Done! Thermostat helper is created

**Pros:**
- No YAML editing needed
- Easy to update via UI
- Works on all HA versions

**Cons:**
- Less customization than template climate
- Manual relay control (ESPHome handles actual heating)

---

#### Option 2: Uncomment Generic Thermostat YAML

If you prefer YAML configuration:

1. Edit `homeassistant/configuration.yaml`
2. Find the section starting with `# climate:`
3. Remove the `#` from those lines to uncomment
4. Replace entity IDs with your actual device names:
   - `switch.propane_stove_relay` → your relay entity ID
   - `sensor.gazebo_temperature` → your temperature sensor ID
5. Restart Home Assistant
6. Done!

**Lines to uncomment:** 158-171 in configuration.yaml

```yaml
climate:
  - platform: generic_thermostat
    name: "Propane Stove Thermostat"
    heater: switch.propane_stove_relay
    target_sensor: sensor.gazebo_temperature
    min_temp: 5
    max_temp: 35
    ac_mode: false
    target_temp: 22
    cold_tolerance: 0.5
    hot_tolerance: 0.5
    min_cycle_duration:
      minutes: 5
    away_temp: 16
```

**Pros:**
- Works on all HA versions
- Version controlled in YAML
- Documented in config file

**Cons:**
- Requires YAML editing
- Less customizable than template climate

---

#### Option 3: Use ESPHome Climate (Preferred if Available)

If your ESPHome device already exposes a climate entity:

1. Check **Settings → Devices & Services → Entities**
2. Search for "climate"
3. Look for one of these entities:
   - `climate.gazebo_stove_propane_stove_thermostat`
   - `climate.propane_stove_thermostat`
   - Any other climate entity from your ESPHome device

**If found:**
- No additional configuration needed!
- The climate entity from ESPHome is ready to use
- Delete all the template/generic thermostat lines from configuration.yaml
- Restart Home Assistant

**If not found:**
- Check your ESPHome configuration includes a climate platform
- See `includes/entities.yaml` in the GazeboStove project

**Pros:**
- No duplicate entities
- All control logic in ESPHome
- Single source of truth

**Cons:**
- Requires ESPHome climate platform to be configured

---

## Step 3: Fix the Error

### Quick Fix (Disable the Error)

If you just want to disable the template section temporarily:

1. Edit `homeassistant/configuration.yaml`
2. Make sure ALL `template:` lines are commented out (have `#` at the start)
3. Make sure the entire `template:` section is commented
4. Restart Home Assistant

**Current state:** The configuration.yaml already has template climate commented out, so just restart HA.

---

## Step 4: Restart Home Assistant

1. Go to **Settings → System**
2. Click **Restart Home Assistant** (bottom right)
3. Wait 2-3 minutes for restart
4. Check your **Entities** list for your thermostat entity

---

## Verify It's Fixed

After restarting Home Assistant:

1. Check logs for errors:
   - Go to **Settings → System → Logs**
   - Search for "template" or "climate"
   - Should show no errors about 'climate' being invalid

2. Check for your thermostat entity:
   - Go to **Settings → Devices & Services → Entities**
   - Search for "thermostat" or "propane"
   - Should find your climate entity

---

## Entity ID Reference

Your climate entity will be one of these (depending on which option you chose):

| Option | Entity ID |
|--------|-----------|
| Template Climate | `climate.propane_stove_thermostat` |
| Generic Thermostat | `climate.propane_stove_thermostat` |
| Generic Helper | `climate.propane_stove_thermostat` (or custom name) |
| ESPHome | `climate.gazebo_stove_propane_stove_thermostat` |

Use this entity ID in your automations and dashboard cards.

---

## Troubleshooting

### Still Getting "Invalid config" Error

1. **Check for syntax errors:**
   - Go to Settings → Automations & Scenes → YAML
   - Click "Validate YAML"
   - Should show green checkmark

2. **Verify indentation:**
   - The comment lines should all start with `#`
   - The active lines (if uncommented) should have proper YAML indentation
   - No mixing of tabs and spaces

3. **Check file was saved:**
   - Did you actually save the file?
   - Did Home Assistant reload/restart?

4. **Clear browser cache:**
   - Press Ctrl+Shift+Delete (or Cmd+Shift+Delete on Mac)
   - Clear all cached data
   - Refresh Home Assistant

### Thermostat Entity Not Appearing

1. Check you chose a valid option (not leaving everything commented)
2. Check entity IDs are correct (Settings → Entities)
3. Check sensor/switch entities exist
4. Check for YAML errors (Settings → YAML validation)

### Wrong Entity IDs

Check your actual device entity IDs:

1. Go to **Settings → Devices & Services**
2. Find your ESPHome device (e.g., "Gazebo Stove")
3. Look at the entities list
4. Note the actual entity IDs (not the names)
5. Use those in your configuration

---

## Which Option Should I Choose?

| Situation | Recommendation |
|-----------|-----------------|
| Running HA 2024.6+ | Uncomment **Template Climate** (Option 3) |
| Running HA 2024.5 or earlier | Use **Generic Thermostat Helper** (Option 1) |
| ESPHome has climate entity | Use **ESPHome Climate** (Option 3) |
| Not sure which version | Use **Generic Thermostat Helper** (works everywhere) |

---

## After Fixing the Configuration

Your thermostat will be available as a climate entity. You can:

1. **Add to Dashboard:**
   - Edit dashboard
   - Add "Thermostat" card
   - Select your climate entity

2. **Use in Automations:**
   ```yaml
   service: climate.set_temperature
   target:
     entity_id: climate.propane_stove_thermostat
   data:
     temperature: 22
   ```

3. **Use with Better Thermostat Card:**
   - Install "Better Thermostat Card" via HACS
   - Add to dashboard with your climate entity

---

## Summary

1. **Current state:** The erroneous template climate config is already commented out
2. **Just restart Home Assistant:** The error should go away
3. **Choose an option:** Template (HA 2024.6+), Generic (HA 2024.5-), or ESPHome
4. **Implement your choice:** Uncomment or create via UI
5. **Verify:** Check entities list for your thermostat

Your configuration is now compatible with all Home Assistant versions!

