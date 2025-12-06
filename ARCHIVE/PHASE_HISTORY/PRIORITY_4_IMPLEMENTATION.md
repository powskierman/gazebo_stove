# Priority 4: Thermostat Algorithm - Implementation Summary

**Status**: ✅ Complete  
**Date**: 2025-01-27  
**Priority Level**: 4 (Lowest - Normal Operation)

---

## Overview

Priority 4 implements a custom thermostat control algorithm with **asymmetric hysteresis** logic. This replaces the ESPHome climate platform which uses symmetric deadband, allowing full control over the hysteresis calculation to match user requirements.

---

## Implementation Details

### Asymmetric Hysteresis Logic

**Formula**:
```
Turn ON when:  T_actual < T_desired - H
Turn OFF when: T_actual > T_desired + H
```

**Example** (T_desired=22°C, H=2°C):
- **Turn ON**: T < 20°C (desired - hysteresis)
- **Turn OFF**: T > 24°C (desired + hysteresis)
- **Deadband**: 20°C to 24°C (4°C total, asymmetric)

**Comparison**:
- **Old (ESPHome platform)**: Symmetric ±H/2 → ON: <21°C, OFF: >23°C (2°C deadband)
- **New (Custom script)**: Asymmetric ±H → ON: <20°C, OFF: >24°C (4°C deadband)

### Benefits

✅ **Wider deadband** reduces heating cycles (better for propane stove longevity)  
✅ **User-configurable hysteresis** via `number.heating_hysteresis` entity  
✅ **Immediate response** to temperature, setpoint, and hysteresis changes  
✅ **Respects priority hierarchy** (only runs when no higher priority is active)

---

## Files Modified

### ESPHome Configuration

1. **`includes/entities.yaml`**
   - Removed ESPHome `climate` platform
   - Updated `desired_temp` to trigger thermostat control on setpoint change
   - Updated `heating_hysteresis` to trigger thermostat control on hysteresis change

2. **`includes/automation.yaml`**
   - Added `thermostat_control` script with asymmetric hysteresis logic
   - Added interval automation to run thermostat control every 10 seconds
   - Updated `manage_heating` script comments

3. **`includes/sensors.yaml`**
   - Updated temperature sensor to trigger thermostat control on valid readings

### Home Assistant Configuration

4. **`homeassistant/configuration.yaml`**
   - Added template climate entity for thermostat card UI
   - Maintains thermostat card functionality in Home Assistant
   - Displays current temperature, setpoint, and heating status

---

## How It Works

### Control Flow

1. **Temperature Reading** (every 10 seconds)
   - Sensor reads temperature
   - Triggers `thermostat_control` script

2. **Priority Check**
   - Script checks if higher priorities are active:
     - Priority 1: Force Off (Manual Stop)
     - Priority 2: Emergency Heat (Manual Run)
     - Priority 3: Schedule (Away Mode)
   - If any higher priority is active, script exits

3. **Hysteresis Calculation**
   - Gets current temperature, desired setpoint, and hysteresis value
   - Calculates thresholds:
     - Lower = Desired - Hysteresis
     - Upper = Desired + Hysteresis

4. **Relay Control**
   - If T < Lower Threshold → Turn relay ON
   - If T > Upper Threshold → Turn relay OFF
   - If within deadband → Maintain current state

### Immediate Response Triggers

Thermostat control runs immediately when:
- ✅ Temperature sensor reading changes (valid readings only)
- ✅ Desired temperature setpoint changes
- ✅ Heating hysteresis value changes
- ✅ Every 10 seconds (interval automation)

---

## User Interface

### Home Assistant Thermostat Card

The template climate entity provides:
- **Current Temperature**: Real-time sensor reading
- **Target Temperature**: Setpoint slider (5-35°C, 0.5°C steps)
- **HVAC Action**: Shows "heating", "idle", or "off"
- **Preset Mode**: Shows "home" or "away" based on Presence Mode

### Controls

- **Setpoint Adjustment**: Change target temperature via slider
- **Preset Mode**: Switch between "home" and "away" modes
- **Status Display**: See current heating state at a glance

