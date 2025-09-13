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

#include "components/datetime/DateTimeController.h"
#include "components/fs/FS.h"
#include "systemtask/SystemTask.h"
#include <array>
#include <chrono>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class MultipleAlarmController {
    public:
      struct HardcodedAlarm {
        uint8_t hours;
        uint8_t minutes;
        bool isEnabled;
        std::string name;
        enum class RecurType { None, Daily, Weekdays };
        RecurType recurrence;
      };

      MultipleAlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs);
      void Init(System::SystemTask* systemTask);
      void CheckAlarms();
      void SetOffAlarmNow(uint8_t alarmIndex);
      void StopAlerting();
      void ToggleAlarm(uint8_t alarmIndex);
      
      bool IsAlerting() const {
        return isAlerting;
      }
      
      uint8_t GetAlertingAlarmIndex() const {
        return alertingAlarmIndex;
      }
      
      const std::array<HardcodedAlarm, 5>& GetAlarms() const {
        return alarms;
      }
      
      const HardcodedAlarm& GetAlarm(uint8_t index) const {
        return alarms[index];
      }

    private:
      static constexpr uint8_t maxAlarms = 5;
      std::array<HardcodedAlarm, maxAlarms> alarms;
      bool isAlerting = false;
      uint8_t alertingAlarmIndex = 0;
      
      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      System::SystemTask* systemTask = nullptr;
      
      void InitializeHardcodedAlarms();
      bool ShouldTriggerAlarm(const HardcodedAlarm& alarm) const;
    };
  }
}
