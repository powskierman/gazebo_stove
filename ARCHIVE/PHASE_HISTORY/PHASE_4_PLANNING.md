# Phase 4 Planning - Scheduled Heating & Advanced Features

**Status**: Planning Phase (Ready to begin after Phase 3 validation)

**Target Features**: Scheduled heating, automations, notifications, display enhancements

**Estimated Timeline**: 2-3 weeks intensive development

---

## Phase 4 Overview

Phase 4 builds on the solid foundation of Phase 3 by adding **scheduled heating** and **advanced Home Assistant automations**. This brings the system from manual control to smart, programmable heating.

### Primary Goal
Enable users to create heating schedules (weekday/weekend, time-based modes) and automated responses to events (presence detection, manual mode activation).

---

## Implementation Steps

### **Step 1: Understand Current State (Week 1, Day 1)**

**Objective**: Verify Phase 3 is stable and document current capabilities

**Actions**:
1. Run device for 24+ hours in Phase 3 configuration
2. Monitor logs for any errors or anomalies
3. Test all three controls (Presence Mode, Manual Run, Manual Stop)
4. Create stability report
5. Document any issues found

**Deliverables**:
- Stability test results
- Phase 3 validation checklist
- Issue list (if any)

**Why**: Ensures Phase 3 is solid before adding more complexity

---

### **Step 2: Plan Scheduling System (Week 1, Days 2-3)**

**Objective**: Design how scheduling will work

**Decisions to Make**:

1. **Where should schedules live?**
   - Option A: In Home Assistant (via input_select/input_number helpers)
   - Option B: In ESPHome configuration (static, compiled)
   - Option C: Both (flexible + fallback)
   - **Recommendation**: Option A (Home Assistant) for flexibility

2. **What schedule types?**
   - Simple: Weekday/Weekend toggle
   - Intermediate: Daily time-based (wake/sleep times)
   - Advanced: Per-day schedules with different setpoints
   - **Recommendation**: Start with simple, build to advanced

3. **What controls the schedule?**
   - Thermostat desired temperature (automatic adjustment)
   - Presence mode (automatic away at night)
   - Both (coordinated)
   - **Recommendation**: Both for maximum automation

4. **How does it interact with manual modes?**
   - Manual modes always override schedule
   - Schedule can't override manual stop (safety)
   - Manual run timeout re-enables schedule
   - **Clear rules prevent conflicts**

**Example Schedule Structure**:
```yaml
Weekdays:
  06:00 - 08:00: Home mode, 22°C (wake up)
  08:00 - 18:00: Away mode (energy saving)
  18:00 - 22:00: Home mode, 21°C (evening)
  22:00 - 06:00: Away mode (night saving)

Weekends:
  06:00 - 23:00: Home mode, 22°C (all day)
  23:00 - 06:00: Away mode (night saving)
```

**Deliverables**:
- Decision document: Where/how schedules stored
- Schedule types and interactions documented
- Priority matrix (schedule vs. manual modes)

**Why**: Clear design prevents conflicts and implementation mistakes

---

### **Step 3: Implement Home Assistant Helpers (Week 1, Days 4-5)**

**Objective**: Create input entities for schedule configuration

**Configuration to Add**:

1. **Input Select: Weekday Schedule**
   ```yaml
   input_select:
     weekday_schedule:
       name: "Weekday Schedule"
       options:
         - "Energy Saver"
         - "Normal"
         - "Comfort"
         - "Custom"
       icon: mdi:calendar-week
   ```

2. **Input Select: Weekend Schedule**
   - Same options as weekday

3. **Input Datetime: Wake Time**
   - Default: 06:00
   - Allows custom wake time

4. **Input Datetime: Sleep Time**
   - Default: 22:00
   - Allows custom sleep time

5. **Input Number: Daytime Setpoint**
   - Min: 15°C, Max: 28°C
   - Default: 21°C

6. **Input Number: Nighttime Setpoint**
   - Min: 10°C, Max: 20°C
   - Default: 16°C

**Location**: `automations.yaml` or `configuration.yaml`

**Deliverables**:
- Helper entities in Home Assistant
- Visible in Settings → Devices & Services → Helpers
- All entities properly named and organized

**Why**: Creates the UI for users to configure schedules

---

### **Step 4: Create Schedule Automation Logic (Week 2, Days 1-2)**

**Objective**: Implement automations that execute schedules

**Automations to Create**:

1. **"Weekday Morning - Set Home Mode & Temperature"**
   ```yaml
   automation:
     - alias: "Weekday morning heating"
       trigger:
         - platform: time
           at: input_datetime.weekday_wake_time
       condition:
         - condition: time
           weekday: [mon, tue, wed, thu, fri]
       action:
         - service: select.select_option
           target: select.presence_mode
           data:
             option: "home"
         - service: number.set_value
           target: number.desired_temperature
           data:
             value: "{{ states('input_number.daytime_setpoint') | float }}"
   ```

2. **"Weekday Day - Set Away Mode"**
   - Trigger: Wake time + 2 hours (configurable)
   - Action: Set presence to "away"

3. **"Weekday Evening - Set Home Mode"**
   - Trigger: Sleep time - 2 hours
   - Action: Set presence to "home", set daytime temperature

4. **"Weekday Night - Set Away Mode & Lower Temp"**
   - Trigger: Sleep time
   - Action: Set presence to "away", set nighttime temperature

5. **Weekend automations** (similar pattern)

6. **"Override Manual Modes" protection**
   - If manual stop is on: Don't change presence/temp
   - If manual run is on and < 15 min: Don't override
   - Log when schedule can't execute due to override

**Deliverables**:
- 6-10 automations in Home Assistant
- Tested and verified
- Handles edge cases (manual mode conflicts)

**Why**: Executes the schedule automatically

---

### **Step 5: Add Smart Notifications (Week 2, Days 3-4)**

**Objective**: Notify user of important events

**Notifications to Create**:

1. **"Manual Run Activated"**
   ```yaml
   automation:
     - alias: "Notify manual run activated"
       trigger:
         - platform: state
           entity_id: switch.manual_run
           to: "on"
       action:
         - service: notify.notify
           data:
             title: "Gazebo Stove"
             message: "Emergency heating activated (15 min timer)"
   ```

2. **"Manual Run Auto-Disabled"**
   - Notify when 15-minute timeout expires
   - Message: "Emergency heating finished, normal control resumed"

3. **"Manual Stop Activated"**
   - Notify: "Heating locked off for maintenance"

4. **"Schedule Override Prevented"**
   - Notify if manual mode blocked schedule change
   - Message: "Schedule change blocked by Manual Stop"

5. **"Temperature Alert"**
   - Notify if temperature below minimum (frozen?)
   - Notify if temperature above maximum (overheating?)

6. **"Manual Run Remaining Time"** (Optional)
   - Every 5 minutes: "5 minutes of emergency heat remaining"
   - Helps user know when it will auto-disable

**Deliverables**:
- 5-6 notification automations
- Tested with Home Assistant notify service
- Optional: Template message with countdown timer

**Why**: Keeps user informed of system state

---

### **Step 6: Enhance Nextion Display (Week 2, Days 5 - Week 3, Day 1)**

**Objective**: Show schedule and timing information on display

**Display Updates**:

1. **Show Current Schedule**
   - Display: "Weekday - Day Mode" or "Weekend - Night Mode"
   - Updates every 5 minutes based on time

2. **Show Upcoming Event**
   - Display: "Next change in 2 hours 15 minutes"
   - Countdown to next scheduled change

3. **Show Manual Mode Timer**
   - If Manual Run active: "Emergency: 12:45 remaining"
   - Countdown timer format

4. **Show Active Control Mode**
   - "🏠 Home Mode (Thermostat)"
   - "🚗 Away Mode (Energy Saving)"
   - "🔥 Manual Run (Timer)"
   - "⏸️ Manual Stop (Locked)"

5. **Display Pages**
   - Page 0: Current status (existing)
   - Page 1: Schedule information (new)
   - Page 2: History/logs (optional)

**Technical Changes**:
- Update Nextion display file (.tft)
- Add display elements (text boxes, progress bar for timer)
- Update ESPHome automation to send display data
- Create script to calculate and send countdown

**Deliverables**:
- Updated Nextion display file
- ESPHome code to send schedule data to display
- Visible countdown timer during Manual Run
- Clear mode indicators

**Why**: Users can see schedule and remaining time without opening HA

---

### **Step 7: Create Advanced Automations (Week 3, Days 1-2)**

**Objective**: Implement smart features beyond basic scheduling

**Advanced Automations**:

1. **"Auto-Away Based on Presence Sensor"**
   - If all presence sensors = away: Set presence mode to "away"
   - If any presence sensor = home: Set presence mode to "home"
   - Backup: Manual schedule also active

