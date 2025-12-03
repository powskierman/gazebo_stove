# Temperature Reading Regression - Analysis & Fix

**Status:** Identified
**Severity:** Medium - Display lag, not functional issue
**Date Identified:** November 17, 2025

---

## The Issue

**Observed:** Display temperature shows 15.1°C
**Actual:** ESPHome raw reading shows 20.1°C (5°C difference)
**Log Evidence:** `[dallas.temp.sensor:054]: 'Gazebo Temperature': Got Temperature=20.1°C`

---

## Root Cause

### Problem 1: Moving Average Filter with Large Window (PRIMARY ISSUE)

**Location:** `includes/sensors.yaml` line 17-19

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 10
      send_every: 1
```

**What it does:**
- Takes a rolling average of the last **10 readings**
- Sensor reads every 10 seconds
- So it's averaging data from the last **100 seconds (1.67 minutes)**
- This causes **massive lag** when temperature changes

**Why it's a regression:**
- This was probably added for noise reduction
- But with a window of 10, you get 5-reading startup lag
- When real temp = 20.1°C, it's still averaging old ~15°C readings

**Example:**
```
Reading sequence (every 10 seconds):
Time 0s:   15.0°C (old stable reading)
Time 10s:  15.1°C
Time 20s:  15.2°C
Time 30s:  15.5°C
Time 40s:  16.0°C
Time 50s:  17.0°C
Time 60s:  18.0°C
Time 70s:  19.0°C
Time 80s:  19.5°C
Time 90s:  20.1°C (actual current temp)
Time 100s: Average of all 10 = 16.0°C ← What you see!
```

---

## The Fix

### Option A: Reduce Window Size (RECOMMENDED)

Change window_size from 10 to 3-5:

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 3  # Changed from 10
      send_every: 1
```

**Effect:**
- Only averages last 3 readings (30 seconds)
- Much faster response time
- Still smooths noise
- Startup lag reduced to ~1 reading

**Better alternative for smooth readings:**

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 5
      send_every: 1
```

---

### Option B: Replace with Exponential Moving Average (ADVANCED)

If you want smooth filtering without lag:

```yaml
filters:
  - exponential_moving_average:
      alpha: 0.1  # Adjust 0.1 = slower, 0.5 = faster
```

**Effect:**
- Smoother curve than windowed average
- No fixed lag
- Responds faster to changes
- Better for live control

**Alpha tuning:**
- `alpha: 0.05` = Very smooth, slow response (lag ~20 seconds)
- `alpha: 0.1` = Moderate smooth, reasonable response (lag ~10 seconds)
- `alpha: 0.2` = Light smooth, fast response (lag ~5 seconds)
- `alpha: 0.5` = Barely filtered (almost raw)

---

## Other Potential Causes

### Issue: Delta Filter (Rate Limiting)

**Location:** `includes/sensors.yaml` line 28

```yaml
- delta: 1.0  # Max ±1°C change per reading
```

**What it does:**
- Limits temperature change to max 1°C per reading
- Prevents wild swings if sensor glitches
- But also limits how fast real temp can change

**Possible fix:**
Increase delta to 2.0°C:
```yaml
- delta: 2.0
```

**However:** This is probably OK - temperature usually doesn't change faster than 1°C per 10 seconds anyway.

---

## Temperature Offset Issue?

You mentioned "Temperature Offset" in the entities. Let me verify the offset is 0:

**To check in Home Assistant:**
1. Settings → Devices & Services → Entities
2. Search for "gazebo_stove_temperature_offset"
3. Check what the current value is
4. **It should be 0.0°C** (unless you intentionally set it)

**If offset is non-zero:**
The offset is added AFTER the moving average, so:
```
Display = (Average of 10 readings) + Offset
Display = 15.1°C + (-5.0°C) = 10.1°C  ← Would be even worse!
```

So if offset is negative, it would make it worse.

---

## Recommended Fix

### Step 1: Edit sensors.yaml

```bash
nano includes/sensors.yaml
```

Find line 17-18 (the moving average):

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 10  ← CHANGE THIS TO 3 or 5
      send_every: 1
```

Change to:

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 3
      send_every: 1
