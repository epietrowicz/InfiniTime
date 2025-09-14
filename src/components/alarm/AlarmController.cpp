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
#include "components/alarm/AlarmController.h"
#include "systemtask/SystemTask.h"
#include "task.h"
#include <chrono>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

AlarmController::AlarmController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
  // Initialize alarm data with defaults
  alarmData.version = alarmFormatVersion;
  alarmData.alarms = defaultAlarms;
}

namespace {
  void SetOffAlarm(TimerHandle_t xTimer) {
    auto* data = static_cast<AlarmTimerData*>(pvTimerGetTimerID(xTimer));
    data->controller->SetOffAlarmNow(data->alarmIndex);
  }
}

void AlarmController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;

  // Create timers for each alarm
  for (uint8_t i = 0; i < MaxAlarms; i++) {
    char timerName[16];
    snprintf(timerName, sizeof(timerName), "Alarm%u", i);
    // Set up timer data
    alarmTimerData[i].controller = this;
    alarmTimerData[i].alarmIndex = i;
    alarmTimers[i] = xTimerCreate(timerName, 1, pdFALSE, &alarmTimerData[i], SetOffAlarm);
  }

  LoadSettingsFromFile();

  // Schedule all enabled alarms
  for (uint8_t i = 0; i < MaxAlarms; i++) {
    if (alarmData.alarms[i].isEnabled) {
      NRF_LOG_INFO("[AlarmController] Loaded alarm %u was enabled, scheduling", i);
      ScheduleAlarm(i);
    }
  }
}

const char* AlarmController::GetAlarmName(uint8_t alarmIndex) const {
  return alarmNames[alarmIndex];
}

void AlarmController::SaveAlarms() {
  // verify if it is necessary to save
  if (alarmsChanged) {
    SaveSettingsToFile();
  }
  alarmsChanged = false;
}

void AlarmController::SaveAlarm() {
  SaveAlarms(); // Legacy compatibility
}

void AlarmController::SetAlarmTime(uint8_t alarmIndex, uint8_t alarmHr, uint8_t alarmMin) {
  if (alarmIndex >= MaxAlarms)
    return;

  if (alarmData.alarms[alarmIndex].hours == alarmHr && alarmData.alarms[alarmIndex].minutes == alarmMin) {
    return;
  }
  alarmData.alarms[alarmIndex].hours = alarmHr;
  alarmData.alarms[alarmIndex].minutes = alarmMin;
  alarmsChanged = true;
}

void AlarmController::ScheduleAlarm(uint8_t alarmIndex) {
  if (alarmIndex >= MaxAlarms)
    return;

  // Determine the next time the alarm needs to go off and set the timer
  xTimerStop(alarmTimers[alarmIndex], 0);

  auto now = dateTimeController.CurrentDateTime();
  alarmTimes[alarmIndex] = now;
  time_t ttAlarmTime =
    std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(alarmTimes[alarmIndex]));
  tm* tmAlarmTime = std::localtime(&ttAlarmTime);

  // If the time being set has already passed today,the alarm should be set for tomorrow
  if (alarmData.alarms[alarmIndex].hours < dateTimeController.Hours() ||
      (alarmData.alarms[alarmIndex].hours == dateTimeController.Hours() &&
       alarmData.alarms[alarmIndex].minutes <= dateTimeController.Minutes())) {
    tmAlarmTime->tm_mday += 1;
    // tm_wday doesn't update automatically
    tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
  }

  tmAlarmTime->tm_hour = alarmData.alarms[alarmIndex].hours;
  tmAlarmTime->tm_min = alarmData.alarms[alarmIndex].minutes;
  tmAlarmTime->tm_sec = 0;

  // if alarm is in weekday-only mode, make sure it shifts to the next weekday
  if (alarmData.alarms[alarmIndex].recurrence == RecurType::Weekdays) {
    if (tmAlarmTime->tm_wday == 0) { // Sunday, shift 1 day
      tmAlarmTime->tm_mday += 1;
    } else if (tmAlarmTime->tm_wday == 6) { // Saturday, shift 2 days
      tmAlarmTime->tm_mday += 2;
    }
  }
  tmAlarmTime->tm_isdst = -1; // use system timezone setting to determine DST

  // now can convert back to a time_point
  alarmTimes[alarmIndex] = std::chrono::system_clock::from_time_t(std::mktime(tmAlarmTime));
  auto secondsToAlarm = std::chrono::duration_cast<std::chrono::seconds>(alarmTimes[alarmIndex] - now).count();
  xTimerChangePeriod(alarmTimers[alarmIndex], secondsToAlarm * configTICK_RATE_HZ, 0);
  xTimerStart(alarmTimers[alarmIndex], 0);

  if (!alarmData.alarms[alarmIndex].isEnabled) {
    alarmData.alarms[alarmIndex].isEnabled = true;
    alarmsChanged = true;
  }
}

