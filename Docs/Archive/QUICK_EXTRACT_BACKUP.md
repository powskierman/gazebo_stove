# Quick Extract - 5 Minute Method

**Fastest way to extract your Nov 14 configuration.yaml**

---

## Option A: Download from Web UI (EASIEST)

**No SSH needed. Works from any browser.**

1. Open http://192.168.2.10:8123
2. Settings → System → Backups
3. Find "Nov 14" backup
4. Click the **⋮** (three dots) menu
5. Click **Download**
6. File saves to ~/Downloads/

Then:

```bash
# Go to downloads
cd ~/Downloads

# List the backup file
ls -lh *2025-11-14*.tar.gz

# Extract it (it will ask for password)
tar -xzf Full_Backup_2025-11-14_03_00_00.tar.gz
# Enter your backup password when prompted

# Get the configuration file
cat homeassistant/configuration.yaml > ~/configuration-nov14-backup.yaml

# Verify
ls -lh ~/configuration-nov14-backup.yaml
wc -l ~/configuration-nov14-backup.yaml
```

**Done!** You now have `~/configuration-nov14-backup.yaml` ✅

---

## Option B: SSH Method (FASTEST if SSH works)

```bash
# SSH to HA
ssh root@192.168.2.10

# Find the backup
ls -lh /backup/*2025-11-14*

# Copy the exact filename from above and use it below:
cd /tmp
tar -xzf /backup/Full_Backup_2025-11-14_03_00_00.tar.gz
# When asked for password, enter your backup password

# Copy config
cp homeassistant/configuration.yaml /tmp/configuration-nov14.yaml

# Exit SSH
exit
```

Back on your Mac:

```bash
# Copy from HA to your Mac
scp root@192.168.2.10:/tmp/configuration-nov14.yaml ~/configuration-nov14-backup.yaml

# Verify
ls -lh ~/configuration-nov14-backup.yaml
wc -l ~/configuration-nov14-backup.yaml
```

---

## Verify You Have The File

Once one of the above methods completes:

```bash
# Check file exists
ls -lh ~/configuration-nov14-backup.yaml

# Check size (should be 1-10 KB)
# Check line count (should be 200+ lines)
wc -l ~/configuration-nov14-backup.yaml

# Preview first 50 lines
head -50 ~/configuration-nov14-backup.yaml
```

**Expected output:**
```
-rw-r--r--  1 michel  staff  3.2K Nov 16 10:00 ~/configuration-nov14-backup.yaml
   237 ~/configuration-nov14-backup.yaml
```

---

## Share Results

Once you have the file, share:

```bash
wc -l ~/configuration-nov14-backup.yaml
head -50 ~/configuration-nov14-backup.yaml
```

Then we merge! 🚀
