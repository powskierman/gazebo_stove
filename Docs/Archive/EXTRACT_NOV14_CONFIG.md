# Extract configuration.yaml from Nov 14 Backup

**Status:** Ready to execute
**Time Required:** 15-20 minutes
**Risk Level:** ⚠️ MEDIUM - Read carefully before executing
**Reversibility:** 100% - No changes to HA yet

---

## Why This Is Needed

Your current `/Volumes/config/configuration.yaml` is incomplete (only 179 lines with Gazebo config). The Nov 14 backup contains the full, working configuration.yaml with all the core sections that are currently missing.

**What's Missing:**
- ❌ `homeassistant:` section (name, timezone, coordinates)
- ❌ `frontend:` section
- ❌ `http:` section (proxy, SSL settings)
- ❌ `automation: !include automations.yaml`
- ❌ `script: !include scripts.yaml`
- ❌ `scene: !include scenes.yaml`
- ❌ `group: !include groups.yaml`
- ❌ `logger:` section
- ❌ `history:` section
- ❌ `mqtt:` section
- ❌ `recorder:` or `database:` section
- ❌ `tts:` section

---

## Prerequisites

Before you start, verify you have:

- [ ] SSH access to your HA instance at 192.168.2.10
- [ ] Can reach Home Assistant UI at http://192.168.2.10:8123
- [ ] Password for Nov 14 backup (you set this when backups were created)
- [ ] About 20 minutes of uninterrupted time

---

## Step 1: Verify SSH Access

Run this command on your Mac terminal to verify you can SSH to HA:

```bash
ssh root@192.168.2.10
```

**Expected output:**
```
Last login: [date]
core-ssh ~]$
```

**If you get "Permission denied":**
- Try without password: `ssh root@192.168.2.10`
- If prompted for password, try password from your HA secrets
- If still fails, see **Troubleshooting** section below

**If successful:**
- Type `exit` to exit SSH
- Continue to Step 2

---

## Step 2: Find Backup File on HA System

SSH into Home Assistant and locate the backup file:

```bash
ssh root@192.168.2.10
cd /backup
ls -lh *.tar.gz | grep "2025-11-14"
exit
```

**Expected output:**
```
-rw-r--r-- 1 root root 1.2G Nov 14 03:00:00 Full_Backup_2025-11-14_03_00_00
```

**Note the exact filename.** It should be something like `Full_Backup_2025-11-14_03_00_00` or similar.

---

## Step 3: Understand the Backup Format

Home Assistant backups are **encrypted SecureTar format**:

- ✅ Single `.tar.gz` file
- ✅ Encrypted with AES-128
- ✅ Requires backup password to extract
- ✅ Contains: configuration.yaml, automations.yaml, all entities, database, etc.