uint32_t AlarmController::SecondsToAlarm(uint8_t alarmIndex) const {
  if (alarmIndex >= MaxAlarms)
    return 0;
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTimes[alarmIndex] - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm(uint8_t alarmIndex) {
  if (alarmIndex >= MaxAlarms)
    return;

  xTimerStop(alarmTimers[alarmIndex], 0);
  if (alarmData.alarms[alarmIndex].isEnabled) {
    alarmData.alarms[alarmIndex].isEnabled = false;
    alarmsChanged = true;
  }
}

void AlarmController::SetOffAlarmNow(uint8_t alarmIndex) {
  if (alarmIndex >= MaxAlarms)
    return;

  isAlerting = true;
  currentAlertingAlarm = alarmIndex;
  systemTask->PushMessage(System::Messages::SetOffAlarm);
}

void AlarmController::StopAlerting() {
  isAlerting = false;
  // Disable alarm unless it is recurring
  if (alarmData.alarms[currentAlertingAlarm].recurrence == RecurType::None) {
    alarmData.alarms[currentAlertingAlarm].isEnabled = false;
    alarmsChanged = true;
  } else {
    // set next instance
    ScheduleAlarm(currentAlertingAlarm);
  }
}

void AlarmController::SetRecurrence(uint8_t alarmIndex, RecurType recurrence) {
  if (alarmIndex >= MaxAlarms)
    return;

  if (alarmData.alarms[alarmIndex].recurrence != recurrence) {
    alarmData.alarms[alarmIndex].recurrence = recurrence;
    alarmsChanged = true;
  }
}

void AlarmController::LoadSettingsFromFile() {
  lfs_file_t alarmFile;
  AlarmData alarmBuffer;

  if (fs.FileOpen(&alarmFile, "/.system/alarms.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file, using defaults");
    InitializeDefaultAlarms();
    return;
  }

  fs.FileRead(&alarmFile, reinterpret_cast<uint8_t*>(&alarmBuffer), sizeof(alarmBuffer));
  fs.FileClose(&alarmFile);

  if (alarmBuffer.version == alarmFormatVersion) {
    alarmData = alarmBuffer;
    NRF_LOG_INFO("[AlarmController] Loaded alarm settings from file");
  } else {
    NRF_LOG_WARNING("[AlarmController] Loaded alarm settings has version %u instead of %u, using defaults",
                    alarmBuffer.version,
                    alarmFormatVersion);
    InitializeDefaultAlarms();
  }
}

void AlarmController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system");
  }
  fs.DirClose(&systemDir);
  lfs_file_t alarmFile;
  if (fs.FileOpen(&alarmFile, "/.system/alarms.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[AlarmController] Failed to open alarm data file for saving");
    return;
  }

  fs.FileWrite(&alarmFile, reinterpret_cast<const uint8_t*>(&alarmData), sizeof(alarmData));
  fs.FileClose(&alarmFile);
  NRF_LOG_INFO("[AlarmController] Saved alarm settings with format version %u to file", alarmData.version);
}

void AlarmController::InitializeDefaultAlarms() {
  alarmData.version = alarmFormatVersion;
  alarmData.alarms = defaultAlarms;
  NRF_LOG_INFO("[AlarmController] Initialized with default alarms");
}
