# Decrypt and Extract Nov 14 Backup - Direct Method

**Status:** Ready to execute
**Time Required:** 10-15 minutes
**Risk Level:** ✅ ZERO - Read-only operation, no changes to HA
**Reversibility:** N/A - We're just reading a backup file

---

## Prerequisites

You need:
- [ ] The Nov 14 backup file (on your HA system or accessible)
- [ ] The backup password (you have this! ✅)
- [ ] SSH access to your HA at 192.168.2.10
- [ ] About 10 minutes

---

## Method 1: Using Home Assistant's Built-in Tools (RECOMMENDED)

Home Assistant has built-in Python tools to decrypt backups. We'll use these via SSH.

### Step 1: SSH Into Home Assistant

```bash
ssh root@192.168.2.10
```

You should see:
```
core-ssh ~]$
```

If SSH fails, see **Troubleshooting** section below.

### Step 2: Find Your Nov 14 Backup

```bash
ls -lh /backup/*2025-11-14*
```

You should see output like:
```
-rw-r--r-- 1 root root 1.2G Nov 14 03:00:00 Full_Backup_2025-11-14_03_00_00.tar.gz
```

**Note the exact filename** - copy it exactly for the next step.

### Step 3: Use Home Assistant's Backup Tool to Decrypt

Home Assistant CLI has a backup command. Try:

```bash
# List available backups (with details)
ha backups list

# Or manually extract using tar (Home Assistant backups are actually tar.gz)
cd /tmp
tar -xzf /backup/FILENAME-HERE.tar.gz  # Replace FILENAME-HERE with actual name
```

**If prompted for password**, enter your backup password.

### Step 4: Find and Copy configuration.yaml

Once extracted, find the configuration file:

```bash
# Look for extracted files
ls -la /tmp/
find /tmp -name "configuration.yaml" -type f 2>/dev/null

# Once found, copy it to a known location
cp /tmp/homeassistant/configuration.yaml /tmp/configuration-nov14.yaml

# Verify
ls -lh /tmp/configuration-nov14.yaml
wc -l /tmp/configuration-nov14.yaml
```

### Step 5: Copy to Your Mac

Exit SSH first:
```bash
exit
```

Then on your Mac terminal:

```bash
scp root@192.168.2.10:/tmp/configuration-nov14.yaml ~/configuration-nov14-backup.yaml

# Verify the file
ls -lh ~/configuration-nov14-backup.yaml
wc -l ~/configuration-nov14-backup.yaml
head -50 ~/configuration-nov14-backup.yaml
```

---

## Method 2: Using Python decryption (Advanced)

If the simple tar extraction doesn't work, Home Assistant backups use this Python library:

```bash
ssh root@192.168.2.10

# SSH into the home assistant container where Python tools are available
cd /backup

# Try using Python to decrypt:
python3 << 'PYTHON_EOF'
import tarfile
import sys

backup_file = "/backup/Full_Backup_2025-11-14_03_00_00.tar.gz"  # Replace with actual name
password = "YOUR_PASSWORD_HERE"  # Replace with your backup password

try:
    with tarfile.open(backup_file, mode="r|gz") as tar:
        tar.extractall(path="/tmp/backup-extract", pwd=password.encode())
    print("✅ Backup decrypted successfully!")
except Exception as e:
    print(f"❌ Error: {e}")
    sys.exit(1)
PYTHON_EOF

# Then copy the configuration:
cp /tmp/backup-extract/homeassistant/configuration.yaml /tmp/configuration-nov14.yaml
```

---

## Method 3: Download from Home Assistant UI (Easiest if SSH Fails)

If SSH isn't working well, you can download the backup from Home Assistant UI directly:

### Step 3A: Use Home Assistant's Web UI

1. Open http://192.168.2.10:8123 in your browser
2. Go to **Settings** → **System** → **Backups**
3. Find the **Nov 14 backup**
4. Click the three dots menu (⋮)
5. Select **"Download"** to download the .tar.gz file to your Mac

Once downloaded (should go to ~/Downloads/):

```bash
cd ~/Downloads
ls -lh *2025-11-14*.tar.gz

# Extract locally on your Mac
tar -xzf Full_Backup_2025-11-14_03_00_00.tar.gz  # Replace with actual name
# When prompted for password, enter your backup password

# Find configuration.yaml
find . -name "configuration.yaml" -type f
cat ./homeassistant/configuration.yaml > ~/configuration-nov14-backup.yaml

# Verify
wc -l ~/configuration-nov14-backup.yaml
```

---

## Troubleshooting

### SSH "Permission denied"

```bash
# Try with password prompt:
ssh -o PasswordAuthentication=yes root@192.168.2.10

# Or use alternate port if configured:
ssh -p 22 root@192.168.2.10
```

### Backup File Not Found

```bash
# Search for all tar.gz files on the system:
find /backup -name "*.tar.gz" -type f

# Or check the backup location:
ls -lh /homeassistant/backups/
ls -lh /config/backups/
```

### Tar Extraction Says "No such file or directory"

The backup might be in a different location:

```bash
# Find it:
find / -name "*2025-11-14*" -type f 2>/dev/null | grep -i backup

# Use the full path found above in your tar command
```

### Password Error When Extracting

Make sure you have the exact password. If special characters, try quoting:

```bash
# If password has special chars, use single quotes:
tar -xzf /backup/file.tar.gz --password 'your_password_here'

# Or use a variable:
read -sp "Enter backup password: " PASS
tar -xzf /backup/file.tar.gz --password "$PASS"
```

---

## Quick Reference - All Commands in Order

```bash
# 1. SSH to HA
ssh root@192.168.2.10

# 2. Find backup
ls -lh /backup/*2025-11-14*

# 3. Extract (replace FILENAME with actual name)
cd /tmp
tar -xzf /backup/Full_Backup_2025-11-14_03_00_00.tar.gz

# 4. Find config in extracted files
find /tmp -name "configuration.yaml" 2>/dev/null

# 5. Copy to temp location
cp /tmp/homeassistant/configuration.yaml /tmp/configuration-nov14.yaml

# 6. Verify
ls -lh /tmp/configuration-nov14.yaml
wc -l /tmp/configuration-nov14.yaml

# 7. Exit SSH
exit

# 8. Copy to Mac (run on Mac terminal, NOT in SSH)
scp root@192.168.2.10:/tmp/configuration-nov14.yaml ~/configuration-nov14-backup.yaml

# 9. Verify on Mac
ls -lh ~/configuration-nov14-backup.yaml
wc -l ~/configuration-nov14-backup.yaml
head -50 ~/configuration-nov14-backup.yaml
```

---

## What To Do If SSH Fails Completely

If SSH is completely broken, use **Method 3** above:

1. Open http://192.168.2.10:8123
2. Settings → System → Backups
3. Download the Nov 14 backup directly
4. Extract locally on your Mac with the password
5. Copy the configuration.yaml from the extracted files

This requires no SSH at all - just the Home Assistant web interface.

---

## Next Steps

Once you have the file on your Mac at `~/configuration-nov14-backup.yaml`:

1. Share the output of:
   ```bash
   ls -lh ~/configuration-nov14-backup.yaml
   wc -l ~/configuration-nov14-backup.yaml
   head -50 ~/configuration-nov14-backup.yaml
   ```

2. Then we'll compare with current config and merge

---

## Ready?

Choose one method above and follow it. **Method 1** (SSH + tar) is fastest if SSH works. **Method 3** (download from UI) is most reliable if SSH has issues.

Let me know which method you're trying, and if you hit any errors, share the exact error message! 🚀