---

## Testing

### Test Case 1: Asymmetric Hysteresis

**Setup**:
- Desired Temperature: 22°C
- Hysteresis: 2°C
- Expected Thresholds: Lower=20°C, Upper=24°C

**Test**:
1. Heat room to 25°C
2. Let temperature drop slowly
3. **Expected**: Relay turns ON at ~20°C (not 21°C)
4. Let heating run until relay turns OFF
5. **Expected**: Relay turns OFF at ~24°C (not 23°C)

### Test Case 2: Priority Hierarchy

**Test**:
1. Set desired temp to 30°C (force heating demand)
2. Enable Force Off → Relay should remain OFF
3. Disable Force Off → Relay should turn ON if temp < threshold
4. Enable Away Mode → Relay should turn OFF
5. Disable Away Mode → Relay should turn ON if temp < threshold

### Test Case 3: Immediate Response

**Test**:
1. Change desired temperature → Thermostat should respond immediately
2. Change hysteresis value → Thermostat should recalculate thresholds immediately
3. Temperature reading changes → Thermostat should evaluate immediately

---

## Configuration

### Hysteresis Range

Current configuration:
- **Minimum**: 1°C
- **Maximum**: 5°C
- **Step**: 0.5°C
- **Default**: 2°C

**Recommendation**: With asymmetric hysteresis (±H), consider reducing max to 3°C:
- H=5°C means ±5°C (10°C total deadband) - may be too wide
- Suggested range: 0.5°C to 3°C

### Update Interval

- **Interval Automation**: Every 10 seconds
- **Sensor Trigger**: On every valid temperature reading (~10 seconds)
- **Setpoint/Hysteresis Change**: Immediate

---

## Troubleshooting

### Thermostat Not Responding

**Check**:
1. Verify no higher priority is active (Force Off, Emergency Heat, Away Mode)
2. Check ESPHome logs for thermostat decisions
3. Verify `heating_hysteresis` entity value is set
4. Verify `desired_temp` entity value is set
5. Check temperature sensor is reading correctly

### Heating Cycles Too Frequent

**Solution**:
- Increase hysteresis value (e.g., from 2°C to 3°C)
- This widens the deadband and reduces cycling

### Heating Cycles Too Infrequent

**Solution**:
- Decrease hysteresis value (e.g., from 2°C to 1°C)
- This narrows the deadband and increases responsiveness

### Thermostat Card Not Showing

**Check**:
1. Verify template climate entity is added to `configuration.yaml`
2. Update entity IDs to match your ESPHome device names
3. Restart Home Assistant
4. Check Home Assistant logs for template errors

---

## Comparison: Old vs New

| Aspect | Old (ESPHome Platform) | New (Custom Script) |
|--------|------------------------|---------------------|
| Hysteresis | Symmetric ±H/2 | Asymmetric ±H |
| Deadband Width | H (e.g., 2°C) | 2×H (e.g., 4°C) |
| User Control | Limited (platform internal) | Full (uses `heating_hysteresis` entity) |
| Customization | No | Yes |
| Heating Cycles | More frequent | Less frequent |
| User Requirements | ❌ Doesn't match | ✅ Matches exactly |

---

## Status

✅ **Complete**: All Priority 4 requirements implemented

**Features**:
- ✅ Custom thermostat script with asymmetric hysteresis
- ✅ Uses `heating_hysteresis` entity for configuration
- ✅ Respects priority hierarchy (only runs when Priority 1-3 are inactive)
- ✅ Immediate response to changes
- ✅ Home Assistant thermostat card maintained
- ✅ Comprehensive logging for debugging

**Next Steps**:
1. Deploy ESPHome configuration
2. Add Home Assistant template climate entity
3. Test asymmetric hysteresis behavior
4. Monitor heating cycles for 48 hours
5. Adjust hysteresis value if needed

---

**Implementation Date**: 2025-01-27  
**Priority Level**: 4 (Lowest - Normal Operation)  
**Status**: ✅ Ready for Testing