To extract the configuration.yaml, we'll:
1. Mount/extract the backup in Home Assistant
2. Copy the configuration.yaml out
3. Cancel the restore (don't complete it)

---

## Step 4: Start Backup Restore (Don't Complete!)

**In Home Assistant UI:**

1. Go to **Settings** → **System** → **Backups**
2. Find the **Nov 14 backup** in the list
3. Click on it
4. Click the **"RESTORE"** button (DO NOT complete the restore!)
5. When it starts extracting, the files become accessible on the system
6. Leave this restore window **OPEN** (don't close it, don't cancel it yet)

---

## Step 5: SSH and Extract Configuration While Restore Is Open

While the restore process is running (files are being extracted), SSH in and copy the configuration.yaml:

```bash
ssh root@192.168.2.10

# Navigate to temporary backup extraction location
cd /tmp
ls -la | grep -i backup

# Or try common locations:
cd /var/lib/docker/volumes
find . -name "configuration.yaml" -type f 2>/dev/null
```

**Expected:** You should find a temporary directory with extracted backup files.

```bash
# Once you find the location, copy configuration.yaml
# For example, if it's in a docker volume:
cp /var/lib/docker/volumes/*/data/configuration.yaml /tmp/configuration-nov14.yaml

# Verify the file was copied:
ls -lh /tmp/configuration-nov14.yaml
wc -l /tmp/configuration-nov14.yaml
```

**Expected file size:** ~2-5 KB
**Expected line count:** 150-300 lines

---

## Step 6: Copy to Your Mac

Once you have the file on the HA system, copy it to your Mac:

```bash
# On your Mac terminal (NOT in SSH):
scp root@192.168.2.10:/tmp/configuration-nov14.yaml ~/configuration-nov14-backup.yaml

# Verify the copy:
ls -lh ~/configuration-nov14-backup.yaml
wc -l ~/configuration-nov14-backup.yaml
```

**Expected:** File should be 1-5 KB with 150-300 lines

---

## Step 7: Cancel the Restore in HA UI

Now that you have the configuration file copied:

1. **Go back to Home Assistant UI restore window**
2. **Click "Cancel"** or just close the window
3. **Verify HA is still running normally**
   - Check http://192.168.2.10:8123
   - You should be able to access HA normally
   - No changes have been made yet

---

## Step 8: Review the Nov 14 Configuration

Now examine what you recovered:

```bash
# On your Mac:
cat ~/configuration-nov14-backup.yaml

# Or use an editor:
nano ~/configuration-nov14-backup.yaml
```

**Look for:**
- ✅ `homeassistant:` section at the top
- ✅ `automation: !include automations.yaml`
- ✅ `script: !include scripts.yaml`
- ✅ `mqtt:` section
- ✅ Other core integrations
- ✅ `sensor:` section (should have more than just time_date)
- ✅ `weather:` section

**Count the lines:**
```bash
wc -l ~/configuration-nov14-backup.yaml
```

Should be much more than 179 lines!

---

## Step 9: Compare with Current Configuration

Compare the recovered file with your current incomplete one:

```bash
# See the difference side-by-side
diff -u /Volumes/config/configuration.yaml ~/configuration-nov14-backup.yaml

# Or save the diff to a file for review:
diff -u /Volumes/config/configuration.yaml ~/configuration-nov14-backup.yaml > ~/configuration-diff.txt
cat ~/configuration-diff.txt
```

**You should see:**
- ✅ Lines with `-` = in current but not Nov 14 (these are your Gazebo additions - KEEP THESE)
- ✅ Lines with `+` = in Nov 14 but not current (these are MISSING - NEED TO ADD)

---

## Troubleshooting

### SSH "Permission denied"

**Issue:** `Permission denied (publickey,password)`

**Solution 1:** Use key-based auth
```bash
# Check if you have HA SSH keys set up
ls ~/.ssh/

# If not, try password auth:
ssh -o PasswordAuthentication=yes root@192.168.2.10
# When prompted, enter your HA password or admin password
```

**Solution 2:** Reset SSH in Home Assistant
1. Go to Settings → System
2. Look for SSH & Web Terminal add-on
3. Check if it's running
4. Restart it
5. Try SSH again

**Solution 3:** Use direct file access instead
If SSH still fails, you can try accessing through Home Assistant's direct file access or ask for the file via Home Assistant UI download feature.

---

### Backup Restore Won't Start

**Issue:** Restore button doesn't work or shows error

**Solution:**
1. Check Home Assistant is fully loaded
2. Try in a different browser
3. Check backup file isn't corrupted (try deleting backup cache in HA)
4. If persistent, restart Home Assistant first, then try restore again

---

### Can't Find Backup Files on HA System

**Issue:** `/backup` directory empty or no files found

**Solution:**
```bash
# Check if backups are stored elsewhere:
ssh root@192.168.2.10
find / -name "*2025-11-14*" -type f 2>/dev/null

# Or check HA config location:
ls -lh /homeassistant/backups/
```

---

### File Copy (SCP) Fails

**Issue:** `scp: command not found` or `Permission denied`

**Solution:**
If SCP doesn't work, try direct cat and pipe:

```bash
# On your Mac, create a script to extract from HA
ssh root@192.168.2.10 "cat /tmp/configuration-nov14.yaml" > ~/configuration-nov14-backup.yaml

# Verify:
ls -lh ~/configuration-nov14-backup.yaml
```

---

## What Happens Next

Once you have the Nov 14 configuration.yaml:

1. ✅ **Compare** - We'll compare it with your current config
2. ✅ **Merge** - We'll merge Nov 14 full config with your current Gazebo additions
3. ✅ **Validate** - We'll check the merged YAML is valid
4. ✅ **Test** - We'll test it before applying
5. ✅ **Apply** - We'll restore the merged configuration
6. ✅ **Verify** - We'll confirm everything works

Each step has safety checkpoints and rollback capability.

---

## Summary

| Step | Action | Time | Risk |
|------|--------|------|------|
| 1 | Test SSH access | 2 min | ✅ None |
| 2 | Find backup file | 2 min | ✅ None |
| 3 | Understand format | 2 min | ✅ None |
| 4 | Start restore | 2 min | ✅ Low (don't complete) |
| 5 | Copy configuration | 3 min | ✅ Low |
| 6 | Copy to Mac | 3 min | ✅ None |
| 7 | Cancel restore | 1 min | ✅ None |
| 8 | Review file | 2 min | ✅ None |
| 9 | Compare configs | 2 min | ✅ None |
| **Total** | | **~20 min** | ✅ **Very Low** |

---

## Ready?

When you've completed this extraction and have the `~/configuration-nov14-backup.yaml` file on your Mac, let me know:

1. The file exists: `ls -lh ~/configuration-nov14-backup.yaml`
2. How many lines it has: `wc -l ~/configuration-nov14-backup.yaml`
3. Share the first 50 lines so I can see the structure: `head -50 ~/configuration-nov14-backup.yaml`

Then we'll proceed to the merge phase with confidence that we have the complete original configuration! ✅

---

**Good luck! This is the key step to recovering your complete configuration.** 🚀