2. **"Weather-Based Adjustments"**
   - If outdoor temp < 0°C: Increase setpoint by 1°C
   - If sunny and indoor temp > 25°C: Lower setpoint by 1°C

3. **"Learning Mode"**
   - Track user manual adjustments
   - Suggest schedule changes based on patterns
   - (Advanced, optional)

4. **"Low Battery Alert"**
   - If any sensor low battery: Notify user

5. **"System Health Check"**
   - Daily: Verify device is responding
   - Monthly: Report system statistics
   - Alert if any component offline

**Deliverables**:
- 3-5 advanced automations
- Tested and working
- Optional: Machine learning for learning mode

**Why**: Makes system truly "smart" not just automated

---

### **Step 8: Create User Documentation (Week 3, Days 3-4)**

**Objective**: Document everything for user reference

**Documents to Create**:

1. **PHASE_4_USER_GUIDE.md**
   - How to set up schedules
   - How to modify schedules
   - Understanding schedule priority vs. manual modes
   - Troubleshooting schedule issues

2. **SCHEDULE_EXAMPLES.md**
   - Weekday/weekend example
   - Energy saver example
   - Comfort example
   - Custom example with explanation

3. **NOTIFICATIONS_GUIDE.md**
   - What each notification means
   - How to enable/disable notifications
   - How to customize notification messages

4. **AUTOMATION_REFERENCE.md**
   - How each automation works
   - What conditions it checks
   - How to modify for custom needs

5. **TROUBLESHOOTING_PHASE_4.md**
   - "Schedule isn't working" → solutions
   - "Notification spam" → fix
   - "Display shows wrong info" → debug

**Deliverables**:
- 5 comprehensive documentation files
- 300+ lines total
- Examples and use cases included

**Why**: Users need to understand how to use new features

---

### **Step 9: Testing & Validation (Week 3, Days 5)**

**Objective**: Thoroughly test all Phase 4 features

**Test Cases**:

1. **Schedule Execution Tests**
   - [ ] Weekday morning: Presence changes at wake time
   - [ ] Weekday day: Presence changes away at configured time
   - [ ] Weekend: Correct schedule executes
   - [ ] Schedule respects manual mode conflicts
   - [ ] Temperature adjustment works with schedule

2. **Notification Tests**
   - [ ] Manual Run triggered → notification received
   - [ ] Manual Run timeout → notification received
   - [ ] Manual Stop activated → notification received
   - [ ] Schedule prevented by override → notification received

3. **Display Tests**
   - [ ] Current schedule shown on display
   - [ ] Countdown timer displays during Manual Run
   - [ ] Mode indicator updated correctly
   - [ ] Display updates every 5 minutes

4. **Advanced Automation Tests**
   - [ ] Presence sensors trigger mode changes
   - [ ] Weather adjustments work
   - [ ] Health check sends monthly report
   - [ ] Low battery alerts notify user

5. **Edge Case Tests**
   - [ ] DST changes handled correctly
   - [ ] Manual override blocks schedule as expected
   - [ ] Schedule resumes after manual override ends
   - [ ] System survives 72-hour continuous operation

**Deliverables**:
- Test results checklist
- Any bugs found and fixed
- Performance metrics (CPU, memory, update frequency)

**Why**: Ensures everything works reliably before final release

---

### **Step 10: Final Polish & Release (Week 3, Days 5 onwards)**

**Objective**: Finalize and document Phase 4 completion

**Final Tasks**:

1. **Code Review**
   - Check all automations for efficiency
   - Optimize any slow-running scripts
   - Clean up unused code

2. **Documentation Review**
   - Ensure all files are clear and complete
   - Add any missing examples
   - Review for typos/clarity

3. **Git Commits**
   - Commit all Phase 4 work
   - Create comprehensive commit messages
   - Tag release (e.g., `phase-4-complete`)

4. **Create PHASE_4_SUMMARY.md**
   - What was accomplished
   - Performance metrics
   - Statistics (automations, lines of code, etc.)
   - Known limitations

5. **Update Documentation Index**
   - Add Phase 4 files to DOCUMENTATION_INDEX.md
   - Link to all Phase 4 guides

**Deliverables**:
- Polished, production-ready code
- Complete documentation
- Git history showing all changes
- Release notes/summary

**Why**: Professional completion of the phase

---

## Phase 4 Decision Points

### Decision 1: Schedule Storage Location
**Options**:
- A) Home Assistant helpers (flexible, user-friendly)
- B) ESPHome YAML (static, compiled)
- C) Database (complex but powerful)

