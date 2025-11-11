# Nextion Display Test - Quick Start Guide

**Purpose**: Verify UART communication and basic display functionality during Phase 1
**Test**: Cycles temperature value on Page 0 from 0→50°C→0 at 1-second intervals
**Duration**: Runs indefinitely (each full cycle ~100 seconds)
**Status**: ✅ Ready to use

---

## Quick Start

### 1. Prepare Hardware

- [ ] Connect Nextion display to ESP32:
  - TX (GPIO17) → Nextion RX
  - RX (GPIO16) → Nextion TX
  - GND → GND
  - 5V → Power
- [ ] Connect ESP32 to USB for flashing
- [ ] Power on Nextion display
- [ ] Verify Page 0 shows (should display initial values)

### 2. Flash Test Configuration

```bash
# Compile
esphome compile NEXTION_TEST.yaml

# Flash to device (USB)
esphome upload NEXTION_TEST.yaml

# Monitor output
esphome logs NEXTION_TEST.yaml
```

### 3. Expected Output

**Logs** (first 10 lines):
```
[00:00:00][I][NEXTION_TEST]: GazeboWx Nextion Test Starting
[00:00:02][I][NEXTION_TEST]: UART Connected to Nextion
[00:00:02][I][NEXTION_TEST]: Starting temperature cycle script
[00:00:03][I][NEXTION_TEST]: Cycle iteration: 0, Value: 0
[00:00:03][I][NEXTION_TEST]: Updating temp to: 0°C
[00:00:03][I][NEXTION_TEST]: UART Command Sent: temp.val=0
[00:00:04][I][NEXTION_TEST]: Cycle iteration: 1, Value: 1
[00:00:04][I][NEXTION_TEST]: Updating temp to: 1°C
[00:00:04][I][NEXTION_TEST]: UART Command Sent: temp.val=1
...
```

**Display** (Nextion Page 0):
- Temperature value should count: 0 → 1 → 2 → ... → 50
- Then count down: 49 → 48 → ... → 1
- Then repeat indefinitely
- Each step = exactly 1 second

### 4. Success Verification

✅ **Test PASSED if**:
- [ ] Logs show "UART Connected to Nextion"
- [ ] Temperature counts correctly (0→50, then 49→1)
- [ ] Display updates every 1 second
- [ ] No UART errors in logs
- [ ] No display corruption
- [ ] Cycle repeats smoothly

❌ **Test FAILED if**:
- [ ] "UART Connection Failed" error
- [ ] Display shows "888" instead of counting
- [ ] Values skip or don't match logs
- [ ] UART timeout errors
- [ ] Display freezes or garbles

---

## Troubleshooting

### "UART Connection Failed"

**Check**:
1. GPIO pin connections (RX/TX swapped?)
2. Baud rate: Must be 9600
3. Power to Nextion: Is it on?
4. USB cable: Is device uploading correctly?

**Solution**:
```yaml
# Verify UART pins in NEXTION_TEST.yaml:
uart:
  - id: uart_nextion
    tx_pin: GPIO17   # ESP32 TX
    rx_pin: GPIO16   # ESP32 RX
    baud_rate: 9600
```

### Display Shows "888" or No Update

**Check**:
1. UART termination bytes correct? (0xFF 0xFF 0xFF)
2. Nextion baud rate set to 9600?
3. Page 0 loaded on Nextion?

**Solution**:
```bash
# Check Nextion settings:
# Menu → Settings → Serial → Check baud = 9600
# Then restart Nextion and device
```

### Values Don't Match Logs

**Check**:
1. UART cable quality (loose connection?)
2. Interference (WiFi antenna nearby?)
3. CPU overload (other tasks running?)

**Solution**:
```bash
# Monitor UART directly (if available):
esphome logs NEXTION_TEST.yaml --verbose

# Check for UART errors in output
```

### Test Runs but Display Frozen

**Check**:
1. UART buffer overflow
2. Nextion firmware crash

**Solution**:
1. Power cycle Nextion
2. Reduce update frequency (change 1s to 2s delay)
3. Flash Nextion firmware

---

## What This Test Does

### Temperature Cycling Sequence

```
Time (sec) | Temperature Value | Direction
-----------|------------------|----------
0          | 0°C              | ↑ Ascending
1          | 1°C              | ↑
2          | 2°C              | ↑
...        | ...              | ↑
50         | 50°C             | ↓ Starts descending
51         | 49°C             | ↓
52         | 48°C             | ↓
...        | ...              | ↓
99         | 1°C              | ↓
100        | 0°C              | ↑ Restart ascending
101        | 1°C              | ↑
```

