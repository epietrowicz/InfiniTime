/*  Copyright (C) 2021 mruss77, Florian

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include <array>
#include <chrono>
#include <string>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class AlarmController;

    struct AlarmTimerData {
      Pinetime::Controllers::AlarmController* controller;
      uint8_t alarmIndex;
    };

    class AlarmController {
    public:
      AlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs);

      void Init(System::SystemTask* systemTask);
      void SaveAlarms();
      void SaveAlarm();
      void SetAlarmTime(uint8_t alarmIndex, uint8_t alarmHr, uint8_t alarmMin);
      void ScheduleAlarm(uint8_t alarmIndex);
      void DisableAlarm(uint8_t alarmIndex);
      void SetOffAlarmNow(uint8_t alarmIndex);
      uint32_t SecondsToAlarm(uint8_t alarmIndex) const;
      void StopAlerting();
      enum class RecurType { None, Daily, Weekdays };

      // Legacy single alarm interface for backward compatibility
      void SetAlarmTime(uint8_t alarmHr, uint8_t alarmMin) {
        SetAlarmTime(0, alarmHr, alarmMin);
      }

      void ScheduleAlarm() {
        ScheduleAlarm(0);
      }

      void DisableAlarm() {
        DisableAlarm(0);
      }

      void SetOffAlarmNow() {
        SetOffAlarmNow(0);
      }

      uint32_t SecondsToAlarm() const {
        return SecondsToAlarm(0);
      }

      uint8_t Hours(uint8_t alarmIndex = 0) const {
        return alarmData.alarms[alarmIndex].hours;
      }

      uint8_t Minutes(uint8_t alarmIndex = 0) const {
        return alarmData.alarms[alarmIndex].minutes;
      }

      bool IsAlerting() const {
        return isAlerting;
      }

      bool IsEnabled(uint8_t alarmIndex = 0) const {
        return alarmData.alarms[alarmIndex].isEnabled;
      }

      RecurType Recurrence(uint8_t alarmIndex = 0) const {
        return alarmData.alarms[alarmIndex].recurrence;
      }

      void SetRecurrence(uint8_t alarmIndex, RecurType recurrence);

      void SetRecurrence(RecurType recurrence) {
        SetRecurrence(0, recurrence);
      } // Legacy

      const char* GetAlarmName(uint8_t alarmIndex) const {
        return alarmNames[alarmIndex];
      }

      static constexpr uint8_t MaxAlarms = 5;

    private:
      // Versions 255 is reserved for now, so the version field can be made
      // bigger, should it ever be needed.
      static constexpr uint8_t alarmFormatVersion = 2;

      struct AlarmSettings {
        uint8_t hours = 7;
        uint8_t minutes = 0;
        RecurType recurrence = RecurType::None;
        bool isEnabled = false;
      };

      struct AlarmData {
        uint8_t version = alarmFormatVersion;
        std::array<AlarmSettings, MaxAlarms> alarms;
      };

      bool isAlerting = false;
      bool alarmsChanged = false;
      uint8_t currentAlertingAlarm = 0;

      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      System::SystemTask* systemTask = nullptr;
      std::array<TimerHandle_t, MaxAlarms> alarmTimers;
      std::array<struct AlarmTimerData, MaxAlarms> alarmTimerData;
      AlarmData alarmData;
      std::array<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>, MaxAlarms> alarmTimes;

      // Hardcoded alarm names
      inline static constexpr std::array<const char*, MaxAlarms> alarmNames = {"Wake Up", "Work Time", "Lunch Break", "Evening", "Bedtime"};

      // Hardcoded alarm definitions
      inline static constexpr std::array<AlarmSettings, MaxAlarms> defaultAlarms = {{{7, 0, RecurType::Daily, false},
                                                                                     {8, 30, RecurType::Weekdays, false},
                                                                                     {12, 0, RecurType::Weekdays, false},
                                                                                     {18, 0, RecurType::Daily, false},
                                                                                     {22, 0, RecurType::Daily, false}}};

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
      void InitializeDefaultAlarms();
    };
  }
}
