# GazeboStove Troubleshooting & Debugging Guide

**Last Updated:** November 16, 2025
**Status:** Comprehensive consolidated guide covering all common issues and solutions

---

## 🚀 Quick Diagnosis (Start Here)

### Symptom: Stove Won't Turn On

**Most Common Cause:** One of the 4 priority overrides is blocking heating

| Check | Fix |
|-------|-----|
| **Force Off switch is ON?** | Turn "Manual Stop" switch OFF |
| **Emergency Heat is ON?** | Turn "Manual Run" switch OFF |
| **Presence Mode is "away"?** | Change to "home" or disable schedule |
| **Desired temp below current?** | Increase desired temperature |

### Symptom: Stove Won't Turn Off

**Most Common Cause:** Climate platform not responding to temperature changes

| Check | Fix |
|-------|-----|
| **Force Off is OFF?** | Good - problem elsewhere |
| **Desired temp above current?** | Lower desired temperature |
| **Relay gets stuck?** | Check min_heating_run_time (should be 30s) |

### Symptom: Temperature Not Updating

**Most Common Cause:** Sensor malfunction or offset issue

| Check | Fix |
|-------|-----|
| **Sensor malfunction alarm?** | Check one-wire connection, restart device |
| **Offset set to extreme value?** | Reset offset to 0 and recalibrate |
| **Current temp stuck?** | Restart ESPHome device |

---

## 📋 Detailed Troubleshooting by Component

### 1. RELAY CONTROL ISSUES

#### Issue: Relay stays ON despite desired < current

**Root Cause:** Climate platform's idle_action wasn't triggered

**Solution:**
1. Check climate entity state in Home Assistant
2. Verify desired_temp < current_temp
3. Wait 5-10 seconds (climate platform needs time to evaluate)
4. If still on, restart device: `esphome run gazebo_stove.yaml`

**Logs to Check:**
```
[DESIRED_TEMP:074]: Desired <= Current: Relay OFF  ✅ Correct
[PRIORITY:089]: Priority 4: Climate idle - Relay OFF  ✅ Correct
```

#### Issue: Relay won't turn ON when desired > current

**Root Cause:** One of 4 priorities is blocking heating

**Debug Steps:**
1. Check "Manual Stop" switch is OFF
2. Check "Manual Run" switch is OFF
3. Check "Presence Mode" = "home"
4. Check desired temperature > current temperature

**Logs to Check:**
```
[PRIORITY:068]: Priority 3: Away Mode - Relay OFF  ❌ Fix: Change mode to home
[PRIORITY:050]: Priority 1: Force Off - Relay OFF  ❌ Fix: Disable Manual Stop
[DESIRED_TEMP:068]: Setpoint changed to 25.0°C, current=19.2°C  ✅ Good
[PRIORITY:088]: Priority 4: Climate heating - Relay ON  ✅ Good
```

#### Issue: Relay cycles ON/OFF rapidly

**Root Cause:** Hysteresis too small or sensor bouncing

**Solution:**
```yaml
# In includes/entities.yaml, check these values:
min_heating_off_time: 5s      # Min time between cycles
min_heating_run_time: 30s     # Min run time before turning off
min_idle_time: 5s             # Min idle time before turning on

# In sensors.yaml, check temperature filter:
- delta: 1.0  # Max ±1°C change per reading (prevents bouncing)
```

**Logs to Check:**
```
[PRIORITY]: In deadband zone (desired 24.5, current 24.8)  ✅ Expected in transition
```

---

### 2. TEMPERATURE SENSOR ISSUES

#### Issue: "Sensor Malfunction" alarm triggered

**Root Cause:** Temperature sensor reading NaN (invalid) >10 times

**Solution:**
1. Check one-wire bus wiring (GPIO4)
2. Verify DS18B20 sensor is connected properly
3. Check for loose connectors
4. Restart ESPHome device
5. If persistent, may need to replace sensor

**Logs to Check:**
```
[TEMP_SENSOR]: Bad temperature read #11  ❌ Too many errors
[SENSOR:117]: Sensor Malfunction: true  ❌ Alarm triggered
[TEMP_SENSOR]: Sensor recovered - resetting bad read counter  ✅ Fixed
```

**Reset Procedure:**
- Restart device: `esphome run gazebo_stove.yaml`
- Check one-wire connections
- Wait 2 minutes for sensor to stabilize

#### Issue: Temperature stuck at same value

**Root Cause:** Sensor not updating or filter caching old value

**Solution:**
1. Restart device
2. Check update_interval: 10s is correct
3. Verify temperature is actually changing
4. Check temperature offset isn't set to extreme value

#### Issue: Temperature offset not updating

**Root Cause:** Offset value changed but sensor hasn't re-evaluated

**Solution:**
```
Wait 10-15 seconds for sensor to update with new offset
The moving average filter recalculates on each reading
```

**Logs to Check:**
```
[TEMP_OFFSET]: Offset changed to 0.0°C  ✅ Value updated
[CLIMATE_SYNC]: Climate state republished with temperature: 19.2°C  ✅ Synced
```

