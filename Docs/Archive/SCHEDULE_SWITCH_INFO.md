# Heating Schedule Switch - Expected Behavior

## What It Should Do

The **"Gazebo Heating Schedule"** switch (`input_boolean.gazebo_schedule_enabled`) is a simple enable/disable toggle for the automatic schedule feature.

### When ON (Enabled):
- ✅ Schedule automations run at configured times
- ✅ Automatically sets Presence Mode to "home" at Home Mode Time
- ✅ Automatically sets Presence Mode to "away" at Away Mode Time
- ✅ Does NOT affect Force Off switch
- ✅ Does NOT affect Emergency Heat switch
- ✅ Does NOT control relay directly

### When OFF (Disabled):
- ✅ Schedule automations do NOT run
- ✅ No automatic mode changes
- ✅ User must manually control Presence Mode
- ✅ Does NOT affect Force Off switch
- ✅ Does NOT affect Emergency Heat switch

## What It Should NOT Do

❌ **Should NOT turn on Force Off switch**  
❌ **Should NOT turn off Force Off switch**  
❌ **Should NOT control Emergency Heat**  
❌ **Should NOT directly control the relay**

## Troubleshooting

If turning on the Heating Schedule switch also turns on Force Off:

1. **Check for conflicting automations**:
   - Go to Settings → Automations & Scenes → Automations
   - Look for any automation that triggers on `input_boolean.gazebo_schedule_enabled`
   - Check if any automation targets `switch.manual_stop` or `switch.force_off`

2. **Check Home Assistant logs**:
   - Settings → System → Logs
   - Look for errors when toggling the schedule switch
   - Check for any automation execution messages

3. **Verify entity IDs**:
   - Make sure `input_boolean.gazebo_schedule_enabled` is unique
   - Make sure `switch.manual_stop` entity ID is correct
   - Check for entity ID conflicts

4. **Check dashboard configuration**:
   - If switches are in the same card, verify they're not accidentally linked
   - Check if there's a custom card or template that might be causing issues

## Expected Independent Behavior

These switches should work completely independently:

- **Heating Schedule** (`input_boolean.gazebo_schedule_enabled`): Controls schedule automations only
- **Force Off** (`switch.manual_stop`): Controls manual safety override
- **Emergency Heat** (`switch.manual_run`): Controls 15-minute emergency heating
- **Presence Mode** (`select.presence_mode`): Controls home/away state

None of these should affect each other except through the priority system logic in ESPHome.