```

### Step 2: Save and Recompile

```bash
# In ESPHome web dashboard or command line:
esphome compile gazebo_stove.yaml
esphome upload gazebo_stove.yaml
```

Or in ESPHome web UI:
1. Click the device
2. Click "Update" or "Compile & Upload"
3. Wait for upload to complete

### Step 3: Verify Temperature

- Check device logs: should show temperature update immediately
- In Home Assistant: temperature should match actual within 1-2 readings (10-20 seconds)
- Compare with log output: should be much closer now

---

## Testing the Fix

**Before fix:**
```
Log: Got Temperature=20.1°C
HA Display: 15.1°C
Lag: ~100 seconds
Difference: 5°C
```

**After fix (window_size: 3):**
```
Log: Got Temperature=20.1°C
HA Display: Should be 19.5-20.1°C within 30 seconds
Lag: ~30 seconds
Difference: <0.5°C
```

---

## Why This Happened (Regression Analysis)

**Possible reasons window_size was set to 10:**

1. **Noise reduction** - DS18B20 can have noisy readings
2. **Display flickering** - Trying to prevent UI flicker
3. **Overshooting** - Trying to prevent relay oscillation
4. **Testing artifact** - Was left at test value and never changed

**Better approach:**
- Use **smaller window (3-5)** for responsiveness
- Use **delta filter (1.0°C)** to prevent oscillation
- Use **offset adjustment** if sensor is inaccurate

---

## Complete Sensors.yaml Fix

Here's the corrected sensors.yaml section with better filtering:

```yaml
sensor:
  # DS18B20 Temperature Sensor
  - platform: dallas_temp
    one_wire_id: ow_bus
    id: gazebo_temp
    name: "Gazebo Temperature"
    address: 0x7ce066bd0164ff28
    resolution: 12
    update_interval: 10s
    filters:
      # Light smoothing (5-reading average = 50 seconds window)
      - sliding_window_moving_average:
          window_size: 5  # FIXED: Changed from 10
          send_every: 1
      # Apply temperature offset/calibration (recalculates dynamically)
      - lambda: |-
          return x + id(temp_offset).state;
      # Clamp to valid range (-40°C to 85°C)
      - clamp:
          min_value: -40
          max_value: 85
      # Rate limiting: max ±1°C change per reading
      - delta: 1.0

    # Bad read detection and heating control
    on_value:
      then:
        - lambda: |-
            if (isnan(x)) {
              id(bad_read_count) += 1;
              ESP_LOGI("TEMP_SENSOR", "Bad temperature read #%d", id(bad_read_count));
              if (id(bad_read_count) > 10) {
                id(sensor_malfunction).publish_state(true);
                ESP_LOGI("TEMP_SENSOR", "ALARM: Sensor malfunction detected!");
              }
            } else {
              if (id(bad_read_count) > 0) {
                ESP_LOGI("TEMP_SENSOR", "Sensor recovered - resetting bad read counter");
              }
              id(bad_read_count) = 0;
              if (id(sensor_malfunction).state) {
                id(sensor_malfunction).publish_state(false);
              }
            }
        # Force climate platform to republish state with updated temperature
        - if:
            condition:
              lambda: |-
                return !isnan(x);
            then:
              - lambda: |-
                  auto* climate = id(propane_stove_thermostat);
                  if (climate != nullptr) {
                    climate->publish_state();
                    ESP_LOGD("CLIMATE_SYNC", "Climate state republished with temperature: %.1f°C", x);
                  }
              - script.execute: apply_priority_overrides

  # WiFi Signal Strength
  - platform: wifi_signal
    name: "WiFi Signal Strength"
    id: wifi_signal_strength
    update_interval: 60s
    unit_of_measurement: "dBm"
    entity_category: "diagnostic"
```

**Key change:** Line 16 - `window_size: 5` (was 10)

---

## Summary

| Item | Current | After Fix |
|------|---------|-----------|
| **Window Size** | 10 readings | 5 readings |
| **Averaging Period** | 100 seconds | 50 seconds |
| **Display Lag** | ~100 seconds | ~50 seconds |
| **Response Time** | Very slow | Acceptable |
| **Noise Smoothing** | Excellent | Good |
| **Temperature Accuracy** | ❌ Off by 5°C | ✅ Within 0.5°C |

---

## Action Required

1. **Edit** `includes/sensors.yaml` line 17
2. **Change** `window_size: 10` to `window_size: 5`
3. **Recompile** and **Upload** to device
4. **Verify** temperature reads correctly
5. **Commit** to git with explanation

Ready to proceed? Let me know if you want me to make this edit.
