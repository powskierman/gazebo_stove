# Backup Your Current Home Assistant Configuration

**Status:** Ready to execute on your HA instance
**Location:** SSH directly into your Home Assistant
**Time:** 5 minutes
**Commands:** Copy/paste ready

---

## Step 1: SSH Into Home Assistant

You mentioned you're in `core-ssh ~]$` prompt, which means you're already in the Home Assistant SSH add-on. Perfect!

From that prompt, you're already in the container. Just run:

```bash
cd /config
ls -lh
```

This should show you your configuration files.

---

## Step 2: List Current Configuration Files

Run this to see what you have:

```bash
ls -lh /config/*.yaml
```

You should see output like:
```
-rw-r--r-- configuration.yaml
-rw-r--r-- automations.yaml
-rw-r--r-- scripts.yaml
-rw-r--r-- groups.yaml
-rw-r--r-- scenes.yaml
-rw-r--r-- secrets.yaml
(and possibly others)
```

**What we're looking for:**
- ✅ `configuration.yaml` - Main config (MUST exist)
- ✅ `automations.yaml` - Your automations
- ✅ `scripts.yaml` - Your scripts
- ✅ `groups.yaml` - Your groups
- ✅ `scenes.yaml` - Your scenes
- ✅ `secrets.yaml` - Your passwords

**Tell me:** What files do you see? List them exactly.

---

## Step 3: Create Backup (Once You Confirm Files Exist)

Once we know what files exist, run this exact command:

```bash
cd /config
tar -czf config-backup-nov-16-WORKING.tar.gz \
  configuration.yaml \
  automations.yaml \
  scripts.yaml \
  groups.yaml \
  scenes.yaml \
  secrets.yaml \
  2>/dev/null

ls -lh config-backup-nov-16-WORKING.tar.gz
```

This will:
1. Create a compressed backup of all config files
2. Show you the file size (should be non-zero!)

**Expected output:**
```
-rw-r--r-- 1 root root 15K Nov 16 12:34 config-backup-nov-16-WORKING.tar.gz
```

(Should show a real file size like 15K, 25K, etc. - NOT 0!)

---

## Step 4: Verify Backup Is Valid

Run this to check the backup:

```bash
cd /config
tar -tzf config-backup-nov-16-WORKING.tar.gz
```

You should see a list of files:
```
configuration.yaml
automations.yaml
scripts.yaml
groups.yaml
scenes.yaml
secrets.yaml
```

If you see file names, backup is good! ✅

---

## Step 5: Copy to Safe Location (Optional)

If you want to also save to another location:

```bash
cp /config/config-backup-nov-16-WORKING.tar.gz /mnt/data/backups/
```

Or just keep it in `/config` - it's safe there.

---

## Next Steps

**Once you run Step 2:**

Reply with exactly what files you see, and I'll give you the exact commands to run for Steps 3-4.

This way we ensure:
1. ✅ Files actually exist
2. ✅ Backup includes the right files
3. ✅ Backup is not empty
4. ✅ No mistakes in the process

---

## Alternative: If Files Don't Exist

If some files don't exist (like automations.yaml), that's important info.

**Just tell me which files exist, which don't, and we'll adjust accordingly.**

---

## Quick Command Cheat Sheet

```bash
# What you should run right now:
cd /config
ls -lh *.yaml

# Copy exactly what you see
```

**Then share the output with me, and I'll give you the next exact commands to run.**

---

**This approach is foolproof because:**
- ✅ You run commands directly (no SSH auth issues)
- ✅ We verify files exist first
- ✅ We avoid creating empty backups
- ✅ We know exactly what we're backing up
- ✅ We can test the backup before proceeding

