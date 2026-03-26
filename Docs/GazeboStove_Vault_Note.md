---
created: 2025-12-16
tags:
  - project
  - esphome
  - electronics
  - gazebo_stove
  - documentation
status: active
---

# GazeboStove Project Summary

## Overview
The **GazeboStove** project is a comprehensive ESPHome-based control system for a pellet stove/heating unit. It integrates advanced scheduling, local interface control via a Nextion touch display, and seamless Home Assistant integration.

### Key Components
*   **Controller**: ESP32 running ESPHome.
*   **Display**: Nextion HSD035383E5 (3.5" resistive touch) via UART.
*   **Sensors**: DS18B20 (Temperature), DHT22/BME280 (Environmental).
*   **Connectivity**: WiFi (Home Assistant API + OTA).
*   **Features**:
    *   PID Thermostat emulation.
    *   7-day Home/Away scheduling (on-device).
    *   Weather forecasting (OpenMeteo integration).
    *   Hardware safety interlocks.

## Documentation Index

### Core Documentation
*   [Specifications](SPECIFICATION.md): Feature requirements and hardware specs.
*   [Constitution](CONSTITUTION.md): Core principles and operational rules.
*   [Project Status](PROJECT_STATUS.md): Current development phase and tracking.
*   [Implementation Plan](PLAN.md): Original roadmap and phase breakdown.

### Technical Guides
*   [Nextion Display Guide](NEXTION_DISPLAY.md): UI layout, UART protocol, and update strategy.
*   [Schedule Implementation](SCHEDULE_IMPLEMENTATION.md): Logic for the Home/Away automation engine.
*   [Modules Configuration](CONFIG_MODULES.md): Breakdown of the YAML configuration files.
*   [Thermostat Migration](THERMOSTAT_PID_MIGRATION.md): Details on the PID control logic.
*   [OpenMeteo API](OPENMETEO_API_REFERENCE.md): Weather data integration details.
*   [Feature Mapping](FEATURE_MAPPING.md): Legacy Arduino to ESPHome feature map.

### Usage & Quick Links
*   [Quick Start](QUICK_START.md): Setup and flashing instructions.
*   [Quick Reference](QUICK_REFERENCE.md): Common commands and troubleshooting.
*   [Documentation Index](DOCUMENTATION_INDEX.md): Full hierarchical index.

## Current State
*   **Phase**: Active Development / Optimization.
*   **Focus**: Nextion display UI refinement and Hardware Upload stability.
