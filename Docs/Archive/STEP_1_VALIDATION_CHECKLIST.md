# Step 1 Validation Checklist
## Priority Hierarchy Correction - Complete Testing Guide

**Date**: 2025-11-14
**Status**: Ready for Device Testing
**Compilation**: ✅ SUCCESS (15.04 seconds)
**Flash Size**: 52.0% (953,790 bytes / 1,835,008 bytes available)
**RAM Usage**: 12.1% (39,524 bytes / 327,680 bytes available)

---

## Quick Start: Device Upload & Testing

### Phase 1: Device Upload (5-10 minutes)

#### Option A: Network Upload (Preferred if device is online)
```bash
# From project directory
cd /Volumes/Users/Michel/Documents/Electronics/Projects/GazeboStove

# Upload to device
esphome upload gazebo_stove.yaml

# Expected output:
# INFO Uploading /path/to/.esphome/build/gazebo-stove/.pioenvs/gazebo-stove/firmware.ota.bin (953790 bytes) to gazebo-stove.local
# INFO Uploaded successfully
```

**If upload fails**: Device is not on network. Use Option B (USB upload).

#### Option B: USB Upload (If device is disconnected)
1. Connect ESP32 to USB cable
2. Wait 2 seconds for device detection
3. Run:
```bash
esphome upload gazebo_stove.yaml --device /dev/cu.usbserial-[PORT_ID]
```

Replace `[PORT_ID]` with output from:
```bash
ls /dev/cu.usbserial-*
```

---

## Phase 2: Home Assistant Entity Verification (2 minutes)

### ✅ Checklist: Entities Appear

After device boots (wait 30 seconds):

**1. Check Desired Temperature Number Entity**
- [ ] Navigate to Home Assistant → Settings → Devices & Services → Entities
- [ ] Search for: `number.gazebo_stove_desired_temp`
- [ ] Expected: Shows slider with current value (default: 22°C)
- [ ] Range: 5°C to 35°C with 0.5° steps
- [ ] Action: Try adjusting slider → value should update in real-time

**2. Check Current Temperature Sensor**
- [ ] Search for: `sensor.gazebo_temperature`
- [ ] Expected: Shows current room temperature (e.g., 19.5°C)
- [ ] Update Rate: Should update every 10 seconds (sensor throttling)
- [ ] Action: Verify it's changing (wait 10 seconds, should see update)

**3. Check Force Off Switch**
- [ ] Search for: `switch.gazebo_stove_manual_stop`
- [ ] Expected: Toggle switch, initially OFF
- [ ] Action: Toggle ON → Relay should turn OFF (safety feature)

**4. Check Emergency Heat Switch**
- [ ] Search for: `switch.gazebo_stove_manual_run`
- [ ] Expected: Toggle switch with 15-minute timer
- [ ] Initial: OFF
- [ ] Action: Toggle ON → Relay should turn ON

**5. Check Presence Mode Selector**
- [ ] Search for: `select.gazebo_stove_presence_mode`
- [ ] Expected: Shows "home" or "away"
- [ ] Options: "home" and "away"
- [ ] Action: Try switching between modes

**6. Check Sensor Malfunction Alarm**
- [ ] Search for: `binary_sensor.gazebo_stove_sensor_malfunction_alarm`
- [ ] Expected: Should show OFF (no sensor issues)
- [ ] Purpose: Triggers if >10 consecutive bad temperature readings

**Summary for Phase 2:**
```
✅ All entities appear → Continue to Phase 3
❌ Entities missing → Check device logs with: esphome logs gazebo_stove.yaml
```

---

## Phase 3: Relay Control Verification (3 minutes)

### Test 1: Manual Stop (Force Off) - PRIORITY 1
**Purpose**: Verify highest priority override works

1. **Setup**:
   - Ensure desired_temp is set to 25°C (above room temp)
   - Verify presence_mode is "home"
   - Verify Emergency Heat is OFF

2. **Test Steps**:
   - [ ] Toggle `switch.gazebo_stove_manual_stop` to ON
   - [ ] Observe: Relay should immediately turn OFF
   - [ ] Expected Log: "Force Off: Relay OFF (Priority 1 - HIGHEST)"
   - [ ] Verify: Can't turn relay ON even though temp is below setpoint
   - [ ] Toggle `switch.gazebo_stove_manual_stop` back to OFF

3. **Success Criteria**: ✅ Relay stays OFF while Force Off is active

---

### Test 2: Emergency Heat (Manual Run) - PRIORITY 2
**Purpose**: Verify 15-minute override and Priority 2 functionality

1. **Setup**:
   - Force Off is OFF
   - Presence Mode is "away" (standby)
   - Desired Temp: 25°C

2. **Test Steps**:
   - [ ] Toggle `switch.gazebo_stove_manual_run` to ON
   - [ ] Observe: Relay should turn ON immediately
   - [ ] Expected Log: "Emergency Heat: Relay ON (Priority 2, 900000 ms remaining)"
   - [ ] Wait: Let timer count down (check logs every 30 seconds)
   - [ ] Verify: After 15 minutes, relay automatically turns OFF
   - [ ] Expected Log: "15-minute timeout reached - auto-disabling"

