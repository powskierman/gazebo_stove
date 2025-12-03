# Home Assistant Sensor Entities Guide

**Last Updated:** November 16, 2025
**Status:** Complete guide for restoring and configuring missing sensors

---

## Overview

Your Home Assistant configuration includes several sensor entities that provide time, date, and weather information. This guide explains what they are and how to set them up if they're missing.

---

## 1. Date/Time Sensors

### Platform: `time_date`

The `time_date` platform creates time and date-related sensor entities. Add this to your `configuration.yaml`:

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
```

### Entities Created

| Entity ID | Content | Format | Example |
|-----------|---------|--------|---------|
| `sensor.time` | Current time | HH:MM | 14:30 |
| `sensor.date` | Current date | YYYY-MM-DD | 2025-11-16 |
| `sensor.date_time` | Date and time | YYYY-MM-DD HH:MM | 2025-11-16 14:30 |
| `sensor.date_time_utc` | UTC datetime | ISO format with UTC | 2025-11-16 19:30:00 |
| `sensor.date_time_iso` | ISO format datetime | ISO 8601 | 2025-11-16T14:30:00 |
| `sensor.time_date` | Time then date | HH:MM YYYY-MM-DD | 14:30 2025-11-16 |
| `sensor.time_utc` | UTC time only | HH:MM | 19:30 |

### Key Characteristics
- **Updates:** Every minute
- **No API key required**
- **Requires:** Home Assistant core (built-in)
- **Latency:** <1 second (local)

### Use Cases
- Display current date/time on dashboard
- Automation triggers based on time
- Condition checking in automations
- Dashboard clocks and date widgets

---

## 2. Weather Sensors

### Platform: `openmeteo`

The `openmeteo` platform provides free weather data without requiring an API key. It uses the Open-Meteo free weather API.

Add this to your `configuration.yaml`:

```yaml
weather:
  - platform: openmeteo
```

### Initial Setup

After adding the configuration, you need to configure the location:

1. **Restart Home Assistant** (or reload YAML configurations)
2. **Go to:** Settings → Devices & Services → Integrations
3. **Search for:** "Open-Meteo" (or "openmeteo")
4. **Click:** "Create Automation" or configure the integration
5. **Set your location** using:
   - Latitude/Longitude coordinates, OR
   - Home Assistant's default location (from General Settings)
6. **Click:** "Create" or "Finish"

### Entities Created

#### Primary Weather Entity
- **Entity ID:** `weather.openmeteo`
- **Type:** Weather integration
- **Contains:** Current conditions, forecast data
- **Updates:** Every 30 minutes

#### Derived Sensor Entities

| Entity ID | Description | Unit | Range |
|-----------|-------------|------|-------|
| `sensor.openmeteo_temperature` | Current outdoor temperature | °C | -50 to 50 |
| `sensor.openmeteo_humidity` | Current outdoor humidity | % | 0-100 |
| `sensor.openmeteo_apparent_temperature` | "Feels like" temperature | °C | -50 to 50 |
| `sensor.openmeteo_precipitation` | Current precipitation | mm | 0+ |
| `sensor.openmeteo_wind_speed` | Wind speed | m/s | 0+ |
| `sensor.openmeteo_wind_direction` | Wind direction | ° | 0-360 |
| `sensor.openmeteo_condition` | Weather condition | text | "sunny", "rainy", etc. |

### Custom Entity Names

The entity IDs shown above are for the standard `openmeteo` configuration. If you had a custom name like `weather.deck` or `weather.gazebo`, the sensor entities might be named differently:

- `sensor.deck_temperature` (instead of `sensor.openmeteo_temperature`)
- `sensor.deck_humidity` (instead of `sensor.openmeteo_humidity`)

If you previously had entities named `sensor.outside_temperature_deck` and `sensor.outside_humidity_deck`, you may need to check if your custom integration or template sensors are creating those names.

### Checking Your Entity Names

To find the actual entity IDs in your Home Assistant:

1. **Go to:** Settings → Devices & Services → Entities
2. **Search for:** "openmeteo", "temperature", or "humidity"
3. **Note the entity IDs** shown in the list

### Key Characteristics
- **API:** Open-Meteo (free, no registration)
- **Updates:** Every 30 minutes
- **Requires:** Internet connection
- **Accuracy:** Moderate (grid-based weather data)
- **Forecast:** Included (5-14 days depending on type)
- **Cost:** Free

### Use Cases
- Display outdoor weather on dashboard
- Automation: Turn off heating during warm weather
- Automation: Adjust setpoint based on forecast
- Dashboard widgets showing outdoor conditions
- Weather-based scene triggers

---

## 3. Restoring Your Missing Sensors

### If You Previously Had These Entities

The `configuration.yaml` file has been updated with both sensor platforms. To restore your missing entities:

1. **Edit your Home Assistant `configuration.yaml`**
2. **Add the sensor section** (if not already present):
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
   ```

