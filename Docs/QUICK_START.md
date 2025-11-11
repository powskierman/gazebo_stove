# GazeboWx Quick Start - Phase 1 Setup

**Time Required**: 30-60 minutes
**Difficulty**: Beginner-friendly

---

## 5-Minute Setup Summary

### 1. Install ESPHome
```bash
pip install esphome
```

### 2. Configure WiFi
Edit `secrets.yaml`:
```yaml
wifi_ssid: "Your_WiFi_Network"
wifi_password: "Your_WiFi_Password"
ota_password: "SecurePassword"
```

### 3. Flash Device
```bash
esphome upload gazebo_stove.yaml
```
(Select USB port when prompted)

### 4. Watch Logs
```bash
esphome logs gazebo_stove.yaml
```

### 5. Check Home Assistant
Device should appear automatically. Control relay switch, view temperature.

---

## What You Need

**Hardware**:
- [ ] esp32doit-devkit-v1 OR Custom ESP32 WROOM board
- [ ] USB cable (USB-C for dev board)
- [ ] DS18B20 temperature sensor + 4.7kΩ resistor
- [ ] Relay module (5V or 12V)
- [ ] Nextion NX4832-F035 display
- [ ] Jumper wires, soldering iron, solder

**Software**:
- [ ] Python 3.7+
- [ ] ESPHome installed
- [ ] Home Assistant (optional but recommended)
- [ ] WiFi network available

---

## Wiring Diagram

```
ESP32 GPIO12    ─────→  Relay Input (heating control)
ESP32 GPIO4     ─────→  DS18B20 Data (with 4.7kΩ pullup)
ESP32 GPIO16    ←─────  Nextion RX
ESP32 GPIO17    ─────→  Nextion TX
ESP32 3.3V      ─────→  DS18B20 Power
ESP32 GND       ─────→  All grounds together

Relay Power:    5V external supply
Nextion Power:  5V external supply
```

---

## Step-by-Step

### Step 1: Connect Hardware (10 min)

1. Solder DS18B20 sensor:
   - Red (power) → 3.3V
   - Black (GND) → GND
   - Yellow (data) → GPIO4 (with 4.7kΩ to 3.3V)

2. Connect relay:
   - GPIO12 → Relay input
   - 5V supply → Relay power

3. Connect Nextion:
   - GPIO16 (RX) ← Nextion RX
   - GPIO17 (TX) → Nextion TX
   - 5V supply → Nextion power
   - GND → GND

4. (Optional) Status LED:
   - GPIO2 → LED positive
   - GND → LED negative (through resistor)

### Step 2: Software Setup (5 min)

1. Open terminal
2. Install ESPHome:
   ```bash
   pip install esphome
   ```

3. Create project directory:
   ```bash
   cd /Users/michel/Documents/Electronics/Projects/GazeboStove
   ```

4. Edit `secrets.yaml`:
   ```yaml
   wifi_ssid: "Your_SSID"
   wifi_password: "Your_Password"
   ota_password: "RandomPassword123"
   ```

### Step 3: Find Sensor Address (5 min)

1. Plug in device via USB
2. Run:
   ```bash
   esphome upload gazebo_stove.yaml
   ```

3. Select port (usually `/dev/cu.usbserial-*`)

4. Monitor logs:
   ```bash
   esphome logs gazebo_stove.yaml
   ```

5. Look for:
   ```
   Found device with address: 0x00000000000001D6
   ```

6. Update `gazebo_stove.yaml`:
   ```yaml
   address: 0x00000000000001D6  # Use YOUR address
   ```

### Step 4: Final Flash (5 min)

1. Compile:
   ```bash
   esphome compile gazebo_stove.yaml
   ```

2. Upload:
   ```bash
   esphome upload gazebo_stove.yaml
   ```

3. Monitor:
   ```bash
   esphome logs gazebo_stove.yaml
   ```

### Step 5: Test in Home Assistant (5 min)

1. Open Home Assistant
2. Look for "gazebo-wx" in Settings → Devices
3. Click to see entities
4. Toggle relay switch (should click)
5. View temperature sensor (should show reading)

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "Port not found" | Check USB cable, try different port |
| Temperature shows "NaN" | Verify GPIO4, sensor power, 4.7kΩ resistor |
| Relay doesn't click | Check GPIO12, relay power, relay wiring |
| "Cannot connect to WiFi" | Check SSID/password in secrets.yaml |
| Device not in Home Assistant | Restart HA, check mDNS (port 5353 firewall) |

---

## Success Indicators

✅ Device boots in 30 seconds
✅ WiFi connects in 10 seconds
✅ Temperature reads every 10 seconds
✅ Relay toggles on button click
✅ All entities visible in Home Assistant
✅ Logs show no critical errors

---

## What's Next?

Once Phase 1 is working:

1. Run temperature test:
   ```bash
   esphome run NEXTION_TEST.yaml
   ```
   (Verify display cycles 0→50°C)

2. Read Phase 2 plan:
   ```
   PLAN.md (Phase 2: Thermostat Control)
   ```

3. Implement thermostat logic

---

## Files to Know

| File | Purpose |
|------|---------|
| `gazebo_stove.yaml` | Main configuration |
| `secrets.yaml` | WiFi & credentials |
| `PHASE_1_IMPLEMENTATION.md` | Detailed guide |
| `NEXTION_TEST.yaml` | Display test |
| `CLARIFICATIONS_RESOLVED.md` | Hardware specs |

---

## Common Commands

```bash
# Compile only
esphome compile gazebo_stove.yaml

# Upload (first time via USB)
esphome upload gazebo_stove.yaml

# Update (OTA, after WiFi connected)
esphome upload gazebo_stove.yaml

# Monitor logs
esphome logs gazebo_stove.yaml

# Clean build
esphome clean gazebo_stove.yaml && esphome compile gazebo_stove.yaml
```

---

## Safety Checklist

- [ ] Relay defaults to OFF (won't heat on startup)
- [ ] All grounds connected together
- [ ] Power supplies separated (relay ≠ Nextion ≠ microcontroller)
- [ ] No loose wires or solder bridges
- [ ] Device set to auto-update WiFi fallback
- [ ] Sensor malfunction alarm enabled

---

## Support

**Getting Help**:
1. Check logs: `esphome logs gazebo_stove.yaml`
2. Search issues: ESPHome docs or GitHub
3. Reference: `PHASE_1_IMPLEMENTATION.md` troubleshooting section
4. Advanced: `CLARIFICATIONS_RESOLVED.md` for hardware specs

---

**Estimated time to working device: 45 minutes**

Good luck! 🚀
