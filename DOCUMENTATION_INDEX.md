# Gazebo Stove Project - Complete Documentation Index

## Project Overview

**Project**: Gazebo Stove Heating Control System
**Status**: Phase 2 Complete, Phase 3 In Planning
**Last Updated**: 2025-11-11

---

## Core Configuration Files

### **gazebo_stove.yaml** (59 lines)
**Location**: `/gazebo_stove.yaml`
**Purpose**: Main ESPHome entry point that includes all sub-modules
**Contains**: YAML merge directives to include modular configurations
**Edit When**: You need to change the include order or add new modules

### **includes/core.yaml** (79 lines)
**Location**: `/includes/core.yaml`
**Purpose**: System and network configuration
**Contains**: WiFi, API, OTA, NTP time sync, logging
**Edit When**: Changing WiFi credentials, timezone, or API settings

### **includes/hardware.yaml** (49 lines)
**Location**: `/includes/hardware.yaml`
**Purpose**: GPIO pin and hardware control configuration
**Contains**: OneWire bus, relay, status LED outputs
**Edit When**: Adding GPIO devices, changing pin assignments

### **includes/sensors.yaml** (62 lines)
**Location**: `/includes/sensors.yaml`
**Purpose**: Temperature sensor and measurement configuration
**Contains**: DS18B20 sensor, filtering, malfunction detection
**Edit When**: Replacing sensor, adjusting filters, changing sensor address

### **includes/entities.yaml** (157 lines)
**Location**: `/includes/entities.yaml`
**Purpose**: User-facing controls and status indicators
**Contains**: Number sliders, select dropdowns, binary sensors, text sensors
**Edit When**: Adding user controls, changing value ranges

### **includes/automation.yaml** (60 lines)
**Location**: `/includes/automation.yaml`
**Purpose**: Heating control logic and automation scripts
**Contains**: `manage_heating` script, display initialization
**Edit When**: Modifying heating algorithm, adding new scripts

### **includes/display.yaml** (29 lines)
**Location**: `/includes/display.yaml`
**Purpose**: Nextion display integration via UART
**Contains**: UART configuration, display component
**Edit When**: Changing UART pins, adding display commands

### **includes/buttons.yaml** (22 lines)
**Location**: `/includes/buttons.yaml`
**Purpose**: Manual control buttons
**Contains**: Restart, safe shutdown buttons
**Edit When**: Adding new control buttons

### **includes/README.md** (Module Development Guide)
**Location**: `/includes/README.md`
**Purpose**: Comprehensive guide for module development
**Contains**: Overview of each module, dependencies, best practices
**Read When**: Creating new modules, understanding module structure

---

## Setup & Quick Start Guides

### **Docs/QUICK_START.md** (Setup Instructions)
**Location**: `/Docs/QUICK_START.md`
**Purpose**: Quick start guide for new setup
**Contains**: Hardware wiring, software installation, initial configuration
**Read When**: First time setup or troubleshooting initial issues

### **QUICK_REFERENCE.md** (Fast Lookup)
**Location**: `/QUICK_REFERENCE.md`
**Purpose**: Quick reference for common tasks and module locations
**Contains**: What to edit for common changes, debug commands
**Read When**: Looking for where to make specific configuration changes

---

## Phase Documentation

### **Docs/SPECIFICATION.md** (Feature Specification)
**Location**: `/Docs/SPECIFICATION.md`
**Purpose**: Complete feature specification and requirements
**Contains**: Functional requirements, hardware specs, design constraints
**Read When**: Understanding project requirements and scope

### **Docs/PHASE_1_IMPLEMENTATION.md** (Phase 1 Details)
**Location**: `/Docs/PHASE_1_IMPLEMENTATION.md`
**Purpose**: Phase 1 implementation and completion details
**Contains**: GPIO setup, sensor integration, relay control
**Status**: ✅ COMPLETE
**Read When**: Understanding Phase 1 foundation

