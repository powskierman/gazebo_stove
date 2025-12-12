# OpenMeteo API Reference - Magog, QC Implementation

**Location**: Magog, Quebec, Canada
**Latitude**: 45.2617° N
**Longitude**: -72.1393° W
**Timezone**: America/Toronto
**Date**: 2025-11-10

---

## Quick Reference

### Hourly Forecast Endpoint

```
https://api.open-meteo.com/v1/forecast?
latitude=45.2617&
longitude=-72.1393&
hourly=temperature_2m,precipitation_probability,weather_code&
timezone=America/Toronto
```

**Response contains**: Next 7 days of hourly data (168 hours)

### Daily Forecast Endpoint

```
https://api.open-meteo.com/v1/forecast?
latitude=45.2617&
longitude=-72.1393&
daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&
timezone=America/Toronto
```

**Response contains**: Next 7 days of daily high/low temperatures and precipitation

---

## Complete API URLs

### Hourly (for Page 2 - "Ressentie")

```
https://api.open-meteo.com/v1/forecast?
latitude=45.2617&
longitude=-72.1393&
hourly=temperature_2m,precipitation_probability,weather_code&
timezone=America/Toronto&
forecast_days=1
```

**Parameters**:
- `latitude=45.2617` - Magog latitude
- `longitude=-72.1393` - Magog longitude
- `hourly=temperature_2m,precipitation_probability,weather_code` - Data fields
- `timezone=America/Toronto` - Eastern Time (EST/EDT)
- `forecast_days=1` - Include today + next days

**Returns**:
```json
{
  "latitude": 45.26,
  "longitude": -72.14,
  "generationtime_ms": 0.5,
  "utc_offset_seconds": -18000,
  "timezone": "America/Toronto",
  "timezone_abbreviation": "EST",
  "elevation": 30.0,
  "hourly": {
    "time": ["2025-11-10T00:00", "2025-11-10T01:00", ...],
    "temperature_2m": [5.2, 4.8, 3.9, ...],
    "precipitation_probability": [10, 15, 20, ...],
    "weather_code": [0, 1, 2, ...]
  }
}
```

### Daily (for Page 3 - "Prévisions")

```
https://api.open-meteo.com/v1/forecast?
latitude=45.2617&
longitude=-72.1393&
daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&
timezone=America/Toronto&
forecast_days=7
```

**Parameters**:
- `latitude=45.2617` - Magog latitude
- `longitude=-72.1393` - Magog longitude
- `daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max` - Daily data fields
- `timezone=America/Toronto` - Eastern Time
- `forecast_days=7` - 7-day forecast

**Returns**:
```json
{
  "daily": {
    "time": ["2025-11-10", "2025-11-11", ..., "2025-11-16"],
    "weather_code": [2, 3, 45, 61, 0, 1, 80],
    "temperature_2m_max": [12, 10, 8, 5, 14, 16, 11],
    "temperature_2m_min": [5, 2, 1, -2, 3, 8, 4],
    "precipitation_probability_max": [20, 60, 40, 80, 0, 10, 30]
  }
}
```

---

## WMO Weather Code Reference

**Used in OpenMeteo API responses**:

| Code | Condition | Icon | ESPHome Icon ID |
|------|-----------|------|-----------------|
| **0** | Clear sky | ☀️ | 0 |
| **1** | Mainly clear | 🌤️ | 1 |
| **2** | Partly cloudy | ⛅ | 1 |
| **3** | Overcast | ☁️ | 2 |
| **45** | Foggy | 🌫️ | 2 |
| **48** | Depositing rime fog | 🌫️ | 2 |
| **51** | Light drizzle | 🌧️ | 4 |
| **53** | Moderate drizzle | 🌧️ | 4 |
| **55** | Dense drizzle | 🌧️ | 4 |
| **61** | Slight rain | 🌧️ | 5 |
| **63** | Moderate rain | 🌧️ | 5 |
| **65** | Heavy rain | 🌧️ | 5 |
| **71** | Slight snow | ❄️ | 6 |
| **73** | Moderate snow | ❄️ | 6 |
| **75** | Heavy snow | ❄️ | 6 |
| **77** | Snow grains | ❄️ | 6 |
| **80** | Slight rain showers | 🌦️ | 7 |
| **81** | Moderate rain showers | 🌦️ | 7 |
| **82** | Violent rain showers | 🌦️ | 7 |
| **85** | Slight snow showers | ❄️ | 8 |
| **86** | Heavy snow showers | ❄️ | 8 |
| **95** | Thunderstorm | ⚡ | 9 |
| **96** | Thunderstorm with slight hail | ⚡ | 9 |
| **99** | Thunderstorm with heavy hail | ⚡ | 9 |

---

## ESPHome Implementation Examples

