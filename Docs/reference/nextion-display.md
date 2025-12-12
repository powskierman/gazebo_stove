# GazeboWx Nextion Display Integration Guide

**Date**: 2025-11-10
**Display Hardware**: Nextion HSD035383E5 (3.5" resistive touchscreen)
**Communication**: UART (GPIO16 RX, GPIO17 TX) @ 9600 baud
**Integration Phase**: Phase 6 (Weeks 11-12)

---

## Overview

The Nextion display is a critical user interface component showing real-time weather, temperature control, forecasts, and heating status. This document provides complete component mappings, UART communication protocols, and ESPHome integration instructions.

**Key Capabilities**:
- 4 pages (0-3) with distinct functionality
- Touch-based navigation and control
- Real-time sensor data display
- Weather forecast integration (OpenMeteo API)
- Interactive temperature setpoint slider
- Dynamic icon rendering based on weather conditions

---

## Display Hardware Specifications

| Specification | Value |
|---------------|-------|
| **Model** | Nextion HSD035383E5 |
| **Screen Size** | 3.5 inches (diagonal) |
| **Resolution** | 320 × 480 pixels (QVGA) |
| **Display Type** | Resistive touchscreen |
| **Communication** | UART (TTL serial) |
| **Baud Rate** | 9600 bps (8 data bits, 1 stop bit, no parity) |
| **GPIO Pins** | GPIO16 (RX), GPIO17 (TX) |
| **Power Supply** | 5V (via Nextion connector) |
| **Firmware Version** | (To be confirmed during setup) |

---

## Page Architecture

**Status**: 
- ✅ **Page 0 (Extérieur)**: OPERATIONAL - Temperature display working, ready for testing
- ❌ **Page 1 (Ressentie)**: NON-OPERATIONAL - Weather functionality not implemented yet
- ❌ **Page 2 (Prévisions)**: NON-OPERATIONAL - Weather functionality not implemented yet  
- ✅ **Page 3 (Slider Control)**: OPERATIONAL - Temperature control working, ready for testing

---

### Page 0: Extérieur (Exterior/Current Conditions)

**Status**: ✅ **OPERATIONAL** - Ready for testing

**Purpose**: Main dashboard showing real-time weather and heating status

**Actual Layout** (from Nextion display):
```
┌─────────────────────────────────────┐
│         Extérieur                   │
│         (now0)                      │
│                                     │
│         [Weather Icon]              │
│         (cloud/storm graphic)       │
│                                     │
│         temp: 88 °C                 │
│         (large numbers)             │
│                                     │
│         precip: 888 %              │
│         (large numbers)            │
│                                     │
│         [b1: Status Button]        │
│         [Stove Graphic]            │
│         (black stove with flames)   │
└─────────────────────────────────────┘
```

**Visual Elements**:
- **Title**: "Extérieur" centered at top
- **Weather Icon**: Cloud/storm graphic below title
- **Temperature**: Large "88" followed by "°C" (placeholder value)
- **Precipitation**: Large "888" followed by "%" (placeholder value)
- **Stove Graphic**: Black stove with orange/yellow flames visible through glass door (indicates heating active)
- **Status Button**: Small button labeled "b1" (debug label visible)

**Component Mapping**:

| Component ID | Variable Name | Type | Data Format | Source | Update Frequency | Status |
|---|---|---|---|---|---|---|
| Page Title | (static) | Text | "Extérieur" | Fixed | - | ✅ Working |
| Weather Icon | (icon element) | Image | WMO/OpenMeteo code → Icon ID | OpenMeteo API | 30 min | ⏸️ Pending US5 |
| Temperature | `temp` | Number | Integer °C | DS18B20 sensor | 2 sec | ✅ **READY FOR TEST** |
| Precipitation % | `precip` | Number | Integer % | OpenMeteo API | 30 min | ⏸️ Pending US5 |
| Now Indicator | `now0` | Text | "now" label | System time | 1 sec | ✅ Working |
| Status Button | `b1` | Button | Status indicator | Relay/Thermostat state | Real-time | ✅ Working |
| Stove Graphic | (image element) | Image | Active flames or idle | Relay state (GPIO12) | Real-time | ✅ Working |

**UART Commands for Page 0 Updates**:

```
# Set current temperature (DS18B20 reading)
temp.val=22

# Set precipitation probability or humidity
precip.val=75

# Update weather icon (based on WMO code)
# Icon IDs: 0=sunny, 1=cloudy, 2=rain, 3=thunderstorm, etc.
(icon_element).pic=2

# Status indicator
b1.txt="Heating"
b1.bco=0xFF0000  # Red for heating active

# Alternative: Button text and color based on state
b1.txt="Idle"
b1.bco=0x0000FF  # Blue for idle
```

**Update Priority**: HIGH
- Temperature: Critical, real-time
- Weather: Important, periodic (30 min)
- Status: Critical, real-time

---

### Page 1: Ressentie (Hourly Forecast - "Feels Like")

**Status**: ❌ **NON-OPERATIONAL** - Weather functionality not implemented yet

**Purpose**: 7-hour hourly weather forecast with temperatures and precipitation

**Actual Layout** (from Nextion display):
```
┌─────────────────────────────────────┐
│  ← Ressentie →                     │
│  (b0)        (b1)                  │
│                                     │
│  dt0:00  [⚡]  f01°C  rain0%       │  Row 0
│  dt1:00  [⚡]  f12°C  rain1%       │  Row 1
│  dt2:00  [⚡]  f23°C  rain2%       │  Row 2
│  dt3:00  [⚡]  f34°C  rain3%       │  Row 3
│  dt4:00  [⚡]  f45°C  rain4%       │  Row 4
│  dt5:00  [⚡]  f56°C  rain5%       │  Row 5
│  dt6:00  [⚡]  f67°C  rain6%       │  Row 6
└─────────────────────────────────────┘
```

**Visual Elements**:
- **Title**: "Ressentie" centered at top
- **Navigation**: Left arrow (b0) and right arrow (b1) buttons
- **7 Rows**: Each row shows:
  - Time: `dt0` through `dt6` (format: "HH:00")
  - Weather Icon: Yellow lightning bolt icon (placeholder)
  - Temperature: `f01` through `f67` (placeholder values) followed by "°C"
  - Precipitation: `rain0` through `rain6` (placeholder values) followed by "%"
- **Placeholder Text**: "wxlco" visible in some rows (weather condition placeholder)

**Note**: This page requires OpenMeteo API integration (User Story 5) which is not yet implemented.

---

### Page 2: Prévisions (Daily Forecast - "Forecasts")

**Status**: ❌ **NON-OPERATIONAL** - Weather functionality not implemented yet

**Purpose**: 7-day daily weather forecast with high/low temps and precipitation probability

**Actual Layout** (from Nextion display):
```
┌─────────────────────────────────────┐
│  ← Prévisions →                     │
│  (b0)        (b1)                  │
│                                     │
│  dt10  [⚡]  tem0°C  prec0%        │  Day 0
│  dt11  [⚡]  tem1°C  prec1%        │  Day 1
│  dt12  [⚡]  tem2°C  prec2%        │  Day 2
│  dt13  [⚡]  tem3°C  prec3%        │  Day 3
│  dt14  [⚡]  tem4°C  prec4%        │  Day 4
│  dt15  [⚡]  tem5°C  prec5%        │  Day 5
│  dt16  [⚡]  tem6°C  prec6%        │  Day 6
└─────────────────────────────────────┘
```

**Visual Elements**:
- **Title**: "Prévisions" centered at top
- **Navigation**: Left arrow (b0) and right arrow (b1) buttons
- **7 Rows**: Each row shows:
  - Date: `dt10` through `dt16` (day identifier)
  - Weather Icon: Yellow lightning bolt icon (placeholder)
  - Temperature: `tem0` through `tem6` (placeholder values) followed by "°C"
  - Precipitation: `prec0` through `prec6` (placeholder values) followed by "%"
- **Placeholder Text**: "wxlco", "tem", "prec" visible (placeholders for weather condition, temperature, precipitation)

**Note**: This page requires OpenMeteo API integration (User Story 5) which is not yet implemented.

---

### Page 3: Slider Control (Temperature Setpoint)

**Status**: ✅ **OPERATIONAL** - Ready for testing

**Purpose**: Interactive temperature setpoint adjustment with visual slider

**Actual Layout** (from Nextion display):
```
┌─────────────────────────────────────┐
│                                     │
│      Slider0                        │
│      (vertical gradient slider)      │
│      ├─ Orange (top)                │
│      ├─ Pink (middle)               │
│      └─ Purple (bottom)             │
│      [Blue segmented indicator]     │
│                                     │
│  n0: 99°C    [☁️]    n1: 88°C      │
│  (left)      (icon)   (right)      │
│                                     │
│         [b2: Button]                │
│         (action button)            │
└─────────────────────────────────────┘
```

**Visual Elements**:
- **Slider**: Large vertical gradient slider (`Slider0`) with:
  - Orange at top (hot)
  - Pink in middle
  - Purple at bottom (cold)
  - Blue segmented horizontal indicator showing current position
- **Current Temperature**: `n0` showing "99°C" (left side, large numbers)
- **Weather Icon**: Cloud icon in center
- **Desired Temperature**: `n1` showing "88°C" (right side, large numbers)
- **Action Button**: `b2` button at bottom

**Component Mapping**:

| Component ID | Variable Name | Type | Data Format | Purpose | Interaction | Status |
|---|---|---|---|---|---|---|
| Gradient Slider | `Slider0` | Slider | Value range: 0-100 | Visual temp control (maps to 5-35°C) | User touch input | ✅ **READY FOR TEST** |
| Current Temp | `n0` | Number | Integer °C | Actual temperature (left side) | Read-only | ✅ **READY FOR TEST** |
| Weather Icon | (icon element) | Image | WMO code → Icon ID | Current condition | Display only | ⏸️ Pending US5 |
| Desired Temp | `n1` | Number | Integer °C | Setpoint (right side) | Updates from Slider0 | ✅ **READY FOR TEST** |
| Action Button | `b2` | Button | Button state | Confirm/Apply action | User touch | ✅ Working |
| Navigation Button | (nav arrows) | Buttons | Page selection | Switch pages | User touch | ✅ Working |

**UART Commands for Page 3 Updates**:

```
# Update current temperature (left side)
n0.val=22

# Update desired/setpoint temperature (right side)
n1.val=24

# Update weather icon
(icon_element).pic=2

# Slider position (maps 5-35°C to 0-100 scale)
# Formula: slider_value = ((desired_temp - 5) / 30) * 100
# Example: 22°C → ((22-5)/30)*100 = 56.67 ≈ 57
Slider0.val=57

# Slider gradient can be pre-configured in Nextion firmware
# (color gradient from orange → pink → purple)
```

**Touch Event Handling**:

```
# When user adjusts Slider0:
# Nextion sends: touch event for Slider0
# ESPHome receives: new slider value
# ESPHome calculates: desired_temp = 5 + (slider_val / 100) * 30
# ESPHome updates: Home Assistant input_number.desired_temperature
# Nextion feedback: n1.val updates immediately

# Example touch handler in ESPHome:
on_touch:
  if: id(page_index) == 3  # Page 3 only
  then:
    - lambda: |-
        // Slider0 touched, calculate new setpoint
        float slider_val = id(gazebo_nextion).get_component_value("Slider0");
        float desired_temp = 5.0 + (slider_val / 100.0) * 30.0;
        id(desired_temperature).publish_state(desired_temp);
```

**Update Priority**: HIGH
- Temperature display: Real-time (user feedback)
- Slider position: Updates when setpoint changes remotely
- Weather: Every 30 minutes

---


**UART Commands for Page 2 Updates** (Example for Row 0):

```
# Time for hour 0
dt0.txt="08:00"

# Weather icon (WMO code conversion)
# 0=sunny, 1=cloudy, 2=overcast, 45/48=foggy, 51-55=drizzle
# 61-67=rain, 71-77=snow, 80-82=rain showers, 85-86=snow showers
# 80-82=rain showers, 85-86=snow showers, 95-99=thunderstorm
wxIcon0.pic=2  # Rain icon

# Temperature
tt0.val=12

# Precipitation probability
rain0.val=85

# Repeat for rows 1-6: dt1-dt6, wxIcon1-wxIcon6, tt1-tt6, rain1-rain6
```

**Data Update Strategy**:

```yaml
# Fetch hourly forecast every 30 minutes
sensor:
  - platform: http_request
    id: openmeteo_hourly
    resource: https://api.open-meteo.com/v1/forecast?latitude=40.12&longitude=-74.21&hourly=temperature_2m,precipitation_probability,weather_code&timezone=America/New_York
    method: GET
    update_interval: 30min
    on_response:
      then:
        - lambda: |-
            // Parse JSON response
            // Extract next 7 hours of data
            // Update Nextion components dt0-dt6, tt0-tt6, rain0-rain6
            for (int i = 0; i < 7; i++) {
              id(gazebo_nextion).send_command_printf("dt%d.txt=\"%02d:00\"", i, (hour + i) % 24);
              id(gazebo_nextion).send_command_printf("tt%d.val=%d", i, hourly_temps[i]);
              id(gazebo_nextion).send_command_printf("rain%d.val=%d", i, precip_probs[i]);
              int wmo_code = hourly_weather_codes[i];
              int icon_id = wmo_to_icon(wmo_code);
              id(gazebo_nextion).send_command_printf("wxIcon%d.pic=%d", i, icon_id);
            }
```

**Update Priority**: MEDIUM
- Hourly forecast: Every 30 minutes
- Icons: With forecast updates
- Temperatures: With forecast updates

---


**UART Commands for Page 3 Updates** (Example for Day 0):

```
# Date (day of week or full date)
dt10.txt="Mon"

# Weather icon (same WMO code conversion as Page 2)
wxIcon0.pic=3  # Thunderstorm icon

# Temperature (high or average)
tem0.val=22

# Precipitation probability (max daily probability)
prec0.val=65

# Repeat for days 1-6: dt11-dt16, tem1-tem6, prec1-prec6
```

**Data Update Strategy**:

```yaml
# Fetch daily forecast every 3 hours
sensor:
  - platform: http_request
    id: openmeteo_daily
    resource: https://api.open-meteo.com/v1/forecast?latitude=40.12&longitude=-74.21&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=America/New_York
    method: GET
    update_interval: 3h
    on_response:
      then:
        - lambda: |-
            // Parse JSON response
            // Extract next 7 days of data
            // Update Nextion components dt10-dt16, tem0-tem6, prec0-prec6
            for (int i = 0; i < 7; i++) {
              time_t day_timestamp = now() + (i * 86400);
              struct tm* timeinfo = localtime(&day_timestamp);
              char day_str[4];
              strftime(day_str, sizeof(day_str), "%a", timeinfo);  // "Mon", "Tue", etc.

              id(gazebo_nextion).send_command_printf("dt1%d.txt=\"%s\"", i, day_str);
              id(gazebo_nextion).send_command_printf("tem%d.val=%d", i, daily_temps_max[i]);
              id(gazebo_nextion).send_command_printf("prec%d.val=%d", i, daily_precip_prob[i]);
              int wmo_code = daily_weather_codes[i];
              int icon_id = wmo_to_icon(wmo_code);
              id(gazebo_nextion).send_command_printf("wxIcon%d.pic=%d", i, icon_id);
            }
```

**Update Priority**: LOW
- Daily forecast: Every 3 hours (less frequent than hourly)
- Still provides good visibility into week ahead

---

## Weather Code to Icon Mapping

**WMO Weather Code Reference** (OpenMeteo API):

| WMO Code | Condition | Icon ID | Description |
|---|---|---|---|
| 0 | Clear sky | 0 | Sunny ☀️ |
| 1-2 | Mainly clear | 1 | Mostly sunny |
| 3 | Overcast | 2 | Cloudy ☁️ |
| 45, 48 | Foggy | 3 | Fog 🌫️ |
| 51-55 | Drizzle | 4 | Light rain 🌧️ |
| 61-67 | Rain | 5 | Rain 🌧️ |
| 71-77 | Snow | 6 | Snow ❄️ |
| 80-82 | Rain showers | 7 | Showers 🌦️ |
| 85-86 | Snow showers | 8 | Snow showers |
| 95-99 | Thunderstorm | 9 | Thunderstorm ⚡ |

**Conversion Function**:

```cpp
int wmo_to_icon(int wmo_code) {
  if (wmo_code == 0) return 0;           // Clear
  else if (wmo_code <= 3) return 1;      // Mainly clear/overcast
  else if (wmo_code <= 48) return 2;     // Cloudy/foggy
  else if (wmo_code <= 55) return 4;     // Drizzle
  else if (wmo_code <= 67) return 5;     // Rain
  else if (wmo_code <= 77) return 6;     // Snow
  else if (wmo_code <= 82) return 7;     // Showers
  else if (wmo_code <= 86) return 8;     // Snow showers
  else if (wmo_code <= 99) return 9;     // Thunderstorm
  else return 0;                         // Default: clear
}
```

---

## UART Communication Protocol

### Baud Rate & Format
- **Baud Rate**: 9600 bps
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None

### Command Format
```
command_name.property=value[terminator]

Terminator: 0xFF 0xFF 0xFF (3 bytes)
```

### Examples

**Set number component value**:
```
temp.val=22[0xFF][0xFF][0xFF]
```

**Set text component value**:
```
dt0.txt="08:00"[0xFF][0xFF][0xFF]
```

**Set button color**:
```
b1.bco=0xFF0000[0xFF][0xFF][0xFF]  # Red (0xFF0000)
```

**Set image/picture ID**:
```
wxIcon0.pic=5[0xFF][0xFF][0xFF]  # Rain icon
```

### ESPHome UART Implementation

```yaml
uart:
  - id: uart_nextion
    tx_pin: GPIO17
    rx_pin: GPIO16
    baud_rate: 9600

display:
  - platform: nextion
    uart_id: uart_nextion
    id: gazebo_nextion
    lambda: |-
      // Placeholder - updates via send_command_printf

script:
  - id: update_nextion_temp
    then:
      - lambda: |-
          id(gazebo_nextion).send_command_printf("temp.val=%d", (int)id(gazebo_temp).state);
          id(gazebo_nextion).send_command_printf("n0.val=%d", (int)id(gazebo_temp).state);
```

---

## Touch Event Handling

### Button Press Detection

When user touches a button or slider on the Nextion, it sends an event via UART.

**Event Format**:
```
Touch event: 0x65 [page] [component] [event_type]
  page: Page number (0-3)
  component: Component ID
  event_type: 0x01 = press, 0x00 = release
```

### Example: Page 1 Slider Adjustment

```yaml
# Listen for touch events
on_page_change:
  then:
    - logger.log: "Page changed"

# For slider interaction, use lambda to read slider value
sensor:
  - platform: template
    id: nextion_slider_value
    update_interval: never
    lambda: |-
      float slider_val = id(gazebo_nextion).get_component_value("Slider0");
      return slider_val;
    on_value:
      then:
        - lambda: |-
            // Convert slider value (0-100) to temperature (5-35°C)
            float desired_temp = 5.0 + (x / 100.0) * 30.0;
            id(desired_temperature).publish_state(desired_temp);
```

---

## Testing & Validation

### Page 0 Testing (Current Priority)

**Test File**: `NEXTION_TEST_PAGE0.yaml`

**Purpose**: Verify wiring and UART communication by cycling temperature 0→99→0

**Test Procedure**:
1. Flash `NEXTION_TEST_PAGE0.yaml` to device
2. Verify display shows Page 0 (Extérieur)
3. Observe temperature counting: 0→99 (100 seconds), then 98→0 (99 seconds)
4. Verify updates occur every 1 second
5. Check logs for UART communication errors

**Success Criteria**:
- ✅ Display shows correct temperature values (0→99→0)
- ✅ Updates occur exactly every 1 second
- ✅ No UART errors in logs
- ✅ No display corruption or freezing
- ✅ Cycle repeats smoothly

**Component Under Test**: `temp` (Page 0 temperature display)

**Related Tasks**: See `specs/001-gazebo-stove-heating-control/tasks.md` - User Story 4 Testing Tasks (T101-T135)

---

### Page 3 Testing (Next Priority)

**Components to Test**:
- `n0` - Current temperature display (left side)
- `n1` - Desired temperature display (right side)
- `Slider0` - Temperature setpoint slider

**Test Procedure** (to be created):
1. Verify `n0` displays current temperature from DS18B20 sensor
2. Verify `n1` displays desired temperature from `desired_temp` entity
3. Test slider touch input - verify setpoint changes
4. Verify slider position updates when setpoint changes remotely

---

## Implementation Checklist (Phase 6)

### Week 11: Page 0 & 3 Implementation (OPERATIONAL PAGES)

**Status**: 🔄 **IN PROGRESS** - Testing Page 0 wiring

- [x] Set up UART communication (GPIO16/17 at 9600 baud)
- [ ] **CURRENT**: Verify Nextion display responds to commands (using NEXTION_TEST_PAGE0.yaml)
- [ ] Implement Page 0 (Extérieur) display updates:
  - [ ] **TESTING**: `temp` updates from DS18B20 (every 2 sec) - **USE TEST FILE FIRST**
  - [ ] `precip` updates from OpenMeteo (every 30 min) - ⏸️ Pending US5
  - [ ] Weather icon updates - ⏸️ Pending US5
  - [ ] `b1` status indicator (relay state)
  - [ ] Stove graphic updates (active/idle)
- [ ] Test Page 0 display accuracy and update timing
- [ ] Implement Page 3 (Slider Control):
  - [ ] Display current temperature (`n0`)
  - [ ] Display desired temperature (`n1`)
  - [ ] Implement Slider0 interaction (touch handling)
  - [ ] Map slider value (0-100) to temperature (5-35°C)
  - [ ] Test setpoint adjustment via slider

### Week 12: Page 1 & 2 Implementation (NON-OPERATIONAL - Pending US5)

**Status**: ⏸️ **DEFERRED** - Requires User Story 5 (Weather Forecast Display) implementation

- [ ] Implement OpenMeteo API integration
- [ ] Implement WMO to icon conversion function
- [ ] Implement Page 1 (Ressentie - hourly forecast):
  - [ ] Fetch 7-hour hourly forecast every 30 min
  - [ ] Update `dt0-dt6` (times)
  - [ ] Update `f01-f67` (temperatures) - Note: actual component names may differ
  - [ ] Update `rain0-rain6` (precipitation %)
  - [ ] Update weather icons (replace lightning bolt placeholders)
  - [ ] Test hourly forecast accuracy
- [ ] Implement Page 2 (Prévisions - daily forecast):
  - [ ] Fetch 7-day daily forecast every 3 hours
  - [ ] Update `dt10-dt16` (dates/days)
  - [ ] Update `tem0-tem6` (high temperatures)
  - [ ] Update `prec0-prec6` (precipitation %)
  - [ ] Update weather icons (replace lightning bolt placeholders)
  - [ ] Test daily forecast accuracy
- [ ] Test navigation between pages (arrow buttons b0, b1)
- [ ] Verify all display updates are smooth and responsive

### Testing & Validation

- [ ] UART communication: No corruption or dropped bytes
- [ ] Display update timing: Page 0 updates within 2 sec
- [ ] Forecast accuracy: Data matches OpenMeteo API
- [ ] Touch responsiveness: Slider adjusts smoothly
- [ ] Icon accuracy: Correct icons for each weather condition
- [ ] Navigation: All pages accessible via arrow buttons
- [ ] 24-hour continuous operation without display glitches

---

## Example ESPHome Configuration (Partial)

```yaml
uart:
  - id: uart_nextion
    tx_pin: GPIO17
    rx_pin: GPIO16
    baud_rate: 9600

display:
  - platform: nextion
    uart_id: uart_nextion
    id: gazebo_nextion
    tft_url: "http://192.168.0.10/gazebo.tft"  # Optional OTA

sensor:
  - platform: dallas_temp
    one_wire_id: ow_bus
    id: gazebo_temp
    on_value:
      then:
        - lambda: |-
            id(gazebo_nextion).send_command_printf("temp.val=%d", (int)x);
            id(gazebo_nextion).send_command_printf("n0.val=%d", (int)x);

  - platform: http_request
    id: openmeteo_hourly
    resource: https://api.open-meteo.com/v1/forecast?latitude=40.12&longitude=-74.21&hourly=temperature_2m,precipitation_probability,weather_code&timezone=America/New_York
    method: GET
    update_interval: 30min
    on_response:
      then:
        - lambda: |-
            // Parse hourly forecast and update Page 2
            // (Implementation details in full config)

  - platform: http_request
    id: openmeteo_daily
    resource: https://api.open-meteo.com/v1/forecast?latitude=40.12&longitude=-74.21&daily=weather_code,temperature_2m_max,precipitation_probability_max&timezone=America/New_York
    method: GET
    update_interval: 3h
    on_response:
      then:
        - lambda: |-
            // Parse daily forecast and update Page 3
            // (Implementation details in full config)

number:
  - platform: template
    id: desired_temperature
    name: Desired Temperature
    unit_of_measurement: "°C"
    min_value: 5
    max_value: 35
    step: 0.5
    set_action:
      then:
        - lambda: |-
            // Update Nextion n1 value and Slider0 position
            id(gazebo_nextion).send_command_printf("n1.val=%d", (int)x);
            float slider_val = ((x - 5.0) / 30.0) * 100.0;
            id(gazebo_nextion).send_command_printf("Slider0.val=%d", (int)slider_val);
```

---

## Troubleshooting

### Issue: Display shows no response

**Solutions**:
1. Verify UART connection: TX(GPIO17) → RX, RX(GPIO16) → TX
2. Check baud rate: Must be exactly 9600
3. Verify power supply to Nextion (5V)
4. Check termination bytes (0xFF 0xFF 0xFF) at end of commands

### Issue: Display updates lag or freeze

**Solutions**:
1. Reduce update frequency (don't send commands faster than 100ms apart)
2. Verify UART buffer not overflowing
3. Check for corrupted display firmware (reflash if needed)
4. Monitor CPU usage - ESPHome may be too busy

### Issue: Temperature values not updating

**Solutions**:
1. Verify DS18B20 sensor is reading correctly (check logs)
2. Ensure sensor.dallas_temp entity has valid state
3. Check UART commands are being sent (enable logging)
4. Verify component names match exactly (case-sensitive)

### Issue: Weather icons not displaying

**Solutions**:
1. Verify WMO code to icon mapping is correct
2. Check icon IDs are valid in Nextion (0-9)
3. Ensure API returns valid weather_code values
4. Test with hardcoded icon ID to verify display works

### Issue: Slider doesn't respond to touch

**Solutions**:
1. Verify Nextion firmware supports touch input
2. Check touch event handling in ESPHome
3. Ensure slider is on correct page (Page 1 only)
4. Test with other buttons to verify touch works

---

## Performance Considerations

### Update Frequency Optimization

| Component | Frequency | Rationale |
|-----------|-----------|-----------|
| Temperature (Page 0) | 2 seconds | Real-time feedback, critical |
| Weather icon (Page 0) | 30 minutes | Infrequent changes |
| Hourly forecast (Page 2) | 30 minutes | API limits, data freshness |
| Daily forecast (Page 3) | 3 hours | Less frequent than hourly |
| Slider position (Page 1) | Real-time | User feedback |

### UART Bandwidth

At 9600 baud:
- ~96 characters per second
- Typical command: `temp.val=22` = 11 bytes
- Max commands per second: ~8-10 commands
- Sufficient for display updates (well below limit)

### Memory Usage

- Forecast data caching: ~1KB per 7-hour forecast
- Display buffer: ~2KB
- Total: <10KB (well within ESP32 limits)

---

## Nextion File Updates

When updating Nextion firmware or adding new elements:

1. **Modify Nextion Editor project** (if available)
2. **Export compiled .tft file**
3. **Upload to web server** (e.g., `http://192.168.0.10/gazebo.tft`)
4. **Configure ESPHome** for OTA:
   ```yaml
   display:
     - platform: nextion
       tft_url: "http://192.168.0.10/gazebo.tft"
       auto_load_on_startup: true
   ```
5. **Restart device** to trigger download and reflash

---

## Testing Procedures

### Functional Testing

**Page 0 (Exterior)**:
1. Verify temperature displays correct DS18B20 reading
2. Verify weather icon updates when API data changes
3. Verify status button reflects relay state (on/off)
4. Verify stove graphic shows active/idle correctly

**Page 1 (Slider)**:
1. Adjust slider and verify temperature updates
2. Verify setpoint persists (check Home Assistant)
3. Verify slider position updates when setpoint changes remotely
4. Test full range (5-35°C)

**Page 2 (Hourly Forecast)**:
1. Verify 7 rows of hourly data display correctly
2. Check times are in 1-hour increments
3. Verify temperatures are realistic
4. Check weather icons match conditions

**Page 3 (Daily Forecast)**:
1. Verify 7 days of forecast display
2. Check dates increment by 1 day each row
3. Verify temperatures are realistic daily highs
4. Check precipitation probabilities are reasonable

**Navigation**:
1. Test arrow buttons move between pages
2. Test all 4 pages accessible
3. Verify page transitions are smooth

### Performance Testing

**Latency**:
- Temperature update latency: <2 seconds
- Setpoint slider response: <500ms
- Forecast fetch + display: <5 seconds

**Stability**:
- Run 24+ hours continuously
- Verify no UART errors in logs
- Verify no display glitches or corruption
- Check memory usage remains stable

---

## References

- **OpenMeteo API**: https://open-meteo.com/
- **WMO Weather Codes**: https://www.noaa.gov/
- **Nextion User Manual**: (Download from Nextion website)
- **HSD035383E5 Specifications**: 3.5" Display module specs

---

## Document Version

**Title**: GazeboWx Nextion Display Integration Guide
**Version**: 1.0.0
**Date**: 2025-11-10
**Status**: Complete - Ready for Phase 6 Implementation
**Implementation Phase**: Weeks 11-12

**Related Documents**:
- CONSTITUTION.md
- SPECIFICATION.md
- PLAN.md (Phase 6)
- FEATURE_MAPPING.md

---

**Next Step**: Use this guide during Phase 6 implementation to build complete Nextion display integration with all 4 pages and real-time data synchronization.