3. **Success Criteria**: ✅ Relay stays ON for ~15 minutes then auto-disables

**Note**: Full 15-minute test is optional. Can verify timeout logic with logs.

---

### Test 3: Schedule (Presence Mode) - PRIORITY 3
**Purpose**: Verify away/home mode controls heating

1. **Setup**:
   - Force Off is OFF
   - Emergency Heat is OFF
   - Desired Temp: 30°C (above room temp to trigger heating)

2. **Test Steps - Home Mode**:
   - [ ] Set `select.gazebo_stove_presence_mode` to "home"
   - [ ] Verify current temp is below 30°C
   - [ ] Observe: Relay should turn ON (thermostat active)
   - [ ] Expected Log: "Start heating: X.X°C < 29.5°C (Priority 4)"

3. **Test Steps - Away Mode**:
   - [ ] Set `select.gazebo_stove_presence_mode` to "away"
   - [ ] Observe: Relay should turn OFF immediately (standby)
   - [ ] Expected Log: "Schedule: Standby mode (Priority 3) - Relay OFF"

4. **Success Criteria**: ✅ Relay responds to home/away changes

---

### Test 4: Thermostat Algorithm - PRIORITY 4
**Purpose**: Verify normal temperature control with hysteresis

1. **Setup**:
   - Force Off is OFF
   - Emergency Heat is OFF
   - Presence Mode is "home"
   - Current temperature: ~20°C (measure with sensor)
   - Hysteresis: 1.0°C (default)

2. **Test Steps**:
   - [ ] Set `number.gazebo_stove_desired_temp` to 22°C
   - [ ] Lower threshold = 22 - 0.5 = 21.5°C
   - [ ] Upper threshold = 22 + 0.5 = 22.5°C
   - [ ] Current temp (20°C) < Lower threshold (21.5°C)
   - [ ] Observe: Relay should turn ON
   - [ ] Expected Log: "Start heating: 20.0°C < 21.5°C (Priority 4)"

3. **Verify Hysteresis Deadband**:
   - [ ] Watch temperature rise: 20.0 → 21.0 → 21.5 → 22.0 → 22.5 → 23.0
   - [ ] At 22.5°C (upper threshold): Relay should turn OFF
   - [ ] Expected Log: "Stop heating: 23.0°C > 22.5°C (Priority 4)"
   - [ ] As temp falls: 23.0 → 22.5 → 22.0 → 21.5 → 21.0 → 20.5
   - [ ] At 21.5°C (lower threshold): Relay should turn ON
   - [ ] Expected Log: "Start heating: 20.5°C < 21.5°C (Priority 4)"

4. **Success Criteria**: ✅ Relay maintains deadband (±0.5°C around setpoint)

---

## Phase 4: Priority Hierarchy Stress Testing (Optional)

### Advanced Test: All Priorities Together
**Purpose**: Verify priority ordering under complex conditions

**Scenario A: Force Off Overrides Emergency Heat**
```
1. Set Emergency Heat ON (Priority 2)
2. Relay is ON ✓
3. Toggle Force Off ON (Priority 1)
4. Relay should turn OFF immediately (Priority 1 > 2) ✓
5. Expected: Force Off wins
```

**Scenario B: Emergency Heat Overrides Schedule**
```
1. Set Presence Mode to "away" (Priority 3)
2. Relay is OFF (standby) ✓
3. Toggle Emergency Heat ON (Priority 2)
4. Relay should turn ON (Priority 2 > 3) ✓
5. Expected: Emergency Heat wins
```

**Scenario C: Schedule Overrides Thermostat**
```
1. Set desired_temp to 30°C (above room)
2. With Presence Mode "home", relay turns ON ✓
3. Switch Presence Mode to "away" (Priority 3)
4. Relay should turn OFF (Priority 3 > 4) ✓
5. Expected: Schedule wins
```

**Scenario D: Complete Priority Chain**
```
1. Start: All off, temp 20°C, desired 25°C, home mode
2. Expected: Relay ON (Priority 4 wants heating)
3. Enable Emergency Heat
4. Expected: Relay stays ON (Priority 2)
5. Set away mode
6. Expected: Relay stays ON (Priority 2 > 3)
7. Enable Force Off
8. Expected: Relay OFF (Priority 1 > all)
9. Disable Force Off
10. Expected: Relay ON again (Emergency Heat still active)
11. Disable Emergency Heat
12. Expected: Relay OFF (Away mode, Priority 3)
13. Set home mode
14. Expected: Relay ON (Priority 4 resumes)
```

---

## Phase 5: Log Verification (5 minutes)

### View Device Logs
```bash
esphome logs gazebo_stove.yaml --level DEBUG
```

**Expected Log Messages During Testing**:

#### Priority 1 (Force Off) Message
```
[PRIORITY] Force Off: Relay OFF (Priority 1 - HIGHEST)
```

#### Priority 2 (Emergency Heat) Message
```
[PRIORITY] Emergency Heat: Relay ON (Priority 2, 900000 ms remaining)
```

