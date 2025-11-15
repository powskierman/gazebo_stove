# Thermostat Not Working - Troubleshooting Guide

## Quick Checks

### 1. Check Force Off Switch
**MOST COMMON ISSUE**: Force Off switch is ON, which blocks ALL heating (Priority 1)

**Fix**: 
- Go to Home Assistant dashboard
- Find "Force Off" switch
- Turn it **OFF**
- Thermostat should now work

### 2. Check Emergency Heat Switch
**Issue**: Emergency Heat switch is ON, which blocks thermostat (Priority 2)

**Fix**:
- Turn Emergency Heat switch **OFF**
- Thermostat should resume control

### 3. Check Schedule/Presence Mode
**Issue**: Presence Mode is set to "away", which blocks thermostat (Priority 3)

**Fix**:
- Set Presence Mode to **"home"**
- Or turn schedule toggle **OFF** if you want manual control

### 4. Set Temperature via Climate Entity (Thermostat Card)
**IMPORTANT**: The ESPHome climate platform has its own target temperature, separate from the number entity.

**Fix**:
- Use the **Thermostat Card** in Home Assistant to set target temperature
- Do NOT use the "Desired Temperature" number entity slider
- The thermostat card will show current temp, target temp, and heating status

## How to Set Target Temperature

### Method 1: Via Thermostat Card (CORRECT)
1. Open Home Assistant dashboard
2. Find "Propane Stove Thermostat" card
3. Use the slider or +/- buttons to set target temperature
4. Climate platform will control heating based on this temperature

### Method 2: Via Number Entity (NOT RECOMMENDED)
- The "Desired Temperature" number entity exists but doesn't control the climate platform
- It's for display/reference only
- You must set temperature via the thermostat card

## Priority System Check

Make sure these are OFF/disabled for thermostat to work:

1. ✅ **Force Off** = OFF
2. ✅ **Emergency Heat** = OFF  
3. ✅ **Presence Mode** = "home" (not "away")
4. ✅ **Schedule Toggle** = OFF (if you want manual control)

## Debugging Steps

### Check ESPHome Logs
1. Go to ESPHome dashboard
2. Open device logs
3. Look for messages like:
   - `"PRIORITY: No priority override - climate platform controls heating"` ✅ Good
   - `"Force Off: Relay OFF (priority 1)"` ❌ Force Off is blocking
   - `"Emergency Heat: Relay ON (priority 2)"` ❌ Emergency Heat is blocking
   - `"Schedule: Standby mode - Relay OFF (priority 3)"` ❌ Away mode is blocking

### Check Current Temperature vs Target
- Current temp must be **below** target temp for heating to start
- Climate platform uses symmetric deadband (±hysteresis/2)
- Example: Target=25°C, Hysteresis=2°C
  - Turns ON when: temp < 24°C (25 - 2/2)
  - Turns OFF when: temp > 26°C (25 + 2/2)

## Still Not Working?

1. **Restart ESPHome device**: Sometimes the climate platform needs a restart
2. **Check relay state**: Verify relay can be controlled manually
3. **Check temperature sensor**: Make sure sensor is reading correctly
4. **Check climate entity**: Verify "Propane Stove Thermostat" entity exists in Home Assistant

## Expected Behavior

When thermostat is working correctly:
- ✅ Relay turns ON when current temp < (target - hysteresis/2)
- ✅ Relay turns OFF when current temp > (target + hysteresis/2)
- ✅ Thermostat card shows "heating" when relay is ON
- ✅ Thermostat card shows "idle" when relay is OFF but temp is below target
- ✅ Priority overrides (Force Off, Emergency Heat, Away) still work