---

### 3. CLIMATE PLATFORM ISSUES

#### Issue: Climate entity not found in Home Assistant

**Root Cause:** ESPHome climate platform not initialized

**Solution:**
1. Verify `includes/entities.yaml` has climate section
2. Check device name is "gazebo-stove"
3. Restart Home Assistant after device restarts
4. Verify ESPHome integration is enabled

**Expected Entity Names:**
- `climate.gazebo_stove_propane_stove_thermostat` ✅

#### Issue: Climate shows "heating" but relay is OFF

**Root Cause:** Display state vs actual relay state mismatch

**Solution:**
This is actually CORRECT if:
- Desired temp < current temp + hysteresis
- Then climate thinks it should heat, but relay is off due to hysteresis deadband

**Logs to Check:**
```
[climate:402]: Mode: HEAT  (Climate thinks it should heat)
[PRIORITY:089]: Climate idle - Relay OFF  (But relay is off per priority)
```

This is expected behavior during temperature transition.

---

### 4. PRIORITY OVERRIDE ISSUES

#### Issue: Manual Stop doesn't turn off relay

**Root Cause:** Manual Stop switch state not updating

**Solution:**
1. Verify switch flips to ON in Home Assistant UI
2. Check device logs show "Manual Stop" state change
3. Restart device if UI shows ON but device shows OFF

**Logs to Check:**
```
[MANUAL_STOP:106]: Force Off enabled - Heating disabled  ✅ Good
[PRIORITY:050]: Priority 1: Force Off - Relay OFF  ✅ Good
```

#### Issue: Manual Run stops after a few seconds

**Root Cause:** 15-minute timer completed or manual stop is active

**Solution:**
1. Check if 15 minutes have actually elapsed
2. Verify "Manual Stop" is OFF (it blocks "Manual Run")
3. Manual Run automatically disables after 15 minutes

**Logs to Check:**
```
[MANUAL_RUN:082]: Activated - Relay ON (15-min delay timer started)  ✅ Good
[MANUAL_RUN:101]: 15-minute timeout reached - auto-disabling  ✅ Normal
```

#### Issue: Away Mode won't disable heating

**Root Cause:** Schedule is overriding manual mode selection

**Solution:**
1. Disable "Heating Schedule" switch
2. Or wait for home mode time to arrive (schedule resets at home time)
3. Check current time is actually in away window

**Logs to Check:**
```
[SCHEDULE:162]: In home window but mode manually set to away  ✅ Respecting manual
[PRIORITY:068]: Priority 3: Away Mode - Relay OFF  ✅ Away mode working
```

---

### 5. SCHEDULE AUTOMATION ISSUES

#### Issue: Schedule doesn't switch to away mode at scheduled time

**Root Cause:**
1. Schedule is disabled
2. Time is not synced
3. Mode is manually set and blocking schedule

**Solution:**
1. Verify "Heating Schedule" switch is ON
2. Verify device time is correct (check NTP sync)
3. Check presence mode isn't manually set to override schedule

**Logs to Check:**
```
[SCHEDULE:130]: Schedule disabled  ❌ Fix: Enable schedule
[SCHEDULE:170]: In away window but mode manually set to home  ❌ Manual override active
```

#### Issue: Schedule keeps changing mode unexpectedly

**Root Cause:** Manual override flag isn't set, schedule keeps forcing mode

**Solution:**
1. Check if this is expected behavior (schedule time window)
2. If you want manual control, keep "Heating Schedule" OFF
3. Or set presence mode manually to reset override flag

---

### 6. HOME ASSISTANT INTEGRATION ISSUES

#### Issue: Better Thermostat card shows wrong temperature

**Root Cause:** Cache not updated or climate entity not synced

**Solution:**
1. Refresh Home Assistant (F5)
2. Check "Climate platform target updated" logs
3. Verify "Desired Temperature" number entity matches climate target
4. Restart device if values stay out of sync

**Logs to Check:**
```
[TEMP_SYNC:048]: Climate platform target updated to 25.0°C  ✅ Synced
[number:033]: 'Desired Temperature': Sending state 25.0  ✅ UI updated
```

#### Issue: Automation doesn't trigger temperature change

**Root Cause:** Service call targeting wrong entity

**Solution:**
Verify automations use correct entity IDs:
```yaml
# CORRECT - Sets climate target (what you want):
service: climate.set_temperature
target:
  entity_id: climate.gazebo_stove_propane_stove_thermostat
data:
  temperature: 25

# WRONG - Sets display only:
service: number.set_value
target:
  entity_id: number.gazebo_stove_desired_temperature
data:
  value: 25
```

---

## 🔧 Advanced Debugging

### View Device Logs in Real-Time

```bash
cd /Users/michel/Documents/Electronics/Projects/GazeboStove
esphome logs gazebo_stove.yaml
```

### Filter for Specific Issues

