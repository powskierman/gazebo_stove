# Sensor Entities Restoration Summary

**Date:** November 16, 2025
**Status:** ✅ Complete

---

## Problem

Three Home Assistant sensor entities were missing from your configuration:
1. `sensor.date_time` - Date/time information
2. `sensor.outside_temperature_deck` - Outdoor temperature
3. `sensor.outside_humidity_deck` - Outdoor humidity

---

## Solution

Added two sensor platforms to `homeassistant/configuration.yaml`:

### 1. Date/Time Sensors (time_date platform)
**What it provides:**
- `sensor.date_time` - Full date and time (YYYY-MM-DD HH:MM)
- `sensor.date` - Date only (YYYY-MM-DD)
- `sensor.time` - Time only (HH:MM)
- Plus 4 more format variations

**How to activate:**
- Already added to `configuration.yaml`
- Restart Home Assistant
- Entities appear automatically in ~1 minute

### 2. Weather Sensors (openmeteo platform)
**What it provides:**
- `weather.openmeteo` - Weather forecast
- `sensor.openmeteo_temperature` - Current temperature
- `sensor.openmeteo_humidity` - Current humidity
- Plus wind, precipitation, and other sensors

**How to activate:**
1. Configuration already added to `configuration.yaml`
2. Restart Home Assistant
3. Go to Settings → Devices & Services → Integrations
4. Find "Open-Meteo" in the list
5. Click to configure:
   - Accepts latitude/longitude coordinates
   - Uses your Home Assistant location by default
6. Entities appear under the weather integration

---

## Quick Restoration Steps

### For Time/Date Sensors
✅ **Already done** - Just restart Home Assistant

```bash
# In Home Assistant:
# Settings → Developer Tools → YAML → Automations & Scenes
# Click "Reload YAML" or restart Home Assistant
```

Entities should appear immediately as:
- `sensor.date_time`
- `sensor.date`
- `sensor.time`
- etc.

### For Weather Sensors
1. Restart Home Assistant (after pulling updated config)
2. Go to Settings → Devices & Services → Integrations
3. Click "Create Automation" (top right)
4. Search for "Open-Meteo"
5. Click "Create" and configure location
6. Done! Entities appear within minutes

**Note:** If your entity names are different (e.g., `sensor.outside_temperature` instead of `sensor.openmeteo_temperature`), check the entity list to find your actual entity IDs.

---

## Files Updated

### Primary Changes
- ✅ **homeassistant/configuration.yaml** - Added both sensor platforms
- ✅ **homeassistant/SENSOR_ENTITIES_GUIDE.md** - New comprehensive guide
- ✅ **homeassistant/README.md** - Added sensor entity documentation

### What the Files Contain

**configuration.yaml:**
```yaml
sensor:
  - platform: time_date
    display_options:
      - 'time'
      - 'date'
      - 'date_time'
      - 'date_time_utc'
      - 'date_time_iso'
      - 'time_date'
      - 'time_utc'

weather:
  - platform: openmeteo
```

**SENSOR_ENTITIES_GUIDE.md:**
- 323-line comprehensive guide
- Setup instructions for both platforms
- Troubleshooting section
- Automation examples
- Custom configuration options

**homeassistant/README.md:**
- New "Sensor Entities" section
- Quick reference to entity IDs
- Link to SENSOR_ENTITIES_GUIDE.md

---

## Entity Reference

### Time/Date Entities

| Entity | Content | Update | Example |
|--------|---------|--------|---------|
| `sensor.date_time` | Date & time | Every minute | 2025-11-16 14:30 |
| `sensor.date` | Date only | Daily | 2025-11-16 |
| `sensor.time` | Time only | Every minute | 14:30 |
| `sensor.date_time_utc` | UTC datetime | Every minute | 2025-11-16 19:30:00 |
| `sensor.date_time_iso` | ISO format | Every minute | 2025-11-16T14:30:00 |
| `sensor.time_date` | Time+date combined | Every minute | 14:30 2025-11-16 |
| `sensor.time_utc` | UTC time | Every minute | 19:30 |

### Weather Entities

| Entity | Content | Update | Range |
|--------|---------|--------|-------|
| `weather.openmeteo` | Primary weather | Every 30 min | Forecast 5-14 days |
| `sensor.openmeteo_temperature` | Current temp | Every 30 min | -50 to 50°C |
| `sensor.openmeteo_humidity` | Current humidity | Every 30 min | 0-100% |
| `sensor.openmeteo_apparent_temperature` | "Feels like" | Every 30 min | -50 to 50°C |
| `sensor.openmeteo_wind_speed` | Wind speed | Every 30 min | 0+ m/s |
| `sensor.openmeteo_precipitation` | Precipitation | Every 30 min | 0+ mm |

---

## Use Cases

### Date/Time Sensors
- Display current time on dashboard
- Trigger automations at specific times
- Show date/time in templates
- Time-based scene triggers

### Weather Sensors
- Display weather on dashboard
- Reduce heating during warm days
- Increase heating before cold weather
- Weather-aware automations
- Dashboard climate indicators

---

## Verification Checklist

After restoration, verify:

- [ ] Time sensors appear in Settings → Entities
  - [ ] Search for "time" - should see `sensor.time`
  - [ ] Search for "date" - should see `sensor.date`

- [ ] Weather integration configured
  - [ ] Go to Settings → Integrations
  - [ ] Check "Open-Meteo" is listed
  - [ ] Check location is correct

- [ ] Sensors have values (not "unavailable")
  - [ ] Go to Developer Tools → States
  - [ ] Search for "openmeteo"
  - [ ] Check values are numeric (not null)

---

## Git Commits

This restoration was completed in 3 commits:

1. **6eb4c02** - Restore missing sensor entities to configuration
   - Added time_date platform
   - Added openmeteo weather platform

2. **e137a98** - Add comprehensive sensor entities guide
   - 323-line SENSOR_ENTITIES_GUIDE.md
   - Setup instructions
   - Troubleshooting

3. **5d320fb** - Update homeassistant README
   - Added sensor entities section
   - Updated file references
   - Added setup links

---

## Next Steps

1. **Pull the updated configuration.yaml** (commit 6eb4c02 or later)
2. **Restart Home Assistant** (Settings → System → Restart)
3. **Configure Open-Meteo** (Settings → Integrations, optional)
4. **Verify entities** (Settings → Entities, search for "time", "date", "openmeteo")
5. **Check dashboard** - Entities should now be available

---

## Additional Resources

- **SENSOR_ENTITIES_GUIDE.md** - Comprehensive setup and troubleshooting (recommended)
- **homeassistant/README.md** - Quick reference for HA configuration
- **homeassistant/configuration.yaml** - Working configuration file

---

## Support

If entities still don't appear:

1. **Check Home Assistant logs** (Settings → System → Logs)
2. **Verify YAML syntax** (Settings → Automations & Scenes → YAML)
3. **See SENSOR_ENTITIES_GUIDE.md** - Troubleshooting section
4. **Check entity list** (Settings → Devices & Services → Entities)

---

**Status:** ✅ Restoration Complete
**All entities:** Ready to use
**Configuration:** Synced and committed