**Recommendation**: Start with **Option A** (Home Assistant helpers)
**Rationale**: Users can modify without recompiling, changes take effect immediately

---

### Decision 2: Schedule Complexity
**Options**:
- A) Simple: Just weekday/weekend toggle
- B) Intermediate: Time-based (wake/sleep times)
- C) Advanced: Minute-by-minute scheduling

**Recommendation**: Start with **Option B**, allow expansion to C
**Rationale**: Covers 80% of use cases, room for enhancement

---

### Decision 3: Manual Mode Interaction
**Options**:
- A) Schedule always overrides manual modes
- B) Manual modes always block schedule
- C) Context-aware (depends on type)

**Recommendation**: **Option C** - Context-aware
- Manual Run: Schedule can't change modes while running (safety)
- Manual Stop: Never overridden by schedule (maintenance safety)
- Schedule: Can change desired temp anytime

**Rationale**: Prevents safety conflicts while maximizing functionality

---

## Phase 4 Success Criteria

✅ **Scheduling Features**:
- [ ] Weekday/weekend schedules implemented
- [ ] Time-based mode changes working
- [ ] Temperature adjustments automatic
- [ ] Schedule respects manual mode safety

✅ **Automations**:
- [ ] All 8+ automations created and tested
- [ ] Notifications sent for key events
- [ ] No schedule conflicts with manual modes
- [ ] System remains stable under continuous operation

✅ **User Experience**:
- [ ] Schedule changes within 1 minute of configured time
- [ ] User notified of important events
- [ ] Display shows schedule information
- [ ] All features accessible from Home Assistant UI

✅ **Documentation**:
- [ ] User guide for schedules
- [ ] Examples for common use cases
- [ ] Troubleshooting guide
- [ ] Automation reference for customization

✅ **Testing**:
- [ ] All test cases passed
- [ ] Edge cases handled (DST, manual overrides, etc.)
- [ ] 72+ hour stability test passed
- [ ] No performance degradation

---

## Estimated Time & Resources

| Task | Time | Effort | Notes |
|------|------|--------|-------|
| Step 1: Validation | 1 day | Easy | Observation & testing |
| Step 2: Planning | 1 day | Easy | Design decisions |
| Step 3: HA Helpers | 0.5 day | Easy | Input entities |
| Step 4: Automations | 2 days | Medium | Main logic |
| Step 5: Notifications | 1 day | Easy | Alert logic |
| Step 6: Display | 2 days | Medium | Nextion programming |
| Step 7: Advanced | 1 day | Medium | Smart features |
| Step 8: Documentation | 1 day | Easy | Writing guides |
| Step 9: Testing | 1 day | Medium | Validation |
| Step 10: Polish | 0.5 day | Easy | Final cleanup |
| **Total** | **~10 days** | **Medium** | **2 weeks intensive** |

---

## Getting Started

When you're ready to begin Phase 4:

1. **Run `/speckit.plan`** to create formal plan template
2. **Start Step 1**: Validate Phase 3 stability
3. **Document findings**: Create Phase 3 validation report
4. **Review decisions**: Make choices on schedule storage, complexity, interactions
5. **Begin Step 3**: Create Home Assistant helper entities

---

## Quick Reference: Phase 4 Checklist

### Pre-Implementation
- [ ] Phase 3 stable for 24+ hours
- [ ] All three controls (Presence, Manual Run/Stop) working
- [ ] No errors in device logs

### Planning Phase
- [ ] Decisions made on schedule storage (HA vs. ESPHome)
- [ ] Schedule structure documented
- [ ] Priority rules defined (manual vs. schedule)

### Implementation Phase
- [ ] Home Assistant helpers created
- [ ] Automations implemented (6-10 total)
- [ ] Notifications added for key events
- [ ] Display updated with schedule info
- [ ] Advanced features added (presence, weather, health check)

### Testing Phase
- [ ] All automations triggered and working
- [ ] Notifications delivered correctly
- [ ] Display updated correctly
- [ ] Edge cases tested (DST, manual overrides)
- [ ] 72-hour stability test passed

### Documentation Phase
- [ ] User guides written
- [ ] Examples provided
- [ ] Troubleshooting guide created
- [ ] All files properly organized

### Release Phase
- [ ] Code reviewed and optimized
- [ ] Git commits created
- [ ] Phase 4 summary completed
- [ ] Documentation index updated

---

**Ready to start Phase 4?** Let me know and we can begin Step 1: Stability Validation!
