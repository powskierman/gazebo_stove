# Critical: Safe Backup Recovery & Configuration Restoration Plan

**Date:** November 16, 2025
**Status:** DETAILED RECOVERY PROCEDURE
**Risk Level:** ⚠️ HIGH - Follow steps EXACTLY to avoid data loss

---

## ⚠️ CRITICAL WARNINGS

### DO NOT:
- ❌ Do NOT restore the entire backup without reading this plan
- ❌ Do NOT overwrite your current configuration.yaml without backing it up
- ❌ Do NOT delete the current working config before testing recovery
- ❌ Do NOT disconnect from your Home Assistant instance mid-process
- ❌ Do NOT assume "restore" will merge - it OVERWRITES

### DO:
- ✅ DO back up your current working config FIRST
- ✅ DO have SSH access to your Home Assistant ready
- ✅ DO test on a copy before modifying originals
- ✅ DO keep detailed notes of each step
- ✅ DO verify at each stage before moving forward

---

## The Situation

**What you have now:**
- Current HA with ONLY Gazebo Stove config
- Missing: MQTT, MariaDB, automations, scripts, etc.
- Working: ESPHome integration, sensors, weather

**What you had on Nov 14:**
- Complete config with all integrations
- All automations, scripts, groups
- MQTT, MariaDB, RTL_433, and more

**The Risk:**
- Full restore overwrites current working setup
- BUT we need some things from the old backup
- We don't want to lose the new sensor setup

**The Solution:**
- Extract ONLY the missing config pieces from backup
- Merge them with current config
- Test thoroughly before finalizing

---

## Phase 1: Preparation (BEFORE touching anything)

### Step 1.1: Create Current Backup

**This protects your current working state:**

```bash
# SSH into your Home Assistant
ssh root@192.168.1.X  # Your HA IP

# Navigate to config directory
cd /config

# Create a full backup of current state
tar -czf config-backup-nov-16-CURRENT-WORKING.tar.gz \
  configuration.yaml \
  automations.yaml \
  scripts.yaml \
  groups.yaml \
  scenes.yaml \
  secrets.yaml \
  2>/dev/null

# Copy to safe location
cp config-backup-nov-16-CURRENT-WORKING.tar.gz /mnt/data/backup/
```

**What this does:**
- Captures your current working config
- Includes all the recent changes
- Stores a copy in data directory
- If anything goes wrong, you can restore THIS

### Step 1.2: Verify Current Config Works

**Before proceeding:**

1. Go to Home Assistant UI
2. Settings → Automations & Scenes → YAML
3. Click **"Validate YAML"**
4. Should show: ✅ **No errors**
5. If errors exist, STOP here and fix them first

### Step 1.3: Download the Nov 14 Backup

**On your computer:**

```bash
# Navigate to Downloads
cd ~/Downloads

# List the backup
ls -lh "Full Backup 2025-11-14 03_00_00"

# We need to extract the encrypted backup
# The backup is password protected (AES128)
# You'll need the backup password from Home Assistant
```

**Get the password:**
1. Home Assistant Settings → System → Backups
2. Click the Nov 14 backup
3. It will ask for password to restore
4. COPY that password for use in next step

**⚠️ DO NOT restore yet - we're just extracting files**

---

## Phase 2: Extract Configuration From Encrypted Backup

### Step 2.1: Identify Backup Format

**The backup is encrypted (SecureTar format):**

```bash
cd "/Users/michel/Downloads/Full Backup 2025-11-14 03_00_00"
file homeassistant.tar.gz
# Output: data (encrypted)
```

**This means:**
- Standard tar commands won't work
- Must use Home Assistant to extract
- OR use specialized tools if you have them

### Step 2.2: Use Home Assistant Restore to Extract

**Best method - use the HA UI:**

1. **In Home Assistant:**
   - Settings → System → Backups
   - Click Nov 14 backup
   - Click "RESTORE" button