### **Docs/PHASE_2_THERMOSTAT.md** (Phase 2 Details - v2.0)
**Location**: `/Docs/PHASE_2_THERMOSTAT.md`
**Purpose**: Heating control implementation with actual code
**Contains**: Algorithm details, testing procedures, troubleshooting
**Status**: ✅ COMPLETE
**Read When**: Understanding Phase 2 thermostat implementation
**Recent Update**: Updated to reflect actual heating-only implementation (not climate component)

### **PHASE_2_CORRECTION_SUMMARY.md** (What Changed)
**Location**: `/PHASE_2_CORRECTION_SUMMARY.md`
**Purpose**: Summary of PHASE_2_THERMOSTAT.md corrections
**Contains**: Before/after comparisons, rationale for changes
**Read When**: Understanding what was changed from original spec

### **Docs/PLAN.md** (Project Roadmap)
**Location**: `/Docs/PLAN.md`
**Purpose**: Overall project plan across all phases
**Contains**: Phase breakdown, timeline, feature descriptions
**Read When**: Planning work for upcoming phases

### **Docs/PROJECT_STATUS.md** (Current Status)
**Location**: `/Docs/PROJECT_STATUS.md`
**Purpose**: Current project status and progress tracking
**Contains**: Completed items, in-progress work, blockers
**Read When**: Checking overall project progress

---

## Architecture & Design Documentation

### **CONFIG_MODULES.md** (Modularization Guide)
**Location**: `/CONFIG_MODULES.md`
**Purpose**: Comprehensive guide to modular configuration structure
**Contains**: Module-by-module documentation, design decisions, future planning
**Length**: 1000+ lines
**Read When**: Understanding overall architecture, design decisions, troubleshooting configuration issues

### **MODULARIZATION_SUMMARY.md** (Executive Summary)
**Location**: `/MODULARIZATION_SUMMARY.md`
**Purpose**: Summary of refactoring from monolithic to modular config
**Contains**: Benefits achieved, file comparison, validation steps
**Length**: 300+ lines
**Read When**: Understanding why modules were created, comparing old vs new approach

### **Docs/FEATURE_MAPPING.md** (Features & Implementation)
**Location**: `/Docs/FEATURE_MAPPING.md`
**Purpose**: Map of features to implementation locations
**Contains**: Each feature, which files implement it, test procedures
**Read When**: Finding where a specific feature is implemented

---

## Hardware & Component Documentation

### **Docs/NEXTION_DISPLAY.md** (Display Documentation)
**Location**: `/Docs/NEXTION_DISPLAY.md`
**Purpose**: Nextion display integration and programming
**Contains**: Display commands, screen design, UART communication
**Read When**: Working with Nextion display features

### **Docs/NEXTION_TEST_README.md** (Display Testing)
**Location**: `/Docs/NEXTION_TEST_README.md`
**Purpose**: Testing and debugging Nextion display communication
**Contains**: UART commands, debug output examples
**Read When**: Troubleshooting display communication

### **Docs/OPENMETEO_API_REFERENCE.md** (Weather API)
**Location**: `/Docs/OPENMETEO_API_REFERENCE.md`
**Purpose**: Open-Meteo weather API reference (future use)
**Contains**: API endpoints, data formats, integration examples
**Read When**: Working on weather integration in Phase 4+

---

## Analysis & Planning Documents

### **Docs/CONSTITUTION.md** (Project Principles)
**Location**: `/Docs/CONSTITUTION.md`
**Purpose**: Project principles and guidelines
**Contains**: Design philosophy, coding standards
**Read When**: Understanding project values and approach

### **Docs/ANALYSIS_SUMMARY.md** (Design Analysis)
**Location**: `/Docs/ANALYSIS_SUMMARY.md`
**Purpose**: Analysis of design decisions
**Contains**: Trade-offs, alternatives considered
**Read When**: Understanding design rationale

### **Docs/CLARIFICATIONS_RESOLVED.md** (Q&A Log)
**Location**: `/Docs/CLARIFICATIONS_RESOLVED.md`
**Purpose**: Clarifications and questions resolved during development
**Contains**: FAQs, edge cases, design explanations
**Read When**: Looking for answers to specific questions

