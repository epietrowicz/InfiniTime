# Multiple Alarms Feature

This document describes the multiple alarm functionality added to InfiniTime.

## Overview

The alarm system has been enhanced to support multiple hardcoded pre-defined alarms instead of just a single alarm. This allows users to set up different alarms for different purposes (wake up, work, lunch, evening, bedtime).

## Features

### Hardcoded Alarms

The system includes 5 pre-defined alarms:

1. **Wake Up** - 7:00 AM, Daily
2. **Work Time** - 8:30 AM, Weekdays (Monday-Friday)
3. **Lunch Break** - 12:00 PM, Weekdays (Monday-Friday)
4. **Evening** - 6:00 PM, Daily
5. **Bedtime** - 10:00 PM, Daily

### Alarm Management

- Each alarm can be individually enabled/disabled
- Alarms maintain their own recurrence settings
- All alarms are persistent across reboots
- Backward compatibility with existing single alarm system

## User Interface

### AlarmList App

A new "AlarmList" app provides a list view of all alarms with:

- Alarm name and time display
- Individual enable/disable switches
- Recurrence information
- Stop button when alarm is alerting

### Original Alarm App

The original Alarm app continues to work and controls the first alarm (Wake Up) for backward compatibility.

## Technical Implementation

### Data Structure

- `AlarmData` struct contains array of `AlarmSettings`
- Each alarm has: hours, minutes, recurrence, enabled state
- File format version 2 for multiple alarms
- Automatic migration from version 1 (single alarm)

### Timer Management

- Each alarm has its own FreeRTOS timer
- Independent scheduling and triggering
- Proper cleanup and resource management

### Storage

- Alarms stored in `/.system/alarm.dat`
- Automatic migration from legacy single alarm format
- Persistent across device reboots

## Usage

1. **Accessing Alarms**: Use the "AlarmList" app from the main menu
2. **Enabling Alarms**: Toggle the switch next to each alarm
3. **Stopping Alarms**: Use the stop button when alarm is alerting
4. **Legacy Support**: Original Alarm app still works for first alarm

## Configuration

The hardcoded alarm times and names can be modified in `AlarmController.h`:

```cpp
static constexpr std::array<const char*, MaxAlarms> alarmNames = {
  "Wake Up",
  "Work Time",
  "Lunch Break",
  "Evening",
  "Bedtime"
};

static constexpr std::array<AlarmSettings, MaxAlarms> defaultAlarms = {{
  {7, 0, RecurType::Daily, false},    // Wake Up
  {8, 30, RecurType::Weekdays, false}, // Work Time
  {12, 0, RecurType::Weekdays, false}, // Lunch Break
  {18, 0, RecurType::Daily, false},   // Evening
  {22, 0, RecurType::Daily, false}    // Bedtime
}};
```

## Backward Compatibility

- Existing single alarm data is automatically migrated
- Original Alarm app continues to work unchanged
- No breaking changes to existing functionality
