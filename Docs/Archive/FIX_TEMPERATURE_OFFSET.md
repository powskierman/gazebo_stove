# Quick Fix: Temperature Offset Set to -5°C

**Status:** SOLVED ✅
**Root Cause:** Temperature offset was -5.0°C (not moving average window)
**Fix:** Reset offset to 0.0°C

---

## The Problem

| Reading | Value |
|---------|-------|
| **ESPHome Log** | 20.1°C |
| **Home Assistant Display** | 15.1°C |
| **Difference** | 5.0°C |
| **Root Cause** | Offset = -5.0°C |

**Math:**
```
Displayed Temp = Actual Temp + Offset
15.1°C = 20.1°C + (-5.0°C)
```

---

## The Fix

### In Home Assistant UI:

1. Open **Settings** → **Devices & Services** → **Entities**

2. Search for `gazebo_stove_temperature_offset`

3. Click on the entity

4. Change the value from **-5.0** to **0.0**

5. Save

### Result:

```
Displayed Temp = 20.1°C + 0.0°C = 20.1°C ✓
```

---

## Why Was It Set to -5°C?

This offset exists to **calibrate** the temperature sensor if it's reading inaccurately. Someone must have set it to -5°C thinking the sensor was reading 5°C too high.

**To verify sensor accuracy:**
- Check if the DS18B20 sensor location is correct
- Compare with an external thermometer
- If readings are accurate, offset should be 0.0°C

---

## After Fix

Once you reset the offset to 0.0°C:
- ✅ Display will show actual temperature
- ✅ Thermostat control will work correctly
- ✅ Temperature-based heating (Priority 4) will function properly

---

## How to Avoid This in Future

If you ever need to calibrate the sensor:
- Only adjust offset if actual sensor is demonstrably wrong
- Document why offset was changed
- Check offset periodically to reset when sensor is recalibrated

**Typical offsets:**
- `0.0°C` = Sensor reading is accurate (most common)
- `-0.5°C` to `+0.5°C` = Minor calibration (acceptable)
- `±2.0°C` = Significant offset (check sensor placement first)
- `±5.0°C` = Likely an error or requires sensor replacement

---

Done! ✅ Temperature should now display correctly.