2. **HA will:**
   - Ask for backup password (from step 1.3)
   - Extract the encrypted archive
   - Show you the contents
   - Ask to confirm restore (DON'T click this yet!)

3. **At the confirmation screen:**
   - DO NOT click "RESTORE"
   - Instead, SSH into your system
   - The extracted files are now temporarily available
   - Copy what you need before backing out

### Step 2.3: Extract Specific Files Via SSH

**Once the restore dialog is open (but BEFORE confirming):**

```bash
# SSH into Home Assistant
ssh root@192.168.1.X

# The backup is being prepared in a temp location
# Find it:
find /tmp -name "configuration.yaml" -type f 2>/dev/null

# Usually found at:
ls -la /var/lib/docker/volumes/*/data/

# OR look in the home assistant container:
docker ps  # Find container ID
docker exec <container_id> ls /config/

# Copy the important config files
# From temporary extracted location to /tmp on host
cp /var/lib/docker/volumes/*/data/configuration.yaml /tmp/config-nov14.yaml
cp /var/lib/docker/volumes/*/data/automations.yaml /tmp/automations-nov14.yaml
cp /var/lib/docker/volumes/*/data/scripts.yaml /tmp/scripts-nov14.yaml
cp /var/lib/docker/volumes/*/data/groups.yaml /tmp/groups-nov14.yaml
cp /var/lib/docker/volumes/*/data/secrets.yaml /tmp/secrets-nov14.yaml

# List what you got
ls -la /tmp/*-nov14.yaml
```

### Step 2.4: Download the Extracted Files

**From your computer:**

```bash
# Create a working directory
mkdir -p ~/GazeboStove-Recovery
cd ~/GazeboStove-Recovery

# Download each extracted file from HA
scp root@192.168.1.X:/tmp/config-nov14.yaml ./
scp root@192.168.1.X:/tmp/automations-nov14.yaml ./
scp root@192.168.1.X:/tmp/scripts-nov14.yaml ./
scp root@192.168.1.X:/tmp/groups-nov14.yaml ./
scp root@192.168.1.X:/tmp/secrets-nov14.yaml ./

# Verify downloads
ls -lh ~/GazeboStove-Recovery/
```

### Step 2.5: Cancel the Restore

**In Home Assistant:**
1. Go back to Backups
2. Close the restore dialog (click X or Back)
3. Do NOT confirm the restore
4. Your current config remains untouched ✅

---

## Phase 3: Analyze What Was Lost

### Step 3.1: Compare Configurations

**On your computer:**

```bash
# Create comparison directory
mkdir -p ~/GazeboStove-Recovery/analysis
cd ~/GazeboStove-Recovery

# Compare the Nov 14 config with your current
# (Need to copy current config from HA first)

scp root@192.168.1.X:/config/configuration.yaml ./config-current.yaml

# Now compare
diff -u config-current.yaml config-nov14.yaml > analysis/config-differences.txt
diff -u <(ls) config-nov14.yaml > analysis/what-was-in-backup.txt

# View differences
cat analysis/config-differences.txt | head -50
```

### Step 3.2: Identify Critical Missing Sections

**Look for these in Nov 14 config:**

```bash
# Check for MQTT configuration
grep -n "^mqtt:" config-nov14.yaml

# Check for MariaDB configuration
grep -n "^database:\|^recorder:" config-nov14.yaml

# Check for automations include
grep -n "automation:" config-nov14.yaml

# Check for scripts include
grep -n "script:" config-nov14.yaml

# Check for groups
grep -n "^group:" config-nov14.yaml

# Check for custom templates
grep -n "^template:" config-nov14.yaml

# Check for custom sensors
grep -n "^sensor:" config-nov14.yaml

# Check for switches/binary sensors
grep -n "^switch:\|^binary_sensor:" config-nov14.yaml
```

**Keep a list of what you find!**

### Step 3.3: Extract These Sections

**For each section found:**

```bash
# Example: Extract MQTT section
sed -n '/^mqtt:/,/^[a-z]/p' config-nov14.yaml > extracted-mqtt.yaml

# Example: Extract recorder section
sed -n '/^recorder:/,/^[a-z]/p' config-nov14.yaml > extracted-recorder.yaml

# Repeat for each major section you found
```

---

## Phase 4: Merge Configurations Safely

### Step 4.1: Plan the Merge

**Create a merge strategy document:**

```
MERGE PLAN:
===========

KEEP from current (Nov 16):
  ✓ All sensor: time_date configuration
  ✓ All weather: openmeteo configuration
  ✓ All input_boolean (gazebo_schedule_enabled)
  ✓ All input_datetime (gazebo home/away times)
  ✓ All Gazebo Stove specific configs

ADD from Nov 14 backup:
  ? mqtt: section (if it exists)
  ? database: or recorder: section
  ? All automations.yaml
  ? All scripts.yaml
  ? All groups.yaml
  ? Custom sensor definitions
  ? Custom templates
  ? RTL_433 configuration
  ? Matter server config
  ? Any other integrations

DO NOT include:
  ✗ Duplicate sensor definitions
  ✗ Conflicting configurations
  ✗ Old Gazebo config (we have the new version)
```

### Step 4.2: Create Merged Configuration

**Build it step by step:**

```bash
cd ~/GazeboStove-Recovery

# Start with current working config
cp config-current.yaml config-merged.yaml

# Now, carefully add sections from Nov 14
# Edit config-merged.yaml and add missing sections

# Instructions:
# 1. Open config-merged.yaml in editor
# 2. Keep everything that's already there
# 3. Find each missing section from Nov 14 backup
# 4. Add it to the merged file
# 5. Make sure YAML indentation is perfect
# 6. Check for duplicate keys
```

**DO NOT merge yet - review first!**

### Step 4.3: Validate Merged Configuration

```bash
# Check YAML syntax
python3 -m yaml config-merged.yaml

# If you get errors, fix them before proceeding
# Errors usually mean:
# - Indentation problems
# - Duplicate keys
# - Invalid syntax

# Once validated, copy to a safe location
cp config-merged.yaml config-merged-VALIDATED.yaml
```

---

## Phase 5: Test Merged Configuration (CRITICAL)

### Step 5.1: Test on a Copy First

**Do NOT apply to live HA yet:**

```bash
# SSH into HA
ssh root@192.168.1.X

# Create a test copy of current config
cp /config/configuration.yaml /config/configuration.yaml.backup-nov16-working

# Copy the merged version for testing
# (But DON'T make it active yet)
cp /config/configuration.yaml /config/configuration.yaml.old

# Make a test directory
mkdir -p /config/test
cp config-merged-VALIDATED.yaml /config/test/configuration.yaml
```

### Step 5.2: Validate in Home Assistant UI

**With the test file in place:**

1. Go to Home Assistant Settings → Automations & Scenes → YAML
2. Click "Validate YAML"
3. Check if validation passes
4. If it fails:
   - Note the error
   - STOP here
   - Fix the error in your merged config
   - Return to Phase 4.3

5. If it passes ✅:
   - Note successful validation
   - Continue to next step

### Step 5.3: Check Entity Count

**Before and after:**

```bash
# Count entities before merge
curl -X GET http://192.168.1.X:8123/api/states \
  -H "Authorization: Bearer YOUR_TOKEN" \
  | jq 'length'
# Note this number

# This gives you baseline of current entities
# You should see MORE entities after merge (not fewer!)
```

---

## Phase 6: Apply Merged Configuration

### Step 6.1: Backup Current (Again)

**One final safety backup:**

```bash
ssh root@192.168.1.X

cd /config

# Backup current working version
cp configuration.yaml configuration.yaml.BACKUP-BEFORE-MERGE-$(date +%s)

# Verify backup exists
ls -lh configuration.yaml.BACKUP-*
```

### Step 6.2: Apply Merged Configuration

**Do this during low-usage time (not during automations):**

```bash
# Upload merged config
scp config-merged-VALIDATED.yaml root@192.168.1.X:/config/configuration.yaml

# Verify upload
ssh root@192.168.1.X "ls -lh /config/configuration.yaml"
```

### Step 6.3: Reload Configuration in HA

**Two methods - choose ONE:**

**Method A: Via UI (Recommended)**
1. Settings → Automations & Scenes → YAML
2. Click "Reload All YAML" or "Reload Automations"
3. Wait for completion message
4. Check for errors in Notifications

**Method B: Via SSH**
```bash
ssh root@192.168.1.X

# Reload automations via HA service call
# This is safer than full restart

# Or restart if needed
hassio ha restart

# Wait 2-3 minutes
sleep 180

# Verify restart worked
curl http://192.168.1.X:8123/api/states \
  -H "Authorization: Bearer YOUR_TOKEN" | jq 'length'
```

### Step 6.4: Monitor the Process

**Watch the logs:**

```bash
# SSH in and watch logs
ssh root@192.168.1.X
docker logs homeassistant -f

# Look for:
# ✅ "Configuration valid"
# ✅ "Starting Home Assistant"
# ❌ Any ERROR messages
# ❌ Any CRITICAL messages

# Press Ctrl+C when stable
```

---

## Phase 7: Verify Recovery Success

### Step 7.1: Check HA Is Running

```bash
# Can you access the UI?
# Open: http://your-ha-ip:8123

# Login and verify you can navigate
# Should NOT see errors or warnings
```

### Step 7.2: Check Entities Restored

**In Home Assistant:**

1. Settings → Devices & Services → Entities
2. Search for entities that SHOULD exist:
   - [ ] `automation.*` (means automations.yaml loaded)
   - [ ] `script.*` (means scripts.yaml loaded)
   - [ ] `group.*` (means groups.yaml loaded)
   - [ ] MQTT sensors (if mqtt configured)
   - [ ] RTL_433 sensors (if configured)
   - [ ] All Gazebo entities

**Count check:**
- Should have MORE entities than before merge
- If you have FEWER, something went wrong

### Step 7.3: Check Automations Work

**Test critical automations:**

1. Go to Settings → Automations & Scenes → Automations
2. Look for your automations from Nov 14
3. Manually trigger one to test
4. Check logs for execution

### Step 7.4: Check Integrations

**In Home Assistant:**

1. Settings → Devices & Services → Integrations
2. Verify you see:
   - [ ] MQTT (if configured)
   - [ ] MariaDB/Database (if configured)
   - [ ] ESPHome
   - [ ] Matter (if configured)
   - [ ] Any others from Nov 14

### Step 7.5: Check Logs for Errors

**Settings → System → Logs:**

```
Should show:
✅ "Configuration valid"
✅ "Starting Home Assistant"
✅ "All modules loaded"

Should NOT show:
❌ "Error loading integration"
❌ "Invalid configuration"
❌ "Connection refused" (for critical services)
```

---

## Phase 8: Rollback Plan (If Something Breaks)

### If Merge Didn't Work:

**Restore previous version:**

```bash
ssh root@192.168.1.X
cd /config

# List your backups
ls -lh configuration.yaml.BACKUP-*

# Restore the latest backup
# (This is your Nov 16 working version)
cp configuration.yaml.BACKUP-<timestamp> configuration.yaml

# Reload
hassio ha restart

# Wait 2 minutes
sleep 120

# Verify back online
curl http://192.168.1.X:8123/api/states \
  -H "Authorization: Bearer YOUR_TOKEN" | jq 'length'
```

### Complete Rollback (Back to Nov 14):

**If you need to go back to Nov 14 entirely:**

```bash
# In Home Assistant:
# Settings → System → Backups
# Click Nov 14 backup
# Click "RESTORE"
# Enter password
# Confirm RESTORE

# This OVERWRITES everything with Nov 14 state
# Only do this if merge completely failed
# Wait 10-15 minutes for restore to complete
```

---

## SAFE RECOVERY SUMMARY

### The Safest Path Forward:

```
1. ✅ Create backup of current working config (Step 1.1)
2. ✅ Extract Nov 14 config files (Step 2)
3. ✅ Analyze what was lost (Step 3)
4. ✅ Create merged configuration (Step 4)
5. ✅ Validate merged config (Step 4.3)
6. ✅ Test merged config on copy (Step 5)
7. ✅ Backup before applying (Step 6.1)
8. ✅ Apply merged config (Step 6.2-6.3)
9. ✅ Verify everything works (Step 7)
10. ✅ Keep backup for emergency rollback (Step 8)
```

### Timeline:
- **Preparation:** 15 minutes
- **Extraction:** 20 minutes
- **Analysis:** 10 minutes
- **Merge:** 30 minutes
- **Testing:** 15 minutes
- **Apply:** 10 minutes
- **Verification:** 10 minutes
- **Total:** ~2 hours (plenty of time for safe recovery)

### Critical Checkpoints:

| Step | Question | Answer | Continue? |
|------|----------|--------|-----------|
| 1.2 | YAML validates? | ✅ Yes | Go ahead |
| 4.3 | Merged YAML validates? | ✅ Yes | Go ahead |
| 5.2 | Test validates? | ✅ Yes | Go ahead |
| 7.2 | Entities restored? | ✅ Yes | Go ahead |
| 7.4 | Integrations loaded? | ✅ Yes | Done! |

**If ANY answer is ❌ NO:**
- STOP immediately
- Go to Phase 8 (Rollback)
- Do NOT continue

---

## Things That Could Go Wrong & Fixes

### Problem: "YAML validation fails"
**Solution:**
- Fix indentation (YAML is very particular)
- Remove duplicate keys
- Check for special characters needing quotes
- Use an online YAML validator

### Problem: "Entities disappeared after merge"
**Solution:**
- Check automations.yaml and scripts.yaml are included
- Check configuration.yaml include directives
- Reload YAML in HA UI
- Check logs for load errors

### Problem: "Integrations won't connect"
**Solution:**
- Check passwords/tokens in secrets.yaml
- Check integration config syntax
- Restart the specific integration
- Check Home Assistant logs

### Problem: "Can't extract files from encrypted backup"
**Solution:**
- Use Home Assistant UI restore process
- SSH in while restore dialog is open
- Files will be in /tmp or /var/lib/docker/volumes

### Problem: "Configuration.yaml is huge and unclear"
**Solution:**
- Break it into packages
- Use !include directives for each integration
- Modularize the config for clarity
- See configuration package examples in docs

---

## Do Not Proceed Without:

- [ ] Reading this entire plan
- [ ] Understanding each phase
- [ ] Having SSH access to HA ready
- [ ] Having backup password from HA
- [ ] 2+ hours of time available
- [ ] Having a way to rollback if needed
- [ ] Understanding the risks

---

## Questions Before You Start?

Before executing this plan:
1. Do you have SSH access to your HA?
2. Do you know the backup password?
3. Can you access your HA UI?
4. Do you understand the rollback process?
5. Do you have 2+ hours available?

**Answer YES to all before proceeding.**

---

**Status:** Ready to execute
**Risk:** Medium (with this plan)
**Recovery:** Yes, fully reversible
**Estimated Time:** 2 hours
**Final Result:** Nov 14 config merged with Nov 16 improvements

