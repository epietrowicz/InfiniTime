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
#include "displayapp/screens/AlarmList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/settings/Settings.h"
#include "components/alarm/AlarmController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"
#include <cstdio>

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::AlarmController;

namespace {
  void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<AlarmList*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

  void StopAlarmTaskCallback(lv_task_t* task) {
    auto* screen = static_cast<AlarmList*>(task->user_data);
    screen->StopAlerting();
  }
}

AlarmList::AlarmList(Controllers::AlarmController& alarmController,
                     Controllers::Settings::ClockType clockType,
                     System::SystemTask& systemTask,
                     Controllers::MotorController& motorController)
  : alarmController {alarmController}, wakeLock(systemTask), motorController {motorController} {

  // Create the main container
  alarmList = lv_list_create(lv_scr_act(), nullptr);
  lv_obj_set_size(alarmList, 240, 200);
  lv_obj_align(alarmList, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(alarmList, LV_LIST_PART_BG, LV_STATE_DEFAULT, Colors::bg);

  // Create alarm items for each alarm
  for (uint8_t i = 0; i < Controllers::AlarmController::MaxAlarms; i++) {
    CreateAlarmItem(i);
  }

  // Create stop button (hidden initially)
  btnStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnEventHandler);
  lv_obj_set_size(btnStop, 240, 70);
  lv_obj_align(btnStop, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStop, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  txtStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_static(txtStop, Symbols::stop);
  lv_obj_set_hidden(btnStop, true);

  UpdateAllAlarmItems();

  if (alarmController.IsAlerting()) {
    SetAlerting();
  }
}

AlarmList::~AlarmList() {
  if (alarmController.IsAlerting()) {
    StopAlerting();
  }
  lv_obj_clean(lv_scr_act());
  alarmController.SaveAlarms();
}

void AlarmList::CreateAlarmItem(uint8_t alarmIndex) {
  if (alarmIndex >= Controllers::AlarmController::MaxAlarms)
    return;

  // Create container for this alarm
  alarmItems[alarmIndex] = lv_cont_create(alarmList, nullptr);
  lv_obj_set_size(alarmItems[alarmIndex], 220, 50);
  lv_obj_set_style_local_bg_color(alarmItems[alarmIndex], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_border_width(alarmItems[alarmIndex], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_border_color(alarmItems[alarmIndex], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  // Create label for alarm info
  alarmLabels[alarmIndex] = lv_label_create(alarmItems[alarmIndex], nullptr);
  lv_obj_align(alarmLabels[alarmIndex], alarmItems[alarmIndex], LV_ALIGN_IN_LEFT_MID, 10, 0);
  lv_obj_set_style_local_text_font(alarmLabels[alarmIndex], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);

  // Create switch for enable/disable
  alarmSwitches[alarmIndex] = lv_switch_create(alarmItems[alarmIndex], nullptr);
  alarmSwitches[alarmIndex]->user_data = this;
  lv_obj_set_event_cb(alarmSwitches[alarmIndex], btnEventHandler);
  lv_obj_align(alarmSwitches[alarmIndex], alarmItems[alarmIndex], LV_ALIGN_IN_RIGHT_MID, -10, 0);
  lv_obj_set_size(alarmSwitches[alarmIndex], 60, 30);
  lv_obj_set_style_local_bg_color(alarmSwitches[alarmIndex], LV_SWITCH_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
}

void AlarmList::UpdateAlarmItem(uint8_t alarmIndex) {
  if (alarmIndex >= Controllers::AlarmController::MaxAlarms)
    return;

  char timeStr[32];
  char recurStr[16];
  char labelStr[64];

  FormatTimeString(timeStr,
                   sizeof(timeStr),
                   alarmController.Hours(alarmIndex),
                   alarmController.Minutes(alarmIndex),
                   Controllers::Settings::ClockType::H24);

  FormatRecurrenceString(recurStr, sizeof(recurStr), alarmController.Recurrence(alarmIndex));

  snprintf(labelStr, sizeof(labelStr), "%s\n%s %s", alarmController.GetAlarmName(alarmIndex), timeStr, recurStr);

  lv_label_set_text(alarmLabels[alarmIndex], labelStr);

  // Update switch state
  if (alarmController.IsEnabled(alarmIndex)) {
    lv_switch_on(alarmSwitches[alarmIndex], LV_ANIM_OFF);
  } else {
    lv_switch_off(alarmSwitches[alarmIndex], LV_ANIM_OFF);
  }
}

void AlarmList::UpdateAllAlarmItems() {
  for (uint8_t i = 0; i < Controllers::AlarmController::MaxAlarms; i++) {
    UpdateAlarmItem(i);
  }
}

void AlarmList::OnAlarmToggle(uint8_t alarmIndex, bool enabled) {
  if (alarmIndex >= Controllers::AlarmController::MaxAlarms)
    return;

  if (enabled) {
    alarmController.ScheduleAlarm(alarmIndex);
  } else {
    alarmController.DisableAlarm(alarmIndex);
  }
}

void AlarmList::FormatTimeString(char* buffer,
                                 size_t bufferSize,
                                 uint8_t hours,
                                 uint8_t minutes,
                                 Controllers::Settings::ClockType clockType) {
  if (clockType == Controllers::Settings::ClockType::H12) {
    bool isPM = hours >= 12;
    uint8_t displayHours = hours;
    if (displayHours == 0)
      displayHours = 12;
    else if (displayHours > 12)
      displayHours -= 12;

    snprintf(buffer, bufferSize, "%2u:%02u %s", displayHours, minutes, isPM ? "PM" : "AM");
  } else {
    snprintf(buffer, bufferSize, "%2u:%02u", hours, minutes);
  }
}

void AlarmList::FormatRecurrenceString(char* buffer, size_t bufferSize, Controllers::AlarmController::RecurType recurrence) {
  switch (recurrence) {
    case Controllers::AlarmController::RecurType::None:
      NRF_LOG_INFO("Once");
      snprintf(buffer, bufferSize, "Once");
      break;
    case Controllers::AlarmController::RecurType::Daily:
      NRF_LOG_INFO("Daily");
      snprintf(buffer, bufferSize, "Daily");
      break;
    case Controllers::AlarmController::RecurType::Weekdays:
      NRF_LOG_INFO("Mon-Fri");
      snprintf(buffer, bufferSize, "Mon-Fri");
      break;
  }
}

void AlarmList::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnStop) {
      StopAlerting();
      return;
    }

    // Check if it's an alarm switch
    for (uint8_t i = 0; i < Controllers::AlarmController::MaxAlarms; i++) {
      if (obj == alarmSwitches[i]) {
        bool enabled = lv_switch_get_state(alarmSwitches[i]);
        OnAlarmToggle(i, enabled);
        return;
      }
    }
  }
}

bool AlarmList::OnButtonPushed() {
  if (alarmController.IsAlerting()) {
    StopAlerting();
    return true;
  }
  return false;
}

bool AlarmList::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // Don't allow closing the screen by swiping while the alarm is alerting
  return alarmController.IsAlerting() && event == TouchEvents::SwipeDown;
}

void AlarmList::SetAlerting() {
  lv_obj_set_hidden(alarmList, true);
  lv_obj_set_hidden(btnStop, false);
  taskStopAlarm = lv_task_create(StopAlarmTaskCallback, 60 * 1000, LV_TASK_PRIO_MID, this); // <-- ms
  motorController.StartRinging();
  wakeLock.Lock();
}

void AlarmList::StopAlerting() {
  alarmController.StopAlerting();
  motorController.StopRinging();
  lv_obj_set_hidden(btnStop, true);
  lv_obj_set_hidden(alarmList, false);
  if (taskStopAlarm != nullptr) {
    lv_task_del(taskStopAlarm); // <-- now matches lv_task_t*
    taskStopAlarm = nullptr;
  }
  wakeLock.Release();
  UpdateAllAlarmItems();
}