### **Docs/FINAL_SUMMARY.md** (Project Summary)
**Location**: `/Docs/FINAL_SUMMARY.md`
**Purpose**: Final summary of project structure and status
**Contains**: Overview of all components, architecture summary
**Read When**: Getting high-level overview of entire project

---

## Special References

### **DOCUMENTATION_INDEX.md** (This File)
**Location**: `/DOCUMENTATION_INDEX.md`
**Purpose**: Index of all documentation files
**Contains**: File listing with descriptions and purposes
**Use**: Navigate to specific documentation

---

## Documentation Summary by Category

### Configuration Documentation
- `gazebo_stove.yaml` - Main entry point
- `includes/*.yaml` - Sub-module files (7 files)
- `CONFIG_MODULES.md` - Module reference
- `QUICK_REFERENCE.md` - Quick lookup
- `includes/README.md` - Module development guide

### Setup & Usage Documentation
- `Docs/QUICK_START.md` - Initial setup
- `QUICK_REFERENCE.md` - Common tasks
- `Docs/README.md` - Project overview

### Phase Documentation
- `Docs/PHASE_1_IMPLEMENTATION.md` - Phase 1 details
- `Docs/PHASE_2_THERMOSTAT.md` - Phase 2 details (v2.0)
- `PHASE_2_CORRECTION_SUMMARY.md` - What changed
- `Docs/PLAN.md` - Project roadmap

### Architecture Documentation
- `CONFIG_MODULES.md` - Architecture guide
- `MODULARIZATION_SUMMARY.md` - Refactoring summary
- `Docs/FEATURE_MAPPING.md` - Feature locations

### Hardware Documentation
- `Docs/NEXTION_DISPLAY.md` - Display guide
- `Docs/NEXTION_TEST_README.md` - Display testing
- `Docs/OPENMETEO_API_REFERENCE.md` - Weather API

### Analysis & Design Documentation
- `Docs/CONSTITUTION.md` - Project principles
- `Docs/ANALYSIS_SUMMARY.md` - Design analysis
- `Docs/CLARIFICATIONS_RESOLVED.md` - Q&A log
- `Docs/FINAL_SUMMARY.md` - Project summary

---

## Quick Navigation Map

```
New to Project?
├─ Start: Docs/QUICK_START.md
├─ Then: Docs/SPECIFICATION.md
└─ Finally: CONFIG_MODULES.md

Need to Change Something?
├─ QUICK_REFERENCE.md (most common tasks)
├─ Locate file in CONFIG_MODULES.md
└─ Edit in includes/*.yaml

Understanding Architecture?
├─ CONFIG_MODULES.md (comprehensive)
├─ MODULARIZATION_SUMMARY.md (summary)
└─ includes/README.md (detailed)

Implementing New Feature?
├─ Docs/PLAN.md (see phase details)
├─ Appropriate phase documentation
└─ Docs/FEATURE_MAPPING.md (where to add)

Troubleshooting Issue?
├─ QUICK_REFERENCE.md (debug commands)
├─ Docs/PHASE_2_THERMOSTAT.md (troubleshooting section)
├─ CONFIG_MODULES.md (detailed explanations)
└─ Relevant includes/README.md (module-specific help)
```

---

## File Statistics

**Total Documentation Files**: 20+
**Total Configuration Files**: 8
**Total Lines of Code**: 517
**Total Lines of Documentation**: 5000+
**Total Project Files**: 30+

---

## Last Updated

- **Date**: 2025-11-11
- **Phase**: 2 Complete
- **Status**: Production Ready
- **Next Update**: After Phase 3 completion

---

## How to Use This Index

1. **Find what you need**: Look in the appropriate category
2. **Read the description**: Understand the file's purpose
3. **Check the status**: See if it's actively used
4. **Use "Edit When"**: Know when to modify that file
5. **Follow "Read When"**: Know when to read that file

For quick reference, use **QUICK_REFERENCE.md**
For detailed information, use **CONFIG_MODULES.md**
For high-level overview, use **Docs/README.md**

---