#### Priority 3 (Schedule) Messages
```
[PRIORITY] Schedule: Standby mode (Priority 3) - Relay OFF
```

#### Priority 4 (Thermostat) Messages
```
[HEATING] Start heating: 20.0°C < 21.5°C (Priority 4)
[HEATING] Stop heating: 23.0°C > 22.5°C (Priority 4)
[HEATING] In deadband: 21.5-22.5, Relay=ON (no change)
```

#### Temperature Readings
```
[HEATING] Current: 20.5°C, Desired: 22.0°C, Hysteresis: 1.0°C
[HEATING] Thresholds: Lower=21.5, Upper=22.5, Relay=ON
```

---

## Validation Summary Table

| Test | Pass Criteria | Status |
|------|---------------|--------|
| **P1: Force Off** | Overrides all others | [ ] |
| **P2: Emergency Heat** | 15-min timer works | [ ] |
| **P3: Schedule Away** | Blocks heating | [ ] |
| **P3: Schedule Home** | Allows thermostat | [ ] |
| **P4: Heating ON** | Turns relay ON below lower threshold | [ ] |
| **P4: Heating OFF** | Turns relay OFF above upper threshold | [ ] |
| **P4: Deadband** | Maintains state in band | [ ] |
| **All Entities** | Present in Home Assistant | [ ] |
| **Temperature Sensor** | Updates every 10 seconds | [ ] |
| **Compilation** | ✅ SUCCESS | ✅ |

---

## Troubleshooting

### Issue: Device won't upload
**Solution**: Check WiFi connection or use USB cable with Option B upload method

### Issue: Entities don't appear in Home Assistant
**Solution**:
1. Restart Home Assistant
2. Check device logs: `esphome logs gazebo_stove.yaml`
3. Look for errors during initialization

### Issue: Relay doesn't respond
**Solution**:
1. Check GPIO12 is not shorted
2. Verify relay configuration in `includes/hardware.yaml`
3. Check device logs for relay control errors

### Issue: Temperature sensor shows wrong value
**Solution**:
1. Check DS18B20 wiring on GPIO4
2. Adjust `temp_offset` in `number.gazebo_stove_temp_offset` entity
3. Check OneWire protocol in logs

### Issue: Emergency Heat doesn't timeout
**Solution**:
1. Check device system time (NTP synced?)
2. View logs to see timeout countdown
3. Manually toggle switch if timeout fails

---

## Next Steps After Validation

If all tests PASS:
1. ✅ Document test results
2. ✅ Commit validation findings
3. ✅ Proceed to **Step 2: Emergency Heat Force Off Check**
   - Ensure Emergency Heat cannot be active with Force Off simultaneously
   - Implement mutual exclusivity lock
   - Time estimate: 20 minutes

If tests FAIL:
1. ❌ Document which tests failed
2. ❌ Review device logs for errors
3. ❌ Check corresponding hardware connections
4. ❌ Create issue document with failure details

---

## Key Concepts Validated

### Priority Hierarchy (Step 1 Implementation)
```
Force Off (P1) ↓ (highest)
Emergency Heat (P2) ↓
Schedule/Presence (P3) ↓
Thermostat Algorithm (P4) (lowest)
```

### Thermostat Algorithm (Priority 4)
```
IF current < (desired - hysteresis/2) → Relay ON
IF current > (desired + hysteresis/2) → Relay OFF
IF between thresholds → Maintain current state
```

### 15-Minute Emergency Heat Timer
```
SET: Emergency Heat toggle ON
START: millisecond timer
RUN: Relay stays ON
CHECK: Every 30 seconds if elapsed >= 15 minutes
AUTO-OFF: Disable after 15 minutes, return to Priority 3
```

---

## Hardware Reference

| Component | Pin | Function |
|-----------|-----|----------|
| Temperature Sensor | GPIO4 | DS18B20 OneWire |
| Heating Relay | GPIO12 | Main heat control (STRAPPING PIN) |
| Status LED | GPIO2 | Visual indicator (STRAPPING PIN) |
| Nextion Display | UART1 | Visual display |

⚠️ **Note**: GPIO2 and GPIO12 are strapping pins. May need external pull-up/down resistors.

---

## Files Involved in Testing

- **[includes/automation.yaml](../../includes/automation.yaml)**: manage_heating script with priority logic
- **[includes/hardware.yaml](../../includes/hardware.yaml)**: Relay configuration
- **[includes/sensors.yaml](../../includes/sensors.yaml)**: Temperature sensor
- **[includes/entities.yaml](../../includes/entities.yaml)**: UI controls (number, select, switch entities)
- **[gazebo_stove.yaml](../../gazebo_stove.yaml)**: Main configuration

---

## Compilation Details

**Last Successful Build**: 2025-11-14 15:04
**Build Time**: 15.04 seconds
**Binary Size**: 953,790 bytes (52.0% of 1,835,008 available)
**RAM**: 39,524 bytes (12.1% of 327,680 available)
**Status**: ✅ READY FOR DEPLOYMENT

---

**Document Version**: 1.0
**Created**: 2025-11-14
**Author**: Claude Code
**Status**: READY FOR TESTING
