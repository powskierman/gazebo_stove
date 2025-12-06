# Backup Recovery: Quick Start Checklist

**⚠️ READ THE FULL PLAN FIRST: BACKUP_RECOVERY_PLAN.md**

This is a checklist, not a replacement for the detailed plan!

---

## Pre-Recovery Checklist

- [ ] Read entire BACKUP_RECOVERY_PLAN.md
- [ ] Have SSH access to Home Assistant ready
- [ ] Know your Home Assistant IP address
- [ ] Know your backup password (from HA Settings → Backups)
- [ ] Have 2+ hours available
- [ ] Understand you can rollback if something goes wrong
- [ ] Created backup of current working config (in HA)

---

## Phase 1: Preparation

```bash
# 1. SSH to Home Assistant
ssh root@YOUR-HA-IP

# 2. Backup current working config
cd /config
tar -czf config-backup-nov-16-CURRENT-WORKING.tar.gz \
  configuration.yaml automations.yaml scripts.yaml groups.yaml \
  scenes.yaml secrets.yaml 2>/dev/null

# 3. Verify YAML validates in HA UI
# Settings → Automations & Scenes → YAML → Click "Validate YAML"
# Should show: ✅ Valid YAML
```

---

## Phase 2: Extract From Backup

```bash
# 1. In Home Assistant UI:
#    Settings → System → Backups
#    Click Nov 14 backup
#    Click "RESTORE"
#    Enter backup password
#    DO NOT CONFIRM - just start the extraction process

# 2. SSH to HA while restore dialog is open
ssh root@YOUR-HA-IP

# 3. Find and copy extracted files
find /tmp -name "configuration.yaml" -type f 2>/dev/null
# Note the path

# 4. Copy to /tmp for transfer (example path)
cp /var/lib/docker/volumes/*/data/configuration.yaml /tmp/config-nov14.yaml
cp /var/lib/docker/volumes/*/data/automations.yaml /tmp/automations-nov14.yaml
cp /var/lib/docker/volumes/*/data/scripts.yaml /tmp/scripts-nov14.yaml
cp /var/lib/docker/volumes/*/data/groups.yaml /tmp/groups-nov14.yaml

# 5. On your computer, download
mkdir -p ~/GazeboStove-Recovery
scp root@YOUR-HA-IP:/tmp/*-nov14.yaml ~/GazeboStove-Recovery/

# 6. Cancel restore in HA UI (click X)
#    DO NOT CONFIRM RESTORE
```

---

## Phase 3: Analyze

```bash
# Download current config
cd ~/GazeboStove-Recovery
scp root@YOUR-HA-IP:/config/configuration.yaml ./config-current.yaml

# Compare
diff -u config-current.yaml config-nov14.yaml | head -100

# Look for missing sections:
grep -n "^mqtt:\|^database:\|^recorder:\|^template:" config-nov14.yaml

# Note what's missing for next step
```

---

## Phase 4: Merge

```bash
# 1. Copy current as base
cp config-current.yaml config-merged.yaml

# 2. Open config-merged.yaml in text editor
#    Add missing sections from config-nov14.yaml
#    - mqtt: section
#    - database: or recorder: section
#    - Custom sensors
#    - Custom templates
#    Keep: time_date, openmeteo, gazebo config

# 3. Validate merged YAML
python3 -m yaml config-merged.yaml
# Should show no errors

# 4. Save as validated
cp config-merged.yaml config-merged-VALIDATED.yaml
```

---

## Phase 5: Test

```bash
# 1. In Home Assistant UI:
#    Settings → Automations & Scenes → YAML
#    Click "Validate YAML"
#    Should pass without errors ✅

# 2. If validation fails:
#    Go back to Phase 4
#    Fix the YAML errors
#    Test again

# 3. If validation passes:
#    Continue to Phase 6
```

---

## Phase 6: Apply

```bash
# 1. Backup before merge (IMPORTANT!)
ssh root@YOUR-HA-IP
cd /config
cp configuration.yaml configuration.yaml.BACKUP-BEFORE-MERGE-$(date +%s)

# 2. Upload merged config
scp config-merged-VALIDATED.yaml root@YOUR-HA-IP:/config/configuration.yaml

# 3. Reload in Home Assistant UI:
#    Settings → Automations & Scenes → YAML
#    Click "Reload All YAML"
#    Wait for completion
```

---

## Phase 7: Verify

**Check these in Home Assistant:**

- [ ] Settings → Devices & Services → Entities
  - Search "automation" - should see automations
  - Search "script" - should see scripts
  - Search "group" - should see groups
  - Search "mqtt" - should see MQTT sensors (if configured)
  - Search "rtl" - should see RTL_433 sensors (if configured)

- [ ] Settings → Devices & Services → Integrations
  - MQTT should be connected (if configured)
  - ESPHome should be connected
  - MariaDB should be connected (if configured)

- [ ] Settings → System → Logs
  - Should show "Configuration valid"
  - Should NOT show errors

- [ ] Settings → Automations & Scenes → Automations
  - Should see your automations from Nov 14

**If all checkmarks pass: ✅ RECOVERY SUCCESSFUL**

**If something fails: Go to Phase 8 (Rollback)**

---

## Phase 8: Rollback (If Needed)

### Quick Rollback (Last Backup):
```bash
ssh root@YOUR-HA-IP
cd /config

# Restore your Nov 16 backup
cp configuration.yaml.BACKUP-BEFORE-MERGE-* configuration.yaml

# Reload
docker exec homeassistant hassio ha restart

# Wait 2-3 minutes
sleep 180

# Check it's back
curl http://YOUR-HA-IP:8123/api/states \
  -H "Authorization: Bearer YOUR_TOKEN" | jq 'length'
```

### Full Rollback to Nov 14:
1. Home Assistant Settings → System → Backups
2. Click Nov 14 backup
3. Click "RESTORE"
4. Enter password
5. Click "RESTORE BACKUP"
6. Wait 10-15 minutes

---

## Emergency Contacts

### If you get stuck:

1. **Check:** Do you still have your backups?
   - Current working config (backup-nov-16-CURRENT-WORKING.tar.gz)
   - Nov 14 backup (encrypted)
   - ALWAYS recoverable

2. **Don't panic:** You can't lose data if you:
   - Have backups (you do)
   - Haven't deleted files (you haven't)
   - Can rollback (you can)

3. **Ask for help:** With detailed error messages

---

## Success Indicators

**After recovery completes, you should have:**

✅ All Nov 14 configurations (MQTT, automations, scripts, etc.)
✅ All Nov 16 improvements (time_date sensors, weather, Gazebo config)
✅ No errors in logs
✅ All entities visible
✅ All integrations connected
✅ Automations running

**Estimated time: 2 hours**
**Risk level: Low (with this plan)**
**Reversibility: Complete (can rollback anytime)**

---

## IF YOU'RE UNSURE

**Do NOT proceed** until you:
1. ✅ Read the FULL plan (BACKUP_RECOVERY_PLAN.md)
2. ✅ Understand each phase
3. ✅ Know how to rollback
4. ✅ Have time available
5. ✅ Have confirmed backups exist

**Better to take 2 hours and do it right than rush and lose data.**

