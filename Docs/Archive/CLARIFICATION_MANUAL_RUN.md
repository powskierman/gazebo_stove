# Clarification: Manual Run Behavior

## The Misunderstanding

The summary said:
```
3. **Manual Run** → Always ON (15-min max)
```

This was **misleading wording**. It made it sound like Manual Run is automatically active, which is NOT correct.

## What Manual Run Actually Does

✅ **Manual Run is OFF by default** - The switch stays OFF until you toggle it

✅ **Stays OFF unless you enable it** - No automatic activation

✅ **Only when YOU toggle it ON** does the relay turn ON for 15 minutes

✅ **Auto-disables after 15 minutes** - Safe timeout limit

✅ **You can disable it anytime** - Toggle it OFF to stop early

## The Correct Wording

The priority logic should be understood as:

```
1. Away Mode (HIGHEST PRIORITY)
   └─ If Presence = "away" → Relay OFF (always)

2. Manual Stop
   └─ If Manual Stop is ENABLED → Relay OFF

3. Manual Run
   └─ If Manual Run is ENABLED AND < 15 minutes → Relay ON

4. Thermostat Algorithm (LOWEST PRIORITY)
   └─ Temperature-based control (Phase 2)
```

**The key point**: The "(if enabled)" part is crucial. All overrides are OFF by default and require explicit user action.

## Real-World Behavior

### Example: You activate Manual Run

**Step 1**: Manual Run switch is OFF (default)
- Relay status: Controlled by thermostat or other modes

**Step 2**: You toggle Manual Run ON
- Relay: Turns ON immediately
- Timer: Starts counting down from 15:00
- Status: Manual Run active

**Step 3**: Wait 15 minutes (or toggle OFF early)
- After 15 minutes: Manual Run auto-disables
- Relay: Returns to thermostat control
- Timer: Reset to 0

## Why This Matters for Safety

1. **Accidental activation impossible** - Requires deliberate toggle
2. **Time-limited heating** - 15-minute hard limit prevents overheating
3. **Default OFF** - System is conservative (heating disabled by default)
4. **Easy to disable** - Just toggle the switch OFF if you need to stop early

## Summary Table

| State | What Happens |
|-------|--------------|
| Manual Run OFF (default) | Relay controlled by other modes/thermostat |
| Manual Run ON (0-14:59) | Relay ON for emergency heat |
| Manual Run ON (15:00+) | Auto-disables, reverts to other control |
| Manual Run toggle OFF | Heating re-evaluated by priority system |

---

**Bottom Line**: Manual Run is a **manual emergency override that must be explicitly enabled**. It's not automatic, not always-on, and not dangerous because of the 15-minute timeout.