### Fetch Hourly Forecast

```yaml
sensor:
  - platform: http_request
    id: openmeteo_hourly
    resource: https://api.open-meteo.com/v1/forecast?latitude=45.2617&longitude=-72.1393&hourly=temperature_2m,precipitation_probability,weather_code&timezone=America/Toronto
    method: GET
    update_interval: 30min
    on_response:
      then:
        - logger.log: "Hourly forecast updated"
        - script.execute: update_hourly_forecast

script:
  - id: update_hourly_forecast
    then:
      - lambda: |-
          // Parse JSON and update Page 2 (Ressentie - hourly)
          // Extract next 7 hours of data
          // Update: dt0-dt6 (times), tt0-tt6 (temps), rain0-rain6 (precip %)
```

### Fetch Daily Forecast

```yaml
sensor:
  - platform: http_request
    id: openmeteo_daily
    resource: https://api.open-meteo.com/v1/forecast?latitude=45.2617&longitude=-72.1393&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=America/Toronto
    method: GET
    update_interval: 3h
    on_response:
      then:
        - logger.log: "Daily forecast updated"
        - script.execute: update_daily_forecast

script:
  - id: update_daily_forecast
    then:
      - lambda: |-
          // Parse JSON and update Page 3 (Prévisions - daily)
          // Extract next 7 days of data
          // Update: dt10-dt16 (dates), tem0-tem6 (highs), prec0-prec6 (precip %)
```

### Weather Code to Icon Conversion

```cpp
int wmo_to_icon(int wmo_code) {
  // Convert WMO code to Nextion icon ID
  if (wmo_code == 0) return 0;           // Clear (sun)
  else if (wmo_code <= 2) return 1;      // Partly cloudy
  else if (wmo_code <= 48) return 2;     // Cloudy/fog
  else if (wmo_code <= 55) return 4;     // Drizzle
  else if (wmo_code <= 67) return 5;     // Rain
  else if (wmo_code <= 77) return 6;     // Snow
  else if (wmo_code <= 82) return 7;     // Showers
  else if (wmo_code <= 86) return 8;     // Snow showers
  else return 9;                         // Thunderstorm (95-99)
}
```

---

## JSON Parsing Examples

### Parse Hourly Temperature

```cpp
// Example response - first 7 hours
const char* response = R"({
  "hourly": {
    "temperature_2m": [5.2, 4.8, 3.9, 2.1, 1.5, 0.8, 2.3],
    "precipitation_probability": [10, 15, 20, 30, 25, 10, 5],
    "weather_code": [1, 2, 3, 61, 51, 1, 0]
  }
})";

// Parse into arrays for Page 2
float hourly_temps[7] = {5, 5, 4, 2, 2, 1, 2};
int hourly_precip[7] = {10, 15, 20, 30, 25, 10, 5};
int hourly_icons[7] = {1, 2, 2, 5, 4, 1, 0};

// Update Nextion components
for (int i = 0; i < 7; i++) {
  id(gazebo_nextion).send_command_printf("tt%d.val=%d", i, (int)hourly_temps[i]);
  id(gazebo_nextion).send_command_printf("rain%d.val=%d", i, hourly_precip[i]);
  id(gazebo_nextion).send_command_printf("wxIcon%d.pic=%d", i, hourly_icons[i]);
}
```

### Parse Daily Temperature

```cpp
// Example response - next 7 days
const char* response = R"({
  "daily": {
    "time": ["2025-11-10", "2025-11-11", "2025-11-12", ...],
    "temperature_2m_max": [12, 10, 8, 5, 14, 16, 11],
    "temperature_2m_min": [5, 2, 1, -2, 3, 8, 4],
    "precipitation_probability_max": [20, 60, 40, 80, 0, 10, 30],
    "weather_code": [2, 3, 45, 61, 0, 1, 80]
  }
})";

// Parse into arrays for Page 3
int daily_temps_max[7] = {12, 10, 8, 5, 14, 16, 11};
int daily_precip[7] = {20, 60, 40, 80, 0, 10, 30};
int daily_icons[7] = {2, 2, 2, 5, 0, 1, 7};

// Update Nextion components
for (int i = 0; i < 7; i++) {
  id(gazebo_nextion).send_command_printf("tem%d.val=%d", i, daily_temps_max[i]);
  id(gazebo_nextion).send_command_printf("prec%d.val=%d", i, daily_precip[i]);
  id(gazebo_nextion).send_command_printf("wxIcon%d.pic=%d", i, daily_icons[i]);
}
```

---

## Key Characteristics

### OpenMeteo Advantages

