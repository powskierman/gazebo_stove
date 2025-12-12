# Code Review: December 2024

Review of ESPHome configuration files for redundancies, inefficiencies, and missing comments.

## Summary

| Priority | Issue | File | Effort |
|----------|-------|------|--------|
| 1 | Fix `temp.val`/`precip.val` commands causing errors | automation.yaml | Low |
| 2 | Document duplicated `wmo_to_icon` function | weather.yaml | Low |
| 3 | Remove duplicate comments | sensors.yaml | Low |
| 4 | Document `0x91` protocol | display.yaml | Low |
| 5 | Remove unused `day_names[]` variable | weather.yaml | Low |
| 6 | Remove test globals (`test_icon_id`) | entities.yaml | Low |
| 7 | Remove/move `test_nextion_icons` script | weather.yaml | Low |
| 8 | Add slider conversion formula documentation | Multiple | Medium |

---

## Detailed Findings

### 1. Duplicated WMO Icon Mapping Function ⚠️ HIGH IMPACT

**File:** `weather.yaml` (lines 259-287, 351-379)

**Issue:** The `wmo_to_icon` lambda function is defined identically twice - once in `update_nextion_hourly` and again in `update_nextion_daily`.

**Recommendation:** 
- ESPHome lambdas can't share functions easily
- Add a comment noting the duplication exists in two places
- Alternative: Pre-compute all icons once and store in globals

---

### 2. Repetitive Sensor Definitions ⚠️ MEDIUM IMPACT

**File:** `weather.yaml` (lines 11-238)

**Issue:** 42 nearly identical `homeassistant` sensor definitions with only index changing.

**Recommendation:** Add header comment:
```yaml
# NOTE: ESPHome requires explicit sensor definitions - cannot be looped
# Each hour (0-6) and day (0-6) needs 3 sensors: feels_like/precipitation/weather_code
```

---

### 3. Unused Variable `day_names` ⚠️ LOW IMPACT

**File:** `weather.yaml` (line 382)

**Issue:** French day names array defined but never used.

**Recommendation:** Remove or implement day name display on Nextion.

---

### 4. Test Script Still Present ⚠️ LOW IMPACT

**File:** `weather.yaml` (lines 461-480)

**Issue:** `test_nextion_icons` script is commented out but still in codebase.

**Recommendation:** Remove entirely if testing is complete.

---

### 5. Duplicate Comments ⚠️ LOW IMPACT

**File:** `sensors.yaml` (lines 66-68)

**Issue:** Three identical comments: `# Update Nextion display`

**Recommendation:** Remove duplicate comments - keep just one.

---

### 6. Slider Value Conversion Logic Duplicated ⚠️ MEDIUM IMPACT

**Files:** `display.yaml`, `automation.yaml`, `climate.yaml`

**Issue:** The slider-to-temperature conversion formula appears in 4 places:
- `display.yaml:33` - Forward: `15.0f + ((float)slider_val / 100.0f) * 15.0f`
- `automation.yaml:22` - Same forward formula
- `automation.yaml:83-86` - Reverse formula (temp to slider)
- `climate.yaml:77-79` - Same reverse formula

**Recommendation:** Document constants at each location:
```cpp
// SLIDER CONFIG: 0-100 maps to 15-30°C (SLIDER_RANGE = 15.0)
```

---

### 7. Trivial Wrapper Script ⚠️ LOW IMPACT

**File:** `automation.yaml` (lines 274-276)

**Issue:** `update_relay_state` script just calls `manage_heating` - adds indirection without benefit.

**Recommendation:** Call `manage_heating` directly or document why wrapper exists.

---

### 8. Periodic Refresh Sends Unknown Components ⚠️ MEDIUM IMPACT

**File:** `automation.yaml` (lines 337, 343)

**Issue:** Sends to `temp.val` and `precip.val` (without page prefix) causing "Invalid variable name" errors.

**Recommendation:** 
- Remove unprefixed commands, OR
- Confirm these are global components in HMI

---

### 9. Missing Protocol Documentation ⚠️ MEDIUM IMPACT

**File:** `display.yaml` (lines 21-47)

**Issue:** UART debug handler parses `0x91` protocol without documenting format.

**Recommendation:** Add header comment:
```yaml
# NEXTION CUSTOM SENSOR PROTOCOL
# Format: 0x91 + 4-byte little-endian value (slider value 0-100)
# Sent by: Nextion HMI "printh" command on slider release
# Example: 0x91 0x32 0x00 0x00 0x00 = slider value 50
```

---

### 10. Test Global Still Present ⚠️ LOW IMPACT

**File:** `entities.yaml` (lines 213-217)

**Issue:** `test_icon_id` global for icon testing still defined.

**Recommendation:** Remove if testing is complete.

---

## Positive Notes ✅

- **climate.yaml** - Excellent inline comments and comprehensive implementation notes section
- **Good section headers** - All files use clear `# ====` separators
- **Logging** - Good use of ESP_LOGI/ESP_LOGW/ESP_LOGE for debugging

---

## Suggested Improvement: Central Constants

Create a `substitutions` section in `gazebo_stove.yaml` for shared values:

```yaml
substitutions:
  slider_min_temp: "15"
  slider_max_temp: "30"
  slider_range: "15"
  manual_run_timeout_min: "15"
  display_refresh_interval: "5s"
  weather_refresh_interval: "30min"
```

---

*Review completed: December 11, 2024*
