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
#include "components/alarm/MultipleAlarmController.h"
#include "systemtask/SystemTask.h"
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;

MultipleAlarmController::MultipleAlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
  InitializeHardcodedAlarms();
}

void MultipleAlarmController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

void MultipleAlarmController::InitializeHardcodedAlarms() {
  // Define 5 hardcoded alarms with different times and patterns
  alarms[0] = {7, 0, true, "Morning", HardcodedAlarm::RecurType::Weekdays};
  alarms[1] = {8, 30, true, "Work Start", HardcodedAlarm::RecurType::Weekdays};
  alarms[2] = {12, 0, true, "Lunch", HardcodedAlarm::RecurType::Daily};
  alarms[3] = {17, 0, true, "Evening", HardcodedAlarm::RecurType::Daily};
  alarms[4] = {22, 0, true, "Bedtime", HardcodedAlarm::RecurType::Daily};
}

void MultipleAlarmController::CheckAlarms() {
  if (isAlerting) {
    return; // Already alerting, don't check for more alarms
  }
  
  auto now = dateTimeController.CurrentDateTime();
  time_t ttNow = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(now));
  tm* tmNow = std::localtime(&ttNow);
  
  for (uint8_t i = 0; i < maxAlarms; i++) {
    if (!alarms[i].isEnabled) {
      continue;
    }
    
    if (ShouldTriggerAlarm(alarms[i])) {
      SetOffAlarmNow(i);
      break; // Only trigger one alarm at a time
    }
  }
}

bool MultipleAlarmController::ShouldTriggerAlarm(const HardcodedAlarm& alarm) const {
  auto now = dateTimeController.CurrentDateTime();
  time_t ttNow = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(now));
  tm* tmNow = std::localtime(&ttNow);
  
  // Check if current time matches alarm time (within 1 minute tolerance)
  if (tmNow->tm_hour == alarm.hours && tmNow->tm_min == alarm.minutes) {
    // Check recurrence pattern
    switch (alarm.recurrence) {
      case HardcodedAlarm::RecurType::None:
        // One-time alarm - would need additional logic to track if already triggered
        return true;
      case HardcodedAlarm::RecurType::Daily:
        return true;
      case HardcodedAlarm::RecurType::Weekdays:
        // Monday = 1, Sunday = 0
        return tmNow->tm_wday >= 1 && tmNow->tm_wday <= 5;
    }
  }
  
  return false;
}

void MultipleAlarmController::SetOffAlarmNow(uint8_t alarmIndex) {
  if (alarmIndex >= maxAlarms) {
    return;
  }
  
  isAlerting = true;
  alertingAlarmIndex = alarmIndex;
  systemTask->PushMessage(System::Messages::SetOffAlarm);
  NRF_LOG_INFO("[MultipleAlarmController] Triggering alarm %d: %s at %02d:%02d", 
               alarmIndex, 
               alarms[alarmIndex].name.c_str(),
               alarms[alarmIndex].hours,
               alarms[alarmIndex].minutes);
}

void MultipleAlarmController::StopAlerting() {
  isAlerting = false;
  alertingAlarmIndex = 0;
}

void MultipleAlarmController::ToggleAlarm(uint8_t alarmIndex) {
  if (alarmIndex >= maxAlarms) {
    return;
  }
  
  alarms[alarmIndex].isEnabled = !alarms[alarmIndex].isEnabled;
  NRF_LOG_INFO("[MultipleAlarmController] Toggled alarm %d (%s) to %s", 
               alarmIndex, 
               alarms[alarmIndex].name.c_str(),
               alarms[alarmIndex].isEnabled ? "enabled" : "disabled");
}