✅ **Free**: No API key required
✅ **Reliable**: High uptime, good reliability
✅ **Fast**: Low latency responses (~100-200ms)
✅ **Accurate**: Same data as commercial APIs
✅ **Privacy-friendly**: No tracking, no ads
✅ **Open-source friendly**: Can use in open projects
✅ **Well-documented**: Clear API documentation
✅ **Rate limiting**: Generous limits (>10,000 requests/day free)

### Magog Location Notes

- **Latitude**: 45.2617° N (southern Quebec)
- **Longitude**: -72.1393° W (Eastern Canada)
- **Timezone**: America/Toronto (Eastern Time)
- **Seasonal variation**:
  - Winter: -15°C to -5°C typical
  - Summer: 15°C to 25°C typical
- **Precipitation**: Regular throughout year

### Data Freshness

- **Hourly forecast**: Updated every hour
- **Daily forecast**: Updated every 6 hours
- **Current conditions**: Real-time from weather stations
- **Lag**: Data 5-15 minutes behind real-time

---

## Magog Weather Patterns

**Typical Conditions by Season**:

| Season | Temperature | Precipitation | Wind | Notes |
|--------|-------------|---|---|---|
| Winter (Dec-Feb) | -15°C to -5°C | Snow 50+ cm | Moderate | Heating season |
| Spring (Mar-May) | 0°C to 15°C | Mixed rain/snow | Increasing | Variable |
| Summer (Jun-Aug) | 15°C to 25°C | Thunderstorms | Light | Warm, humid |
| Fall (Sep-Nov) | 5°C to 15°C | Rain | Moderate | Cooling down |

**Propane Stove Season**: October → April (6 months)

---

## Testing OpenMeteo API

### Manual Test URL

Copy-paste into browser:
```
https://api.open-meteo.com/v1/forecast?latitude=45.2617&longitude=-72.1393&hourly=temperature_2m,precipitation_probability,weather_code&timezone=America/Toronto&forecast_days=1
```

You should see JSON response with:
- Current Magog coordinates
- Hourly data arrays
- Current timezone (America/Toronto)

### cURL Test

```bash
curl "https://api.open-meteo.com/v1/forecast?latitude=45.2617&longitude=-72.1393&hourly=temperature_2m,precipitation_probability,weather_code&timezone=America/Toronto"
```

### Daily Test

```bash
curl "https://api.open-meteo.com/v1/forecast?latitude=45.2617&longitude=-72.1393&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=America/Toronto"
```

---

## Implementation Checklist

### Phase 7: Weather Forecast Integration

- [ ] Verify Magog coordinates (45.2617, -72.1393)
- [ ] Test OpenMeteo API endpoints in browser
- [ ] Implement hourly forecast fetch (30-minute interval)
- [ ] Implement daily forecast fetch (3-hour interval)
- [ ] Create WMO → Icon conversion function
- [ ] Parse JSON responses in lambda
- [ ] Update Nextion Page 2 (hourly) components
- [ ] Update Nextion Page 3 (daily) components
- [ ] Verify update timing and accuracy
- [ ] Test with various weather conditions
- [ ] Verify timezone conversion (UTC → America/Toronto)
- [ ] Monitor API response times

---

## Error Handling

### Common API Errors

| Error | Cause | Solution |
|-------|-------|----------|
| No response | Network down | Retry after 30 sec, show cached data |
| Invalid JSON | Parsing error | Log error, keep previous forecast |
| Coordinates wrong | Wrong lat/long | Verify Magog: 45.2617, -72.1393 |
| Timezone mismatch | Time offset wrong | Confirm America/Toronto |
| API down | OpenMeteo outage | Rare, use cached data fallback |

### Graceful Degradation

```cpp
// If API fetch fails
if (!api_response_valid) {
  // Use last known forecast (cached)
  // Display with "last updated" indicator
  // Retry fetch next interval
}
```

---

## Update Intervals

**Recommended for Weather**:

| Data | Interval | Rationale |
|------|----------|-----------|
| Hourly forecast | 30 minutes | Balance freshness vs API calls |
| Daily forecast | 3 hours | Less frequent changes |
| Current conditions | With forecast | Include in same API call |

**Bandwidth Impact**:
- Hourly fetch: ~1 KB response
- Daily fetch: ~500 bytes response
- Total: ~1.5 KB per 30 min = ~3 KB/hour
- Negligible impact

---

## Document Information

**Title**: OpenMeteo API Reference - Magog, QC Implementation
**Version**: 1.0.0
**Date**: 2025-11-10
**Location**: Magog, Quebec, Canada (45.2617, -72.1393)
**Status**: Ready for Phase 7 implementation

**Related Files**:
- NEXTION_DISPLAY.md (Pages 2-3 display updates)
- PLAN.md (Phase 7: Weather Integration)
- SPECIFICATION.md (Feature 16: Weather Forecast)

---

**Ready to implement OpenMeteo weather integration during Phase 7!**