### UART Commands Sent

Each update sends this to Nextion:
```
temp.val=XX[0xFF][0xFF][0xFF]
```

Where:
- `temp` = Page 0 component name (temperature field)
- `.val` = Set numeric value
- `XX` = Temperature value (0-50, cycling)
- `[0xFF][0xFF][0xFF]` = Termination bytes (required by Nextion)

### Expected Cycle Duration

- Ascending (0→50): 51 steps × 1 second = 51 seconds
- Descending (49→1): 49 steps × 1 second = 49 seconds
- Total cycle: ~100 seconds
- Repeats indefinitely

---

## Advanced Options

### Change Update Frequency

Edit `NEXTION_TEST.yaml`:
```yaml
sensor:
  - platform: template
    id: test_temperature
    update_interval: 1s  # Change from 1s to 2s, 500ms, etc.
```

### Change Temperature Range

Edit `NEXTION_TEST.yaml`:
```yaml
script:
  - id: cycle_temperature
    then:
      - repeat:
          count: 31  # Change from 51 to 31 (0→30 instead of 0→50)
          then:
            # ... rest of script
```

### Monitor UART Bytes Directly

Add logging:
```yaml
logger:
  level: DEBUG
  logs:
    uart.debug: DEBUG  # Shows UART data
```

### Add LED Indicator

Optional status LED that blinks on each update:
- Uses GPIO2 by default
- Blink = successful UART command sent
- Adjust GPIO pin if needed

---

## When Test is Complete

Once this test passes successfully:

1. **Add Real Sensor**
   - Replace test loop with DS18B20 sensor readings
   - See: `NEXTION_DISPLAY.md` Section "Sensor Integration"

2. **Add More Components**
   - Add `precip` (humidity/precipitation)
   - Add weather icon updates
   - Add `b1` status button

3. **Implement Page 1**
   - Add Slider0 temperature control
   - Add `n0` (current) and `n1` (desired) temperature

4. **Continue to Phase 6**
   - Implement Pages 2 & 3 (forecasts)
   - Integrate OpenMeteo API
   - Full display implementation

---

## Files Reference

| File | Purpose |
|------|---------|
| `NEXTION_TEST.yaml` | This test configuration |
| `NEXTION_DISPLAY.md` | Complete display integration guide |
| `PLAN.md` | Phase 1 implementation details (see Week 11-12) |
| `SPECIFICATION.md` | Display requirements (Section: Display Components) |

---

## Safety & Limitations

✅ **Safe to run**:
- No heating/relay activated (display test only)
- No network/API calls
- Safe to run 24/7
- No persistent data changes

⚠️ **Limitations**:
- Only tests `temp` component on Page 0
- Doesn't test other components (precip, icon, etc.)
- Doesn't test Page 1-3
- Doesn't test weather icons or touch input
- Simulated temperature (not real sensor data)

---

## Test Report Template

Use this to document your test results:

```markdown
## Nextion Display Test Report

**Date**: [Date]
**Device**: [Device ID]
**Firmware**: [ESPHome Version]
**Duration**: [Test Duration - e.g., 15 minutes]

### Results
- [ ] UART connected successfully
- [ ] Temperature counts 0→50 correctly
- [ ] Temperature counts down 49→1 correctly
- [ ] Updates occur every 1 second
- [ ] No UART errors
- [ ] No display corruption
- [ ] Cycle repeats smoothly

### Issues Found
[List any issues encountered]

### Performance Metrics
- Latency per update: _____ ms
- UART errors: _____
- Display glitches: _____

### Conclusion
[ ] PASSED - Ready for Phase 1 hardware verification
[ ] FAILED - See issues above
```

---

## Summary

This test verifies:
1. ✅ UART communication between ESP32 and Nextion
2. ✅ Proper baud rate (9600)
3. ✅ Nextion receives and displays commands
4. ✅ Update timing (1 second intervals)
5. ✅ No data corruption

**Goal**: Confirm display hardware is working before proceeding with full Phase 1 implementation.

---

**Document**: NEXTION_TEST_README.md
**Version**: 1.0.0
**Date**: 2025-11-10
**Status**: Ready to use

**See also**: `NEXTION_TEST.yaml` for the full test configuration