```bash
# Priority system debugging
esphome logs gazebo_stove.yaml | grep PRIORITY

# Temperature debugging
esphome logs gazebo_stove.yaml | grep "TEMP\|Gazebo Temperature"

# Climate platform debugging
esphome logs gazebo_stove.yaml | grep -i "climate\|thermostat"

# Relay state changes
esphome logs gazebo_stove.yaml | grep "Relay\|relay"
```

### Key Log Messages by Component

**Climate Platform:**
```
[climate:011]: 'Propane Stove Thermostat' - Setting
[climate:040]: Target Temperature: XX.XX
[climate:402]: Mode: HEAT / OFF
[main:061]: THERMOSTAT: heat_action triggered
[main:212]: THERMOSTAT: idle_action triggered
```

**Priority System:**
```
[PRIORITY:040]: === APPLY_PRIORITY_OVERRIDES CALLED ===
[PRIORITY:050]: Priority 1: Force Off - Relay OFF
[PRIORITY:059]: Priority 2: Emergency Heat - Relay ON
[PRIORITY:068]: Priority 3: Away Mode - Relay OFF
[PRIORITY:088]: Priority 4: Climate heating - Relay ON
```

**Temperature Sensor:**
```
[TEMP_SENSOR]: Bad temperature read #X
[TEMP_SENSOR]: Sensor recovered - resetting bad read counter
[CLIMATE_SYNC]: Climate state republished with temperature
```

**Desired Temperature:**
```
[DESIRED_TEMP:068]: Setpoint changed to XX.X°C
[DESIRED_TEMP:074]: Desired <= Current: Relay OFF
[DESIRED_TEMP:080]: Desired > Current: Relay ON
```

---

## 📊 System Health Check

Run this checklist weekly to ensure system is healthy:

### Hardware
- [ ] WiFi LED blinking (device connected)
- [ ] Relay can be toggled manually
- [ ] Temperature sensor reading reasonable value
- [ ] No sensor malfunction alarm

### Firmware
- [ ] Device logs show no errors
- [ ] Climate entity present in Home Assistant
- [ ] Desired Temperature entity present
- [ ] All switches accessible (Manual Stop, Manual Run, etc.)

### Functionality
- [ ] Relay turns ON when desired > current
- [ ] Relay turns OFF when desired < current
- [ ] Manual Stop blocks heating
- [ ] Manual Run forces heating for 15 min
- [ ] Away mode disables heating
- [ ] Schedule switches modes at correct times

### Configuration
- [ ] Temperature offset is reasonable (±2°C)
- [ ] Desired temperature in valid range (5-35°C)
- [ ] Home/Away mode times are correct
- [ ] Hysteresis prevents relay cycling

---

## 🆘 If All Else Fails

### Emergency Procedures

**1. Force Device to Safe State:**
```bash
# Restart device
esphome run gazebo_stove.yaml

# This will:
# - Clear any state corruption
# - Reload all configurations
# - Reset timers
```

**2. Manual Relay Control:**
In Home Assistant:
1. Go to Developer Tools → Services
2. Call `switch.toggle` on `switch.gazebo_stove_propane_stove_relay`
3. Verify relay physically turns ON/OFF

**3. Check Factory Defaults:**
Verify your setup matches the examples in:
- `Mods/configuration.yaml` - HA config
- `Docs/CONFIG_MODULES.md` - Module structure
- `HASS_IMPLEMENTATION_GUIDE.md` - Integration setup

---

## 📞 Getting Help

When reporting issues, include:

1. **System Info:**
   - Device: ESP32 DOIT DEVKIT V1
   - ESPHome version: 2025.10.3
   - Home Assistant version: (if using)

2. **Current State:**
   - Current temperature: X°C
   - Desired temperature: X°C
   - Relay state: ON/OFF
   - Presence mode: home/away

3. **Recent Logs:**
   ```bash
   esphome logs gazebo_stove.yaml 2>&1 | tail -50
   ```

4. **Steps to Reproduce:**
   - What did you do?
   - What happened?
   - What did you expect?

---

## ✅ Verification Checklist

After fixing an issue, verify:

- [ ] Relay responds to temperature changes
- [ ] All 4 priority levels still work
- [ ] No sensor malfunction alarms
- [ ] Home Assistant shows correct entity states
- [ ] Device logs show expected messages
- [ ] No rapid ON/OFF cycling

---

**Document Status:** ✅ Comprehensive
**Last Review:** November 16, 2025
**Maintained:** Yes
**Cross-Referenced:** Yes

---

For more detailed information, see:
- [README.md](README.md) - System overview
- [TESTING_SUMMARY.md](TESTING_SUMMARY.md) - Test results
- [EVENT_DRIVEN_PRIORITY_SYSTEM.md](EVENT_DRIVEN_PRIORITY_SYSTEM.md) - Architecture
- [Docs/THERMOSTAT_TROUBLESHOOTING.md](Docs/THERMOSTAT_TROUBLESHOOTING.md) - Original thermostat guide