3. **Add the weather section** (if not already present):
   ```yaml
   weather:
     - platform: openmeteo
   ```

4. **Restart Home Assistant** or reload YAML
5. **Wait 1-2 minutes** for entities to be created
6. **Check:** Settings → Devices & Services → Entities for new sensors

### For Time/Date Entities
- Entities should appear immediately as `sensor.time`, `sensor.date`, etc.
- Check Settings → Entities → search for "time_date" if not visible

### For Weather Entities
- First reload or restart Home Assistant
- Then configure the Open-Meteo integration via UI
- Entities should appear as `weather.openmeteo` and related sensors
- If still missing, check Settings → Integrations for errors

---

## 4. Using These Sensors in Automations

### Example: Check if it's bedtime using time sensor
```yaml
automation:
  - alias: "Bedtime Climate Control"
    trigger:
      platform: state
      entity_id: sensor.time
      to: "22:00"  # 10 PM
    action:
      service: climate.set_temperature
      target:
        entity_id: climate.propane_stove_thermostat
      data:
        temperature: 18  # Lower temp at night
```

### Example: Reduce heating on warm days
```yaml
automation:
  - alias: "Reduce Heating on Warm Days"
    trigger:
      platform: numeric_state
      entity_id: sensor.openmeteo_temperature
      above: 20  # Above 20°C
    action:
      service: select.select_option
      target:
        entity_id: select.presence_mode
      data:
        option: "away"  # Disable heating
```

### Example: Display current time on dashboard
```yaml
template:
  - sensor:
      - name: "Current Time Display"
        unique_id: current_time_display
        state: "{{ states('sensor.time') }}"
```

---

## 5. Troubleshooting

### Entities Not Appearing

**Problem:** Sensors don't show up after restart

**Solution:**
1. Check Home Assistant logs (Settings → System → Logs)
2. Verify YAML syntax is correct (Settings → Automations & Scenes → YAML)
3. For `time_date`: Just needs Home Assistant core
4. For `openmeteo`: May need to configure via UI first

### Time/Date Shows Wrong Value

**Problem:** `sensor.time` shows wrong time

**Solution:**
1. Check Home Assistant's time zone (Settings → System → General)
2. Verify NTP is synced (should be automatic)
3. Check if local machine time is correct
4. Time sensors use Home Assistant's system clock

### Weather Sensor Shows "Unavailable"

**Problem:** `weather.openmeteo` shows unavailable

**Solution:**
1. Verify internet connection
2. Check Open-Meteo integration is configured
3. Verify location (latitude/longitude) is set correctly
4. Check Home Assistant logs for error messages
5. Wait 5 minutes and check again (API timeout)

### Custom Entity Names Not Working

**Problem:** Entity IDs don't match expected names

**Solution:**
1. Check actual entity IDs in Settings → Devices & Services → Entities
2. Search for the entity type (e.g., "temperature", "humidity")
3. Use actual entity IDs in automations and templates
4. Update any automations using old entity IDs

---

## 6. Advanced Configuration

### Custom Temperature Offsets

If your weather sensor reads 2°C higher than actual:

```yaml
template:
  - sensor:
      - name: "Corrected Outside Temperature"
        unique_id: corrected_outside_temp
        unit_of_measurement: "°C"
        state: "{{ (states('sensor.openmeteo_temperature') | float(0) - 2) | round(1) }}"
```

### Multiple Weather Locations

```yaml
weather:
  - platform: openmeteo
    name: "Home Weather"
    latitude: 45.123
    longitude: -73.456

  - platform: openmeteo
    name: "Gazebo Weather"
    latitude: 45.124
    longitude: -73.457
```

---

## 7. Configuration File Reference

See `homeassistant/configuration.yaml` for the complete working configuration with all sensors included.

---

## Summary

| Feature | Platform | Setup | Effort |
|---------|----------|-------|--------|
| **Date/Time** | `time_date` | Add 10 lines to YAML | 2 min |
| **Weather** | `openmeteo` | Add 2 lines + UI config | 5 min |
| **Total Restoration** | Both | Both | 7 min |

Both are included in the updated `configuration.yaml` - just copy the sensor and weather sections to your Home Assistant config!

---

**Questions?** Refer to the comments in `homeassistant/configuration.yaml` or check the Home Assistant documentation for these platforms.

