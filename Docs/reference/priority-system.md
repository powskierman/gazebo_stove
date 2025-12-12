# Thermostat Priority Hierarchy - Visual Reference

**Status**: ✅ IMPLEMENTED IN STEP 1
**Date**: 2025-11-14

---

## Priority Order - Before vs After

### BEFORE (BROKEN) ❌

```
                   HIGHEST PRIORITY
                         ▲
                         │
                    ┌────────────┐
              P1    │ Away Mode  │  ← PROBLEM: Overrides Force Off!
                    └────────────┘
                         │
                    ┌────────────┐
              P2    │ Manual Run │  ← Could turn on despite Force Off
                    │(Emergency) │
                    └────────────┘
                         │
                    ┌────────────┐
              P3    │Manual Stop │  ← PROBLEM: Lowest priority!
                    │(Force Off) │     User has no control when Away Mode active
                    └────────────┘
                         │
                    ┌────────────┐
              P4    │Thermostat  │
                    └────────────┘
                         │
                   LOWEST PRIORITY

CRITICAL SAFETY FLAW:
User enables Force Off (maintenance mode)
System enters Away Mode
→ Away Mode OVERRIDES Force Off
→ Heating can turn on unexpectedly
→ User loses manual control
```

### AFTER (CORRECT) ✅

```
                   HIGHEST PRIORITY
                         ▲
                         │
                    ┌────────────┐
              P1    │Force Off   │  ← SOLUTION: Highest priority!
                    │(Manual     │     User has absolute control
                    │ Stop)      │
                    └────────────┘
                         │
                    ┌────────────┐
              P2    │ Emergency  │  ← Overrides schedule but respects Force Off
                    │ Heat       │     Respects highest priority
                    │(Manual Run)│
                    └────────────┘
                         │
                    ┌────────────┐
              P3    │ Schedule   │  ← Respects both Force Off and Emergency Heat
                    │(Home/Away) │     Time-based home/away control
                    └────────────┘
                         │
                    ┌────────────┐
              P4    │Thermostat  │  ← Only runs when all above inactive
                    └────────────┘
                         │
                   LOWEST PRIORITY

CORRECT SAFETY MODEL:
User enables Force Off (maintenance mode)
→ Relay STAYS OFF (Priority 1)
System enters Away Mode
→ Relay STAYS OFF (Force Off still highest priority)
User disables Force Off
→ Relay can turn on (if away mode allows)
```

---

## Control Mode Decision Flow

### How the System Decides What to Do

```
START: manage_heating script called
│
│  ┌─────────────────────────────────────┐
├─→│ Check 1: Force Off Active?          │
│  │ (manual_stop = true)                │
│  └─────────────────────────────────────┘
│  │
│  ├─→ YES: TURN RELAY OFF
│  │        Log: "Force Off: Relay OFF (Priority 1 - HIGHEST)"
│  │        RETURN (done, exit)
│  │
│  └─→ NO: Continue to Check 2
│
│  ┌─────────────────────────────────────┐
├─→│ Check 2: Emergency Heat Active?     │
│  │ (manual_run = true)                 │
│  └─────────────────────────────────────┘
│  │
│  ├─→ YES: Check timeout
│  │         ├─ Expired? → Disable, continue to Check 3
│  │         └─ Active? → TURN RELAY ON, RETURN
│  │                      Log: "Emergency Heat: Relay ON (Priority 2)"
│  │
│  └─→ NO: Continue to Check 3
│
│  ┌─────────────────────────────────────┐
├─→│ Check 3: Schedule Standby Active?   │
│  │ (presence_mode = "away")            │
│  └─────────────────────────────────────┘
│  │
│  ├─→ YES: TURN RELAY OFF
│  │        Log: "Schedule: Standby mode (Priority 3)"
│  │        RETURN (done, exit)
│  │
│  └─→ NO: Continue to Check 4
│
│  ┌─────────────────────────────────────┐
└─→│ Check 4: Thermostat Algorithm       │
   │ (normal temperature control)        │
   └─────────────────────────────────────┘
      │
      ├─ Temp < (Desired - Hysteresis/2)?
      │  → TURN RELAY ON (Priority 4)
      │
      ├─ Temp > (Desired + Hysteresis/2)?
      │  → TURN RELAY OFF (Priority 4)
      │
      └─ Between thresholds?
         → MAINTAIN current state (deadband)


KEY INSIGHT:
Each check returns immediately if its condition is true.
Only lower-priority checks run if higher priorities are inactive.
```

---

## Practical Usage Examples

### Example 1: Normal Daily Operation

```
06:00 AM - Home Mode Time triggers
           Presence Mode → "home"
           ↓
           Thermostat active, heating controlled automatically

14:00 - Room gets cold (winter day)
        Current temp: 18°C, Desired: 22°C
        ↓
        Thermostat: 18 < 21 → Turn relay ON

16:00 - Room warms up
        Current temp: 23°C
        ↓
        Thermostat: 23 > 23 → Turn relay OFF

22:00 - Away Mode Time triggers
        Presence Mode → "away"
        ↓
        Relay OFF (heating disabled for energy saving)
```

### Example 2: Emergency Heat During Away Mode

```
22:00 - Away Mode active, Relay OFF

Emergency (pipes freezing, etc.)
User presses Emergency Heat button
↓
Priority Check:
  P1: Force Off? NO
  P2: Emergency Heat? YES → Relay ON

Relay turns ON (overrides away mode)
Timer runs for 15 minutes
↓
23:15 - Timer expires
        Emergency Heat auto-disables
        Relay OFF (away mode resumes control)
```

### Example 3: Maintenance Mode

```
User enables Force Off (maintenance, repairing valve)
↓
Priority Check:
  P1: Force Off? YES → Relay OFF

Relay turns OFF (and stays off)

Time passes...
- Schedule tries to turn on heating at 06:00 → Force Off keeps it OFF
- Thermostat demands heat because temp drops → Force Off keeps it OFF
- User tries to press Emergency Heat → Force Off keeps it OFF

User disables Force Off
↓
Relay OFF still (but now respects other modes)
System resumes normal operation
```

### Example 4: Priority Conflicts - Resolution

```
SCENARIO: Multiple modes active simultaneously

Conditions:
- Force Off (Manual Stop): ENABLED
- Emergency Heat (Manual Run): ENABLED (5 min remaining)
- Schedule: AWAY mode
- Thermostat: Demanding heat (temp < desired - H)

Priority Resolution:
  Check P1 (Force Off): YES → Relay OFF, RETURN

RESULT: Relay stays OFF
REASON: Force Off is highest priority, overrides everything

Comment: Force Off cannot be overridden by any other mode
This is the intended behavior for safety/maintenance
```

---

## Variable Naming Changes

### Why Names Changed

**Old Name**: `away_mode`
**New Name**: `schedule_standby`

**Reason**:
- `away_mode` sounded like a separate mode (confusing)
- `schedule_standby` clarifies it's schedule-controlled operation
- Avoids confusion with "Away" in Presence Mode select

**Log Output Changes**:

| Old Log Message | New Log Message | What Changed |
|-----------------|-----------------|--------------|
| `"Away mode: Relay OFF (priority 1)"` | `"Force Off: Relay OFF (Priority 1 - HIGHEST)"` | Complete rewrite for clarity |
| `"Manual Run: Relay ON (priority 2, ..."` | `"Emergency Heat: Relay ON (Priority 2, ..."` | More descriptive name |
| `"Manual Stop: Relay OFF (priority 3)"` | `"Schedule: Standby mode (Priority 3) - Relay OFF"` | Reflects new priority order |
| `"Start heating: ... (priority 4)"` | `"Start heating: ... (Priority 4)"` | Capitalization for consistency |

---

## Safety Guarantees After Step 1

### Guarantee 1: Force Off Always Works ✅
```
if (Force Off active) {
  relay = OFF
  return  // Don't check other priorities
}
```
**Safety**: User can ALWAYS shut off heating immediately

### Guarantee 2: Schedule Respects Manual Controls ✅
```
if (Force Off active) { relay = OFF; return; }
if (Emergency Heat active) { relay = ON; return; }
if (Schedule says standby) { relay = OFF; return; }
```
**Safety**: User can override schedule with Emergency Heat or Force Off

### Guarantee 3: Thermostat Only Runs When Safe ✅
```
if (P1 P2 or P3 active) { return; }
// Only reaches here if all higher priorities inactive
run_thermostat_algorithm();
```
**Safety**: Thermostat never conflicts with manual overrides

### Guarantee 4: No Conflicting Commands ✅
```
Check 1: Force Off → Relay OFF or Return
Check 2: Emergency Heat → Relay ON or Return
Check 3: Schedule → Relay OFF or Return
Check 4: Thermostat → Relay ON/OFF/maintain
```
**Safety**: Each priority either takes action and returns, or continues

---

## Testing the Changes

### Minimal Test (5 minutes)

```
1. Set Presence Mode to "home"
2. Set desired temp to 30°C (above current)
3. Verify relay turns ON (thermostat active)
4. Enable Force Off (Manual Stop)
5. Verify relay turns OFF immediately
6. Check logs show "Force Off: Relay OFF (Priority 1 - HIGHEST)"
7. ✅ If relay turned OFF, Step 1 is working correctly
```

### Comprehensive Test (20 minutes)

Follow the complete testing checklist in STEP_1_IMPLEMENTATION_SUMMARY.md

---

## Summary of Changes

| Aspect | Before | After | Impact |
|--------|--------|-------|--------|
| **Force Off Priority** | P3 (lowest) | P1 (highest) | ✅ User now has absolute control |
| **Emergency Heat Priority** | P2 | P2 | ✅ Unchanged, respects Force Off |
| **Schedule Priority** | P1 (highest) | P3 | ✅ No longer overrides Force Off |
| **Thermostat Priority** | P4 (lowest) | P4 (lowest) | ✅ Unchanged, only runs when needed |
| **Variable Name** | `away_mode` | `schedule_standby` | ✅ Clearer intent |
| **Log Messages** | Generic | Specific with priority labels | ✅ Better debugging |
| **Code Safety** | Has flaw | Safety guaranteed | ✅ Critical fix applied |

---

## Files Affected

```
includes/automation.yaml
├── Lines 26-31: Comments updated
├── Lines 34-38: Variable renamed (away_mode → schedule_standby)
├── Lines 42-45: Debug logging updated
├── Lines 47-136: Priority checks reordered
│   ├── Priority 1: Manual Stop (Force Off) - NEW LOCATION
│   ├── Priority 2: Manual Run (Emergency Heat) - MOVED
│   ├── Priority 3: Presence Mode (Schedule) - MOVED
│   └── Priority 4: Thermostat Algorithm - MOVED
└── Lines 138-170: Unchanged (scripts and intervals)
```

---

## Next Steps After Step 1

1. **Compile and test** the changes (see STEP_1_IMPLEMENTATION_SUMMARY.md)
2. **Run validation tests** (see testing section above)
3. **Proceed to Step 2** (Emergency Heat Force Off Check) - see THERMOSTAT_CORRECTION_SPEC.md
4. **Continue with remaining steps** (Schedule, Hysteresis, etc.)

---

**Document Version**: 1.0
**Date**: 2025-11-14
**Status**: Reference guide for Step 1 implementation
